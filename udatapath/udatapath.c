/* Copyright (c) 2008 The Board of Trustees of The Leland Stanford
 * Junior University
 * 
 * We are making the OpenFlow specification and associated documentation
 * (Software) available for public use and benefit with the expectation
 * that others will use, modify and enhance the Software and contribute
 * those enhancements back to the community. However, since we would
 * like to make the Software available for broadest use, with as few
 * restrictions as possible permission is hereby granted, free of
 * charge, to any person obtaining a copy of this Software to deal in
 * the Software under the copyrights without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * The name and trademarks of copyright holder(s) may NOT be used in
 * advertising or publicity pertaining to the Software or any
 * derivatives without specific, written prior permission.
 */

#include <config.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Peicheng Jiang insert */
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> /* netbd.h is needed for struct hostent =) */
/* End */

#include "command-line.h"
#include "daemon.h"
#include "datapath.h"
#include "fault.h"
#include "openflow/openflow.h"
#include "poll-loop.h"
#include "queue.h"
#include "util.h"
#include "rconn.h"
#include "timeval.h"
#include "vconn.h"
#include "dirs.h"
#include "vconn-ssl.h"
#include "vlog-socket.h"

#define THIS_MODULE VLM_udatapath
#include "vlog.h"

/* Strings to describe the manufacturer, hardware, and software.  This data 
 * is queriable through the switch description stats message. */
char mfr_desc[DESC_STR_LEN] = "Stanford University";
char hw_desc[DESC_STR_LEN] = "Reference Userspace Switch";
char sw_desc[DESC_STR_LEN] = VERSION BUILDNR;
char dp_desc[DESC_STR_LEN] = "";
char serial_num[SERIAL_NUM_LEN] = "None";

static void parse_options(int argc, char *argv[]);
static void usage(void) NO_RETURN;

static struct datapath *dp;
static uint64_t dpid = UINT64_MAX;
static char *port_list;
static char *local_port = "tap:";
static uint16_t num_queues = NETDEV_MAX_QUEUES;

static void add_ports(struct datapath *dp, char *port_list);

/* Peicheng Jiang Insert for periodly check file */
static void file_period_check();
/* End in 3.2.2013 */

/* Peicheng Jiang insert for whole environment parameter  */
#define CONFIG_FILE "/etc/dump_flow.conf"
#define WRITE_FILE "/var/dumped_flow.log"

int dump_flow_client_fd; /* files descriptors */
struct sockaddr_in dump_flow_client_server;
socklen_t dump_flow_client_len;

FILE *dump_flow_print_stream;
/* End in 6.6.2012 */

/* Peicheng Jiang insert for periodically dumping flows */
int dump_period = 10000; // the default period (ms)
uint64_t last_dump_time = 0; //define the last dumping time 
bool JS = false;
/* End in 2012.10.12 */
/* Peicheng Jiang insert for the total dump flow */
int total_number = 1;  //the default total number for dump flows
struct list total_dump_list;
/* End in 12.26.2012 */

/* Peicheng Jiang insert for switch of P I D M T */
int PIDMT[5];
int PERIOD = 1;
int INSERT = 0;
int DELETE = 0;
int MODIFY = 0;
int TIMEOUT = 0;
int file_period = 1000; // the default period(ms)
uint64_t last_file_period = 0;
FILE *p_file;
/* End in 3.2.2013 */
int
main(int argc, char *argv[])
{
    int n_listeners;
    int error;
    int i;

    /* Peicheng Jiang insert for the init of the total_dump_list */
    list_init(&total_dump_list);
    /* End on 12.26.2012 */
    /* Peicheng Jiang insert for whole environment parameter */
    struct sockaddr_in  client; /* server's address information */
    char dump_flow_client_ip[20];
    int dump_flow_client_port;
    struct hostent *dump_flow_client_he; /* structure that will get information about remote host */
    FILE *dump_flow_client_pf;
    char s_port[20];
    char s_period[20];
    char s_total[20]; 
    char temp[100];
    char on_off[10];
    char *S_temp;
    dump_flow_client_pf = fopen(CONFIG_FILE,"r");
    if(!dump_flow_client_pf)
    {
        printf("Open the dump_flow_server_IP file error!\n");
        return 1;
    }
    fgets(temp, 100, dump_flow_client_pf);
    fgets(dump_flow_client_ip,20,dump_flow_client_pf);
    dump_flow_client_ip[strlen(dump_flow_client_ip)-1]='\0';
    fgets(temp, 100, dump_flow_client_pf);
    fgets(s_port,20,dump_flow_client_pf);
    dump_flow_client_port = atoi(s_port);
    fgets(temp, 100, dump_flow_client_pf);
    fgets(s_period,20,dump_flow_client_pf);
    dump_period = atoi(s_period);
    
    fgets(temp, 100, dump_flow_client_pf);
    fgets(s_total, 20, dump_flow_client_pf);
    total_number = atoi(s_total);
    total_number--;

    fgets(temp, 100, dump_flow_client_pf);
    fgets(on_off, 10, dump_flow_client_pf);
    for(i=0; i<5; i++){
        S_temp = strtok(on_off, " ");
        PIDMT[i] = atoi(S_temp);
    }
    PERIOD = PIDMT[0];
    INSERT = PIDMT[1];
    DELETE = PIDMT[2];
    MODIFY = PIDMT[3];
    TIMEOUT = PIDMT[4];

    close(dump_flow_client_pf); /* close fd */

    if ((dump_flow_client_he=gethostbyname(dump_flow_client_ip))==NULL)
    { 
        /* calls gethostbyname() */
        printf("dump_flow_client gethostbyname() error\n");
        printf("%s\n",dump_flow_client_ip);
        exit(1);
    }


    if ((dump_flow_client_fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
    { 
        /* calls socket() */
        printf("socket() error\n");
        return 1;
    }
    bzero(&dump_flow_client_server,sizeof(dump_flow_client_server));
    dump_flow_client_server.sin_family = AF_INET;
    dump_flow_client_server.sin_port = htons(dump_flow_client_port); /* htons() is needed again */
    dump_flow_client_server.sin_addr = *((struct in_addr *)dump_flow_client_he->h_addr); /*he->h_addr passes "*he"'s info to "h_addr" */
    client.sin_family = AF_INET;
    client.sin_port = htons(6688);
    client.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(dump_flow_client_fd, (struct sockaddr*)&client, sizeof client);
    dump_flow_client_len=sizeof(struct sockaddr_in);


    dump_flow_print_stream = fopen(WRITE_FILE,"a"); 

    /* End in 6.6.2012 */

    set_program_name(argv[0]);
    register_fault_handlers();
    time_init();
    vlog_init();
    parse_options(argc, argv);
    signal(SIGPIPE, SIG_IGN);

    if (argc - optind < 1) {
        ofp_fatal(0, "at least one listener argument is required; "
          "use --help for usage");
    }

    error = dp_new(&dp, dpid);

    n_listeners = 0;
    for (i = optind; i < argc; i++) {
        const char *pvconn_name = argv[i];
        struct pvconn *pvconn;
        int retval;

        retval = pvconn_open(pvconn_name, &pvconn);
        if (!retval || retval == EAGAIN) {
            dp_add_pvconn(dp, pvconn);
            n_listeners++;
        } else {
            ofp_error(retval, "opening %s", pvconn_name);
        }
    }
    if (!n_listeners) {
        ofp_fatal(0, "could not listen for any connections");
    }

    if (port_list) {
        add_ports(dp, port_list);
    }
    if (local_port) {
        error = dp_add_local_port(dp, local_port, 0);
        if (error) {
            ofp_fatal(error, "failed to add local port %s", local_port);
        }
    }

    error = vlog_server_listen(NULL, NULL);
    if (error) {
        ofp_fatal(error, "could not listen for vlog connections");
    }

    die_if_already_running();
    daemonize();

    for (;;) {
        dp_run(dp);
        file_period_check();
        dp_wait(dp);
        poll_block();
    }
    /* Peicheng Jiang insert */
    close(dump_flow_client_fd);  // close the udp socket to the trace server
    fclose(dump_flow_print_stream);
    /* End */
    return 0;
}

/* Peicheng Jiang */
static void
file_period_check()
{
    uint64_t time_now_ = time_msec();
    if(file_period + last_file_period <= time_now_){
        int i;
        char temp[100];
        char on_off[10];
        p_file = fopen(CONFIG_FILE, "r");
        if(!p_file)
        {
            printf("Open the period file error!\n");
            return 1;
        }
        for(i=0; i<9; i++){
            fgets(temp, 100, p_file);
        }
        fgets(on_off, 10, p_file);
        for(i=0; i<5; i++){
            PIDMT[i] = atoi(strtok(on_off, " "));
        }
        fclose(p_file);
        last_file_period = time_now_;
    } 
}
/* End */

static void
add_ports(struct datapath *dp, char *port_list)
{
    char *port, *save_ptr;

    /* Glibc 2.7 has a bug in strtok_r when compiling with optimization that
     * can cause segfaults here:
     * http://sources.redhat.com/bugzilla/show_bug.cgi?id=5614.
     * Using ",," instead of the obvious "," works around it. */
    for (port = strtok_r(port_list, ",,", &save_ptr); port;
         port = strtok_r(NULL, ",,", &save_ptr)) {
        int error = dp_add_port(dp, port, num_queues);
        if (error) {
            ofp_fatal(error, "failed to add port %s", port);
        }
    }
}

static void
parse_options(int argc, char *argv[])
{
    enum {
        OPT_MFR_DESC = UCHAR_MAX + 1,
        OPT_HW_DESC,
        OPT_SW_DESC,
	OPT_DP_DESC,
        OPT_SERIAL_NUM,
        OPT_BOOTSTRAP_CA_CERT,
        OPT_NO_LOCAL_PORT,
        OPT_NO_SLICING
    };

    static struct option long_options[] = {
        {"interfaces",  required_argument, 0, 'i'},
        {"local-port",  required_argument, 0, 'L'},
        {"no-local-port", no_argument, 0, OPT_NO_LOCAL_PORT},
        {"datapath-id", required_argument, 0, 'd'},
        {"verbose",     optional_argument, 0, 'v'},
        {"help",        no_argument, 0, 'h'},
        {"version",     no_argument, 0, 'V'},
        {"no-slicing",  no_argument, 0, OPT_NO_SLICING},
        {"mfr-desc",    required_argument, 0, OPT_MFR_DESC},
        {"hw-desc",     required_argument, 0, OPT_HW_DESC},
        {"sw-desc",     required_argument, 0, OPT_SW_DESC},
        {"dp_desc",  required_argument, 0, OPT_DP_DESC},
        {"serial_num",  required_argument, 0, OPT_SERIAL_NUM},
        DAEMON_LONG_OPTIONS,
#ifdef HAVE_OPENSSL
        VCONN_SSL_LONG_OPTIONS
        {"bootstrap-ca-cert", required_argument, 0, OPT_BOOTSTRAP_CA_CERT},
#endif
        {0, 0, 0, 0},
    };
    char *short_options = long_options_to_short_options(long_options);

    for (;;) {
        int indexptr;
        int c;

        c = getopt_long(argc, argv, short_options, long_options, &indexptr);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'd':
            if (strlen(optarg) != 12
                || strspn(optarg, "0123456789abcdefABCDEF") != 12) {
                ofp_fatal(0, "argument to -d or --datapath-id must be "
                          "exactly 12 hex digits");
            }
            dpid = strtoll(optarg, NULL, 16);
            if (!dpid) {
                ofp_fatal(0, "argument to -d or --datapath-id must "
                          "be nonzero");
            }
            break;

        case 'h':
            usage();

        case 'V':
            printf("%s %s compiled "__DATE__" "__TIME__"\n",
                   program_name, VERSION BUILDNR);
            exit(EXIT_SUCCESS);

        case 'v':
            vlog_set_verbosity(optarg);
            break;

        case 'i':
            if (!port_list) {
                port_list = optarg;
            } else {
                port_list = xasprintf("%s,%s", port_list, optarg);
            }
            break;

        case 'L':
            local_port = optarg;
            break;

        case OPT_NO_LOCAL_PORT:
            local_port = NULL;
            break;

        case OPT_MFR_DESC:
            strncpy(mfr_desc, optarg, sizeof mfr_desc);
            break;

        case OPT_HW_DESC:
            strncpy(hw_desc, optarg, sizeof hw_desc);
            break;

        case OPT_SW_DESC:
            strncpy(sw_desc, optarg, sizeof sw_desc);
            break;

        case OPT_DP_DESC:
            strncpy(dp_desc, optarg, sizeof dp_desc);
            break;

        case OPT_SERIAL_NUM:
            strncpy(serial_num, optarg, sizeof serial_num);
            break;

        case OPT_NO_SLICING:
            num_queues = 0;
            break;

        DAEMON_OPTION_HANDLERS

#ifdef HAVE_OPENSSL
        VCONN_SSL_OPTION_HANDLERS

        case OPT_BOOTSTRAP_CA_CERT:
            vconn_ssl_set_ca_cert_file(optarg, true);
            break;
#endif

        case '?':
            exit(EXIT_FAILURE);

        default:
            abort();
        }
    }
    free(short_options);
}

static void
usage(void)
{
    printf("%s: userspace OpenFlow datapath\n"
           "usage: %s [OPTIONS] LISTEN...\n"
           "where LISTEN is a passive OpenFlow connection method on which\n"
       "to listen for incoming connections from the secure channel.\n",
           program_name, program_name);
    vconn_usage(false, true, false);
    printf("\nConfiguration options:\n"
           "  -i, --interfaces=NETDEV[,NETDEV]...\n"
           "                          add specified initial switch ports\n"
           "  -L, --local-port=NETDEV set network device for local port\n"
           "  --no-local-port         disable local port\n"
           "  -d, --datapath-id=ID    Use ID as the OpenFlow switch ID\n"
           "                          (ID must consist of 12 hex digits)\n"
           "  --no-slicing            disable slicing\n"
           "\nOther options:\n"
           "  -D, --detach            run in background as daemon\n"
           "  -P, --pidfile[=FILE]    create pidfile (default: %s/udatapath.pid)\n"
           "  -f, --force             with -P, start even if already running\n"
           "  -v, --verbose=MODULE[:FACILITY[:LEVEL]]  set logging levels\n"
           "  -v, --verbose           set maximum verbosity level\n"
           "  -h, --help              display this help message\n"
           "  -V, --version           display version information\n",
        ofp_rundir);
    exit(EXIT_SUCCESS);
}

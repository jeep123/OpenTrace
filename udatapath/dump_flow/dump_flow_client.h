/* @Author: Qingyun Zuo (modified by Peicheng Jiang @05.15.2012)
   @Date  : 05.13.2012
   @Func  : Send the struct using udp
*/
#ifndef DUMP_FLOW_CLIENT_H
#define DUMP_FLOW_CLIENT_H 1

/*#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> /* netbd.h is needed for struct hostent =) */

#define MAXDATASIZE 1000 /* Max number of bytes of data */
//#define CONFIG_FILE "dump_flow/dump_flow_server_IP.conf"
int udp_socket(char *argv, int packet_len);

extern int dump_flow_client_fd; /* files descriptors */
extern struct sockaddr_in dump_flow_client_server;
extern socklen_t dump_flow_client_len;

//extern char dump_flow_client_ip[20];
//extern int dump_flow_client_port;
#endif

/* @Author: Qingyun Zuo (modified by Peicheng Jiang @05.15.2012)
   @Date  : 05.13.2012
   @Func  : Send the struct using udp
*/

#include "dump_flow_client.h"

int udp_socket(char *argv, int packet_len)
{
    //int fd; /* files descriptors */
    //char ip[20];
    //char s_port[20];
    //int port;
    //struct hostent *he; /* structure that will get information about remote host */
    //struct sockaddr_in server, client; /* server's address information */
   // socklen_t len;

    //FILE *pf = fopen(CONFIG_FILE,"r");

    /*if(!dump_flow_clinet_pf)
    {
	printf("Open file error!\n");
	return 0;
    }
    fgets(ip,40,dump_flow_clinet_pf);
    ip[strlen(ip)-1]='\0';
    fgets(s_port,40,dump_flow_clinet_pf);
    port = atoi(s_port);
    close(dump_flow_clinet_pf);  */
	
    //if ((he=gethostbyname(dump_flow_client_ip))==NULL)
    //{ 
		/* calls gethostbyname() */
	///	printf("gethostbyname() error\n");
	//	printf("%s\n",dump_flow_client_ip);
	//	exit(1);
    //}
    /*if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
    { 
		printf("socket() error\n");
		return 1;
    }
    bzero(&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(dump_flow_client_port); 
    server.sin_addr = *((struct in_addr *)dump_flow_client_he->h_addr);
    client.sin_family = AF_INET;
    client.sin_port = htons(6688);
    client.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(fd, (struct sockaddr*)&client, sizeof client);
    len=sizeof(struct sockaddr_in);*/

    sendto(dump_flow_client_fd,argv,packet_len,0,(struct sockaddr *)&dump_flow_client_server,dump_flow_client_len);

    return 0;
}

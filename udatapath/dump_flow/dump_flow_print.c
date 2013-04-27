/* @Author: Peicheng Jiang
   @Date  : 05.13.2012
   @Func  : Print the flow and store it in the local filesystem
*/

#include "dump_flow_print.h"

int flow_print_(struct dump_flow *dump_flow_p)
{

    char *type1;
    char fbuff[BUFSIZE];
    int fbuff_size;
    //FILE *stream = fopen(WRITE_FILE,"a"); 
    if(!dump_flow_print_stream)
    {
	printf("Open file error!\n");
	//return 1;
    }

    if(dump_flow_p->dl_type == 2048){
        if(dump_flow_p->nw_proto == 1){
	    //printf("icmp ->");
	    type1 = "icmp ->";
	  }
	else if(dump_flow_p->nw_proto == 6)
	    //printf("tcp ->"); 
	    type1 = "tcp ->";
	else if(dump_flow_p->nw_proto == 17)
	    //printf( "udp ->");
	    type1 = "udp ->";
	else
	    //printf( "XX ->");
	    type1 = "IP_XX ->";
    }
    else if(dump_flow_p->dl_type == 2054){
            //printf( "arp ->");  
	type1 = "arp ->"; 
    }
    else
	type1 = "XX"; 

    sprintf(fbuff, "@ %lld %c %c %s in_port %d dl_vlan %d dl_src %x:%x:%x:%x:%x:%x dl_dst %x:%x:%x:%x:%x:%x dl_type %04x nw_src %d.%d.%d.%d/%d nw_dst %d.%d.%d.%d/%d nw_proto %d nw_tos %d tp_src %d tp_dst %d duration_sec %llds duration_nsec %lldms idle_timeout %ds hard_timeout %ds packet_count %lld byte_count %lld %s", dump_flow_p->time, dump_flow_p->LorH, dump_flow_p->type, type1, dump_flow_p->in_port, dump_flow_p->dl_vlan, dump_flow_p->dl_src[0], dump_flow_p->dl_src[1], dump_flow_p->dl_src[2], dump_flow_p->dl_src[3], dump_flow_p->dl_src[4], dump_flow_p->dl_src[5], dump_flow_p->dl_dst[0], dump_flow_p->dl_dst[1], dump_flow_p->dl_dst[2], dump_flow_p->dl_dst[3], dump_flow_p->dl_dst[4], dump_flow_p->dl_dst[5], dump_flow_p->dl_type, (dump_flow_p->nw_src_masked)>>24 & 0xff, (dump_flow_p->nw_src_masked)>>16 & 0xff, (dump_flow_p->nw_src_masked)>>8 & 0xff, (dump_flow_p->nw_src_masked) & 0xff, dump_flow_p->nw_src_mask,(dump_flow_p->nw_dst_masked)>>24 & 0xff, (dump_flow_p->nw_dst_masked)>>16 & 0xff, (dump_flow_p->nw_dst_masked)>>8 & 0xff, (dump_flow_p->nw_dst_masked) & 0xff, dump_flow_p->nw_dst_mask, dump_flow_p->nw_proto, dump_flow_p->nw_tos, dump_flow_p->tp_src, dump_flow_p->tp_dst, dump_flow_p->duration_sec, dump_flow_p->duration_nsec, dump_flow_p->idle_timeout, dump_flow_p->hard_timeout, dump_flow_p->packet_count, dump_flow_p->byte_count, dump_flow_p->actions );

    fbuff_size = sizeof(fbuff);
    printf("%s\n", fbuff);

    fprintf(dump_flow_print_stream,"%s\n",fbuff);  
	printf("\n\n\n");
    fflush(dump_flow_print_stream);    //默认是缓冲模式，因此只有当关闭文件或清空缓冲区时才写入文件。
    //fclose(dump_flow_print_stream);
    return 0;

}

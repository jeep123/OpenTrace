/* @Author: Peicheng Jiang
   @Date  : 05.13.2012
   @Func  : Reconstruct the struct that will be sent
*/

#include "dump_flow_stats.h"
/* Peicheng Jiang insert for periodically dumping flows */
int dump_period_flow(struct sw_flow *flow, uint8_t LorH)
{
	dump_sw_flow(flow, LorH, 'P');
}
/* End on 2012.10.12 */

int dump_sw_flow_flow(struct sw_flow_key *key, uint8_t LorH, uint8_t type_)
{
	struct sw_flow flow_;
	struct sw_flow *flow;
	flow = &flow_;
	memset(flow, 0, sizeof flow_);
	flow->key = *key;

   /*	if( (ntohs(key->flow.in_port) != 65534) && (ntohs(key->flow.dl_type) != 0x0806) && (ntohl(key->flow.nw_src) != 0x0a0100fe) && (ntohl(key->flow.nw_dst) != 0x0a0100fe))   //!!!!!!!!!!!!! this place should be revised, because the compatiable is not good. 10.1.0.254 for udp_traceserver and tcp_nox
		flow->key.flow.dl_vlan = htons(65534);*/

	DENY_FLOW(flow) // here deny some kinds of flow
		return(dump_sw_flow(flow, LorH, type_));
}
int dump_sw_flow(struct sw_flow *flow, /*uiint64_t id,*/ uint8_t LorH, uint8_t type_)
{
    struct flow  cflow_match;
    struct dump_flow *dump_flow_p;
  //  struct dump_flow dump_flow_;
    struct ds ds_;
    char *empty = "";
    int temp;
    uint32_t nw_src_;
    uint32_t nw_src_mask_;
    uint32_t nw_dst_; 
    uint32_t nw_dst_mask_;
    char buff[BUFSIZE];
    uint16_t buf_size = 1;

    cflow_match = flow->key.flow;
    dump_flow_p = (struct dump_flow*)malloc(sizeof*dump_flow_p);
    memset(dump_flow_p->actions, 0, 100);
    ds_.string = NULL;
    ds_.length=0;
    ds_.allocated =  0;
    /* Dump actions */
    if(type_ != 'U')
    {
		ofp_print_actions(&ds_, flow->sf_acts->actions, flow->sf_acts->actions_len);
		buf_size = strlen(ds_.string);
       	memcpy(dump_flow_p->actions, ds_.string, buf_size);
  
    }
    else 
    {
        //dump_flow_p = malloc(sizeof(struct dump_flow) + buf_size);
        memcpy(dump_flow_p->actions, empty, buf_size);
    }
 
    dump_flow_p->time = time_msec();
    dump_flow_p->LorH = LorH;
    dump_flow_p->type = type_;

    dump_flow_p->in_port = ntohs(cflow_match.in_port);
    dump_flow_p->dl_vlan = ntohs(cflow_match.dl_vlan);

    for(temp=0;temp<6;temp++){
  
        dump_flow_p->dl_src[temp] = cflow_match.dl_src[temp];

    }
    temp = 0;
    for(temp=0;temp<6;temp++){
     
        dump_flow_p->dl_dst[temp] = cflow_match.dl_dst[temp];

    }
    /* Judge the dl_type is ARP or IP 
    if(ntohs(cflow_match.dl_type) == 2048)
        printf(" dl_type IP ");
    else if(ntohs(cflow_match.dl_type) == 2054)
        printf("dl_type ARP "); */

    dump_flow_p->dl_type = ntohs(cflow_match.dl_type);

	if(type_ != 'U')
	{
    	nw_src_ = ntohl(cflow_match.nw_src);
    	nw_src_mask_ = ntohl(flow->key.nw_src_mask);
    	nw_src_ = nw_src_ & nw_src_mask_;
    	dump_flow_p->nw_src_masked = nw_src_;
    	dump_flow_p->nw_src_mask = 32-((flow->key.wildcards >> 8) & 0x3f);
   		if(dump_flow_p->nw_src_mask > 32)  // Avoid the dl_type is not set, and the nw_IP is not initialized, so this segment is unlegal.
			dump_flow_p->nw_src_mask = 0;

    	nw_dst_ = ntohl(cflow_match.nw_dst); 
    	nw_dst_mask_ = ntohl(flow->key.nw_dst_mask);
    	nw_dst_ = nw_dst_ & nw_dst_mask_;
    	dump_flow_p->nw_dst_masked = nw_dst_;
    	dump_flow_p->nw_dst_mask = 32-((flow->key.wildcards >> 14) & 0x3f);
    	if(dump_flow_p->nw_dst_mask > 32)  // Avoid the dl_type is not set, and the nw_IP is not initialized, so this segment is unlegal.
			dump_flow_p->nw_dst_mask = 0;
	}
	else
	{
		nw_src_ = ntohl(cflow_match.nw_src);
		dump_flow_p->nw_src_masked = nw_src_;
		dump_flow_p->nw_src_mask = 32-((flow->key.wildcards >> 8) & 0x3f);
   		if(dump_flow_p->nw_src_mask > 32)  // Avoid the dl_type is not set, and the nw_IP is not initialized, so this segment is unlegal.
			dump_flow_p->nw_src_mask = 0;

		nw_dst_ = ntohl(cflow_match.nw_dst); 
		dump_flow_p->nw_dst_masked = nw_dst_;
		dump_flow_p->nw_dst_mask = 32-((flow->key.wildcards >> 14) & 0x3f);
    	if(dump_flow_p->nw_dst_mask > 32)  // Avoid the dl_type is not set, and the nw_IP is not initialized, so this segment is unlegal.
			dump_flow_p->nw_dst_mask = 0;
	}

    dump_flow_p->nw_proto = cflow_match.nw_proto;
    dump_flow_p->nw_tos = cflow_match.nw_tos;

    /* Print transport layer */
    dump_flow_p->tp_src = ntohs(cflow_match.tp_src);
    dump_flow_p->tp_dst = ntohs(cflow_match.tp_dst);

    /* Print some other time */
	if(type_ != 'U')
	{
   		dump_flow_p->duration_sec = (dump_flow_p->time - flow->created)/1000; //Attention!! the flow->created may wrong for msec or sec
   		dump_flow_p->duration_nsec = (dump_flow_p->time - flow->used); 
    	dump_flow_p->idle_timeout = flow->idle_timeout;
    	dump_flow_p->hard_timeout = flow->hard_timeout;
	  
    	/* Print packets count and bytes counts  */
    	dump_flow_p->packet_count = (flow->packet_count);
    	dump_flow_p->byte_count = (flow->byte_count);
	}
	else
	{
		dump_flow_p->duration_sec = 0;
   		dump_flow_p->duration_nsec = 0;
    	dump_flow_p->idle_timeout = 0;
    	dump_flow_p->hard_timeout = 0;
	  
    	/* Print packets count and bytes counts  */
    	dump_flow_p->packet_count = 0;
    	dump_flow_p->byte_count = 0;
	}

    dump_flow_p->wildcards = flow->key.wildcards;
/* Peicheng Jiang insert for the total dump */
    if( number < total_number )
    {
        list_insert(&total_dump_list, &dump_flow_p->total_dump_flow);
        number++;
//printf("#############\n%d\n", number);
    } 
    else
    {
        list_insert(&total_dump_list, &dump_flow_p->total_dump_flow);
        number++;
        struct dump_flow *f, *n;
        LIST_FOR_EACH_SAFE(f, n, struct dump_flow, total_dump_flow, &total_dump_list)
        {
            temp = flow_print_(f);
            number--;
//printf("****************\n%d\n",number);
            list_remove(&f->total_dump_flow);
            free(f);
        }
    }

    /* Print */
   // temp = flow_print_(dump_flow_p);

#if 0    
    /* Dump flow to the remote machine */
    memset(buff, 0, BUFSIZE);
    memcpy(buff, dump_flow_p, sizeof(*dump_flow_p));
    temp = udp_socket(buff, sizeof(*dump_flow_p));
    if(temp != 0)
	{
        printf("socket error\n");
        return 1;
    }

#endif
    return 0;
}

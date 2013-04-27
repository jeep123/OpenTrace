/* @Author: Peicheng Jiang
   @Date  : 05.13.2012
   @Func  : Reconstruct the struct that will be sent
*/
#ifndef DUMP_FLOW_STATS_H 
#define DUMP_FLOW_STATS_H 1

#include <string.h>
#include "../../lib/flow.h"
#include "../switch-flow.h"
#include "dump_flow_struct.h"
#include "dump_flow_client.h"
#include "dump_flow_print.h"
#include "../../lib/timeval.h"
#include "../../lib/list.h"
#include "../../lib/ofp-print.h"
#include "../../lib/dynamic-string.h"


#define DENY_FLOW(flow) if(!( ((ntohl(flow->key.flow.nw_dst))>>24 & 0xff) == 224 && ((ntohl(flow->key.flow.nw_dst))>>16 & 0xff) == 0 && ((ntohl(flow->key.flow.nw_dst))>>8 & 0xff) == 0))

int dump_sw_flow(struct sw_flow *flow, /*uint64_t id,*/ uint8_t LorH, uint8_t type_);
int dump_sw_flow_flow(struct sw_flow_key *key, uint8_t LorH, uint8_t type_);  //used for unmatched flow
int dump_period_flow(struct sw_flow *flow, uint8_t LorH);
extern int total_number;
extern struct list total_dump_list;
int number;

#endif

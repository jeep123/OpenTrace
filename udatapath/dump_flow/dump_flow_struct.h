/* @Author: Peicheng Jiang
   @Date  : 05.17.2012
   @Func  : dumped flows' struct 
*/
/* Modified by Peicheng Jiang @05.18.2012 */
#ifndef DUMP_FLOW_STRUCT_H
#define DUMP_FLOW_STRUCT_H 1

#include <stdint.h>
#include "list.h"
#define BUFSIZE 1000

struct dump_flow {
    uint64_t time;
    uint64_t id;
    uint8_t LorH;   // from table-linear or table-hash
    uint8_t type;   // dump for Insert, Delete or Timeout (I, D, T)

    uint16_t in_port;
    uint16_t dl_vlan;
    uint8_t dl_src[6];
    uint8_t dl_dst[6];
    uint16_t dl_type;
    uint32_t nw_src_masked;
    uint8_t nw_src_mask;
    uint32_t nw_dst_masked;
    uint8_t nw_dst_mask;
    uint8_t nw_proto;
    uint8_t nw_tos;
    uint16_t tp_src;
    uint16_t tp_dst;
   
    uint32_t wildcards;   // what
    uint64_t duration_sec;
    uint64_t duration_nsec;
    uint16_t idle_timeout;
    uint16_t hard_timeout;
    uint64_t packet_count;
    uint64_t byte_count;

    char actions[100];

    struct list total_dump_flow; // used for the accumulative dump flows
};

#endif

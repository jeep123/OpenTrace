/* @Author: Peicheng Jiang
   @Date  : 05.13.2012
   @Func  : Print the flow and store it in the local filesystem
*/
#ifndef DUMP_FLOW_PRINT_H
#define DUMP_FLOW_PRINT_H 1

#include "dump_flow_struct.h"
#include <stdio.h>

//#define WRITE_FILE "dump_flow/dumped_flow.log"

int flow_print_(struct dump_flow *dump_flow_p);

extern FILE *dump_flow_print_stream;
#endif


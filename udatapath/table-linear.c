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
#include "table.h"
#include <stdlib.h>
#include "flow.h"
#include "list.h"
#include "openflow/openflow.h"
#include "openflow/nicira-ext.h"
#include "switch-flow.h"
#include "datapath.h"
/* Peicheng Jiang insert for refer ntohll()*/

#include "dump_flow/dump_flow_stats.h"

/* End 05.13.2012 by Peicheng Jiang */

/* Peicheng Jiang insert for periodically dumping flows */
//extern int dump_period; //Introduce the global parameter
//extern uint64_t last_dump_time; //Introduce the glocal parameter
extern bool JS;
extern int PIDMT[5];
/* End on 2012.10.12 */

struct sw_table_linear {
    struct sw_table swt;

    unsigned int max_flows;
    unsigned int n_flows;
    struct list flows;
    struct list iter_flows;
    unsigned long int next_serial;
};

static struct sw_flow *table_linear_lookup(struct sw_table *swt,
                                           const struct sw_flow_key *key)
{
    struct sw_table_linear *tl = (struct sw_table_linear *) swt;
    struct sw_flow *flow;
    LIST_FOR_EACH (flow, struct sw_flow, node, &tl->flows) {
        if (flow_matches_1wild(key, &flow->key))
            return flow;
    }
    return NULL;
}

static int table_linear_insert(struct sw_table *swt, struct sw_flow *flow)
{
    struct sw_table_linear *tl = (struct sw_table_linear *) swt;
    struct sw_flow *f;

    /* Peicheng Jiang Insert for dump */

    uint8_t type_ = 'I';

    /* END 05.13.2012 by Peicheng Jiang */

    /* Loop through the existing list of entries.  New entries will
     * always be placed behind those with equal priority.  Just replace 
     * any flows that match exactly.
     */
    LIST_FOR_EACH (f, struct sw_flow, node, &tl->flows) {
        if (f->priority == flow->priority
                && f->key.wildcards == flow->key.wildcards
                && flow_matches_2wild(&f->key, &flow->key)) {
            flow->serial = f->serial;
            list_replace(&flow->node, &f->node);
            list_replace(&flow->iter_node, &f->iter_node);
            flow_free(f);
// for test
//printf("Hi, I am called!->>> %lld\n", flow->packet_count);
            return 1;
        }

        if (f->priority < flow->priority)
            break;
    }

    /* Make sure there's room in the table. */
    if (tl->n_flows >= tl->max_flows) {
        return 0;
    }
    tl->n_flows++;

    /* Insert the entry immediately in front of where we're pointing. */
    flow->serial = tl->next_serial++;
    list_insert(&f->node, &flow->node);
    list_push_front(&tl->iter_flows, &flow->iter_node);
    
    
    /* Peicheng Jiang Insert for dump */

   // //dump_sw_flow(flow, 'L', type_);

    /* END 05.13.2012 by Peicheng Jiang */

    return 1;
}



/* End on 05.15.2012 by Peicheng Jiang */

static int table_linear_modify(struct sw_table *swt,
                const struct sw_flow_key *key, uint16_t priority, int strict,
                const struct ofp_action_header *actions, size_t actions_len)
{
    struct sw_table_linear *tl = (struct sw_table_linear *) swt;
    struct sw_flow *flow;
    unsigned int count = 0;

    LIST_FOR_EACH (flow, struct sw_flow, node, &tl->flows) {
        if (flow_matches_desc(&flow->key, key, strict)
                && (!strict || (flow->priority == priority))) {
            if(PIDMT[3] == 1){
                dump_sw_flow(flow,'L','m'); //before modified
            }
	    flow_replace_acts(flow, actions, actions_len);
            if(PIDMT[3] == 1){
	        dump_sw_flow(flow,'L','M'); //after modified
            }
            count++;
        }
    }
    return count;
}

static int table_linear_has_conflict(struct sw_table *swt,
                                     const struct sw_flow_key *key,
                                     uint16_t priority, int strict)
{
    struct sw_table_linear *tl = (struct sw_table_linear *) swt;
    struct sw_flow *flow;

    LIST_FOR_EACH (flow, struct sw_flow, node, &tl->flows) {
        if (flow_matches_2desc(&flow->key, key, strict)
                && (flow->priority == priority)) {
            return true;
        }
    }
    return false;
}

static void
do_delete(struct sw_flow *flow) 
{
    list_remove(&flow->node);
    list_remove(&flow->iter_node);
    flow_free(flow);
}

static int table_linear_delete(struct datapath *dp, struct sw_table *swt,
                               const struct sw_flow_key *key, 
                               uint16_t out_port, 
                               uint16_t priority, int strict)
{
    struct sw_table_linear *tl = (struct sw_table_linear *) swt;
    struct sw_flow *flow, *n;
    unsigned int count = 0;
    uint8_t type_ = 'D';

    LIST_FOR_EACH_SAFE (flow, n, struct sw_flow, node, &tl->flows) {
        if (flow_matches_desc(&flow->key, key, strict)
                && flow_has_out_port(flow, out_port)
                && (!strict || (flow->priority == priority))) {
            //printf("[CC-Testing] A flow has been deleted! in_port %d\n", ntohs(flow->key.flow.in_port)); 

            /* Peicheng Jiang insert for dump flows */
            if(PIDMT[2] == 1){
	        dump_sw_flow(flow, 'L', type_);         // This function should be set before do_delete(), otherwise the flow's IP segment will be wrong.
            }
 	    /* End on 05.15.2012 by Peicheng Jiang */

      	    dp_send_flow_end(dp, flow, OFPRR_DELETE);
            do_delete(flow);

            count++;
        }
    }
    tl->n_flows -= count;
    return count;
}

static void table_linear_timeout(struct sw_table *swt, struct list *deleted)
{
    struct sw_table_linear *tl = (struct sw_table_linear *) swt;
    struct sw_flow *flow, *n;
    uint8_t type_ = 'T';
#if 0
	/* Peicheng Jiang insert for periodically duming flows */
	uint64_t Jtime_now; //james insert for record the time 
	bool JS = false;
	Jtime_now = time_msec();
printf("now is %lld\n", Jtime_now);
printf("last is %lld and period is %d === %lld\n", last_dump_time, dump_period, last_dump_time+dump_period);
	if (Jtime_now >= last_dump_time + dump_period)
	{
		JS = true;
printf("dumping........\n");
		last_dump_time = Jtime_now;
	}
	/* End on 2012.10.12 */
#endif

	/* Peicheng Jiang modified for periodically dumping flows */
	if (JS == false)
	{
    	LIST_FOR_EACH_SAFE (flow, n, struct sw_flow, node, &tl->flows) {
            if (flow_timeout(flow)) {
                list_remove(&flow->node);
                list_remove(&flow->iter_node);
                list_push_back(deleted, &flow->node);
                /* Peicheng Jiang insert for dump flows */
                if(PIDMT[4] == 1){
	            dump_sw_flow(flow, 'L', type_);
                }
 	        /* End on 05.15.2012 by Peicheng Jiang */
                tl->n_flows--;
            }    
        }
	}
	else
	{
		
    	LIST_FOR_EACH_SAFE (flow, n, struct sw_flow, node, &tl->flows) {
			dump_period_flow(flow, 'L'); // dump flows
            if (flow_timeout(flow)) {
                list_remove(&flow->node);
                list_remove(&flow->iter_node);
                list_push_back(deleted, &flow->node);
                if(PIDMT[4] == 1){
                    dump_sw_flow(flow, 'L', type_);
                }
                tl->n_flows--;
            }    
        }
	}
	/* End on 2012.10.12 */
}

static void table_linear_destroy(struct sw_table *swt)
{
    struct sw_table_linear *tl = (struct sw_table_linear *) swt;

    while (!list_is_empty(&tl->flows)) {
        struct sw_flow *flow = CONTAINER_OF(list_front(&tl->flows),
                                            struct sw_flow, node);
        list_remove(&flow->node);
        flow_free(flow);
    }
    free(tl);
}

static int table_linear_iterate(struct sw_table *swt,
                                const struct sw_flow_key *key,
                                uint16_t out_port,
                                struct sw_table_position *position,
                                int (*callback)(struct sw_flow *, void *),
                                void *private)
{
    struct sw_table_linear *tl = (struct sw_table_linear *) swt;
    struct sw_flow *flow;
    unsigned long start;

    start = ~position->private[0];
    LIST_FOR_EACH (flow, struct sw_flow, iter_node, &tl->iter_flows) {
        if (flow->serial <= start 
                && flow_matches_2wild(key, &flow->key)
                && flow_has_out_port(flow, out_port)) {
            int error = callback(flow, private);
            if (error) {
                position->private[0] = ~(flow->serial - 1);
                return error;
            }
        }
    }
    return 0;
}

static void table_linear_stats(struct sw_table *swt,
                               struct sw_table_stats *stats)
{
    struct sw_table_linear *tl = (struct sw_table_linear *) swt;
    stats->name = "linear";
    stats->wildcards = OFPFW_ALL;
    stats->n_flows   = tl->n_flows;
    stats->max_flows = tl->max_flows;
    stats->n_lookup  = swt->n_lookup;
    stats->n_matched = swt->n_matched;
}


struct sw_table *table_linear_create(unsigned int max_flows)
{
    struct sw_table_linear *tl;
    struct sw_table *swt;

    tl = calloc(1, sizeof *tl);
    if (tl == NULL)
        return NULL;

    swt = &tl->swt;
    swt->lookup = table_linear_lookup;
    swt->insert = table_linear_insert;
    swt->modify = table_linear_modify;
    swt->has_conflict = table_linear_has_conflict;
    swt->delete = table_linear_delete;
    swt->timeout = table_linear_timeout;
    swt->destroy = table_linear_destroy;
    swt->iterate = table_linear_iterate;
    swt->stats = table_linear_stats;

    tl->max_flows = max_flows;
    tl->n_flows = 0;
    list_init(&tl->flows);
    list_init(&tl->iter_flows);
    tl->next_serial = 0;

    return swt;
}

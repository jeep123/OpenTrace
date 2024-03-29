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
#include "chain.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include "switch-flow.h"
#include "table.h"
/* Peicheng Jiang insert*/
#include "dump_flow/dump_flow_stats.h"
extern uint64_t last_dump_time;
extern int dump_period;
extern bool JS;
uint64_t Jtime_now;
/* End */
#define THIS_MODULE VLM_chain
#include "vlog.h"

/* Attempts to append 'table' to the set of tables in 'chain'.  Returns 0 or
 * negative error.  If 'table' is null it is assumed that table creation failed
 * due to out-of-memory. */
static int add_table(struct sw_chain *chain, struct sw_table *table, int emerg)
{
    if (table == NULL)
        return -ENOMEM;
    if (chain->n_tables >= CHAIN_MAX_TABLES) {
        VLOG_ERR("too many tables in chain\n");
        table->destroy(table);
        return -ENOBUFS;
    }
    if (emerg)
        chain->emerg_table = table;
    else
        chain->tables[chain->n_tables++] = table;
    return 0;
}

/* Creates and returns a new chain.  Returns NULL if the chain cannot be
 * created. */
struct sw_chain *chain_create(struct datapath *dp)
{
    struct sw_chain *chain = calloc(1, sizeof *chain);
    if (chain == NULL)
        return NULL;

    chain->dp = dp;
    if (add_table(chain, table_hash2_create(0x1EDC6F41, TABLE_HASH_MAX_FLOWS,
                                            0x741B8CD7, TABLE_HASH_MAX_FLOWS),
                                            0)
        || add_table(chain, table_linear_create(TABLE_LINEAR_MAX_FLOWS), 0)
        || add_table(chain, table_linear_create(TABLE_LINEAR_MAX_FLOWS), 1)) {
        chain_destroy(chain);
        return NULL;
    }

    return chain;
}

/* Searches 'chain' for a flow matching 'key', which must not have any wildcard
 * fields.  Returns the flow if successful, otherwise a null pointer. */
struct sw_flow *
chain_lookup(struct sw_chain *chain, const struct sw_flow_key *key, int emerg)
{
    int i;

    assert(!key->wildcards);

    if (emerg) {
        struct sw_table *t = chain->emerg_table;
        struct sw_flow *flow = t->lookup(t, key);
        t->n_lookup++;
        if (flow) {
            t->n_matched++;
            return flow;
        }
    } else {
        for (i = 0; i < chain->n_tables; i++) {
            struct sw_table *t = chain->tables[i];
            struct sw_flow *flow = t->lookup(t, key);
            t->n_lookup++;
            if (flow) {
                t->n_matched++;
                return flow;
            }
        }
    }

    return NULL;
}

/* Inserts 'flow' into 'chain', replacing any duplicate flow.  Returns 0 if
 * successful or a negative error.
 *
 * If successful, 'flow' becomes owned by the chain, otherwise it is retained
 * by the caller. */
int
chain_insert(struct sw_chain *chain, struct sw_flow *flow, int emerg)
{
    int i;

    if (emerg) {
        struct sw_table *t = chain->emerg_table;
        if (t->insert(t, flow))
		{
			/* Peicheng Jiang insert */	
		//	dump_sw_flow(flow, 'E', 'I');
			/* End in 6.4.2012 */
            return 0;
		}
    } else {
        for (i = 0; i < chain->n_tables; i++) {
            struct sw_table *t = chain->tables[i];
            if (t->insert(t, flow))
			{
				/* Peicheng Jiang insert */
				//if(ntohs(flow->key.flow.tp_dst) != 6666)
		          if(PIDMT[1] == 1){
				DENY_FLOW(flow)
				{
					if(i == 0)
						dump_sw_flow(flow, 'H', 'I');
					else if( i == 1)
						dump_sw_flow(flow, 'L', 'I');
				}
                            }
				/* End in 6.4.2012 */
                return 0;
			}
        }
    }

    return -ENOBUFS;
}

/* Modifies actions in 'chain' that match 'key'.  If 'strict' set, wildcards 
 * and priority must match.  Returns the number of flows that were modified.
 *
 * Expensive in the general case as currently implemented, since it requires
 * iterating through the entire contents of each table for keys that contain
 * wildcards.  Relatively cheap for fully specified keys. */
int
chain_modify(struct sw_chain *chain, const struct sw_flow_key *key,
        uint16_t priority, int strict,
        const struct ofp_action_header *actions, size_t actions_len,
        int emerg)
{
    int count = 0;
    int i;

    if (emerg) {
        struct sw_table *t = chain->emerg_table;
        count += t->modify(t, key, priority, strict, actions, actions_len);
    } else {
        for (i = 0; i < chain->n_tables; i++) {
            struct sw_table *t = chain->tables[i];
            count += t->modify(t, key, priority, strict, actions, actions_len);
        }
    }

    return count;
}

/* Checks whether the chain has an entry with the same priority which conflicts
 * with 'key'. If 'strict' set, wildcards should also match. If 'strict' is not 
 * set, comparison is done 'module wildcards'.
 *
 * Returns 'true' if such an entry exists, 'false' otherwise. */
int
chain_has_conflict(struct sw_chain *chain, const struct sw_flow_key *key,
                   uint16_t priority, int strict)
{
    int i;

    for (i = 0; i < chain->n_tables; i++) {
        struct sw_table *t = chain->tables[i];
        if (t->has_conflict(t, key, priority, strict)) {
            return true;
        }
    }

    return false;
}

/* Deletes from 'chain' any and all flows that match 'key'.  If 'out_port' 
 * is not OFPP_NONE, then matching entries must have that port as an 
 * argument for an output action.  If 'strict" is set, then wildcards and 
 * priority must match.  Returns the number of flows that were deleted.
 *
 * Expensive in the general case as currently implemented, since it requires
 * iterating through the entire contents of each table for keys that contain
 * wildcards.  Relatively cheap for fully specified keys. */
int
chain_delete(struct sw_chain *chain, const struct sw_flow_key *key,
             uint16_t out_port, uint16_t priority, int strict, int emerg)
{
    int count = 0;
    int i;

    if (emerg) {
        struct sw_table *t = chain->emerg_table;
        count += t->delete(chain->dp, t, key, out_port, priority, strict);
    } else {
        for (i = 0; i < chain->n_tables; i++) {
            struct sw_table *t = chain->tables[i];
            count += t->delete(chain->dp, t, key, out_port, priority, strict);
        }
    }

    return count;
}

/* Deletes timed-out flow entries from all the tables in 'chain' and appends
 * the deleted flows to 'deleted'.
 *
 * Expensive as currently implemented, since it iterates through the entire
 * contents of each table. */
void
chain_timeout(struct sw_chain *chain, struct list *deleted)
{
    int i;
    Jtime_now = time_msec();
    JS = false;
    if ((Jtime_now >= last_dump_time + dump_period) && (PIDMT[0] == 1))
    {
	JS = true;
	last_dump_time = Jtime_now;
    } 
    for (i = 0; i < chain->n_tables; i++) {
        struct sw_table *t = chain->tables[i];
        t->timeout(t, deleted);
    }
}

/* Destroys 'chain', which must not have any users. */
void
chain_destroy(struct sw_chain *chain)
{
    int i;
    struct sw_table *t;

    for (i = 0; i < chain->n_tables; i++) {
        t = chain->tables[i];
        t->destroy(t);
    }
    t = chain->emerg_table;
    t->destroy(t);
    free(chain);
}

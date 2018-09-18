/* ------------------------------------------------------------------------
 *
 * partition_update.h
 *		Insert row to right partition in UPDATE operation
 *
 * Copyright (c) 2017, Postgres Professional
 *
 * ------------------------------------------------------------------------
 */

#ifndef PARTITION_UPDATE_H
#define PARTITION_UPDATE_H

#include "relation_info.h"
#include "utils.h"

#include "postgres.h"
#include "commands/explain.h"
#include "optimizer/planner.h"

#if PG_VERSION_NUM >= 90600
#include "nodes/extensible.h"
#endif


#define UPDATE_NODE_NAME "PartitionRouter"


typedef struct PartitionRouterState
{
	CustomScanState		css;

	Plan			   *subplan;		/* proxy variable to store subplan */
	ExprState		   *constraint;		/* should tuple remain in partition? */
	JunkFilter		   *junkfilter;		/* 'ctid' extraction facility */
	ResultRelInfo	   *current_rri;

	/* Machinery required for EvalPlanQual */
	EPQState			epqstate;
	int					epqparam;

	/* Preserved slot from last call */
	bool				yielded;
	TupleTableSlot	   *yielded_slot;

	/* Need these for a GREAT deal of hackery */
	ModifyTableState   *mt_state;
	bool				update_stmt_triggers,
						insert_stmt_triggers;
} PartitionRouterState;


extern bool					pg_pathman_enable_partition_router;

extern CustomScanMethods	partition_router_plan_methods;
extern CustomExecMethods	partition_router_exec_methods;


#define IsPartitionRouterPlan(node) \
	( \
		IsA((node), CustomScan) && \
		(((CustomScan *) (node))->methods == &partition_router_plan_methods) \
	)

#define IsPartitionRouterState(node) \
	( \
		IsA((node), CustomScanState) && \
		(((CustomScanState *) (node))->methods == &partition_router_exec_methods) \
	)

#define IsPartitionRouter(node) \
	( IsPartitionRouterPlan(node) || IsPartitionRouterState(node) )


void init_partition_router_static_data(void);

Plan *make_partition_router(Plan *subplan, int epq_param);

void prepare_modify_table_for_partition_router(PlanState *state, void *context);


Node *partition_router_create_scan_state(CustomScan *node);

void partition_router_begin(CustomScanState *node, EState *estate, int eflags);

TupleTableSlot *partition_router_exec(CustomScanState *node);

void partition_router_end(CustomScanState *node);

void partition_router_rescan(CustomScanState *node);

void partition_router_explain(CustomScanState *node,
							  List *ancestors,
							  ExplainState *es);

TupleTableSlot *partition_router_run_modify_table(PlanState *state);

#endif /* PARTITION_UPDATE_H */

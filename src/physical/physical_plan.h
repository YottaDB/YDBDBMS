/****************************************************************
 *								*
 * Copyright (c) 2019-2021 YottaDB LLC and/or its subsidiaries.	*
 * All rights reserved.						*
 *								*
 *	This source code contains the intellectual property	*
 *	of its copyright holder(s), and is made available	*
 *	under a license.  If you do not know the terms of	*
 *	the license, please stop and do not read further.	*
 *								*
 ****************************************************************/

#ifndef PHYSICAL_PLAN
#define PHYSICAL_PLAN

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "octo.h"
#include "octo_types.h"

#include "logical_plan.h"

typedef enum PPActionType { PP_PROJECT, PP_DELETE, PP_ADVANCE } PPActionType;

// Some SET operations require special post-condition stuff;
//  this allows us to track what set operation we are doing in
//  in the physical plan
typedef enum PPSetOperation { PP_NOT_SET, PP_UNION_SET, PP_EXCEPT_SET, PP_INTERSECT_SET } PPSetOperation;

typedef struct SetOperType {
	LPActionType	    set_oper_type;
	int		    input_id1;
	int		    input_id2;
	int		    output_id;
	struct SetOperType *prev;
	struct SetOperType *next;
} SetOperType;

typedef struct PhysicalPlan {
	char *		     plan_name, *filename, *trigger_name;
	struct PhysicalPlan *prev, *next;
	// These represent the keys we used to do the iteration
	SqlKey *	    iterKeys[MAX_KEY_COUNT];
	SqlKey *	    outputKey;
	LogicalPlan *	    where;	       /* WHERE clause */
	LogicalPlan *	    tablejoin;	       /* FROM clause */
	LogicalPlan *	    aggregate_options; /* GROUP BY and HAVING */
	SqlOptionalKeyword *keywords;	       /* DISTINCT etc. */
	LogicalPlan *	    order_by;	       /* ORDER BY clause */
	LogicalPlan *	    projection;
	unsigned int	    total_iter_keys;
	// If set to 1, this plan should emit the columns as subscripts of the key,
	//  rather than using a row id
	boolean_t stash_columns_in_keys;
	// If true, this plan outputs a cross reference key, and should be treated thusly
	boolean_t is_cross_reference_key;

	// If true, maintain a column-wise index of known types
	boolean_t maintain_columnwise_index;
	// If true, only add the value to the output key if it doesn't already exist in
	//  the columnwise index; requires the columnwise index
	boolean_t distinct_values;

	/* If non-NULL, this plan should not be emitted in order but waited until after all required
	 * plans have been emitted first. This is important when the plan will not run in-order,
	 * but will be called as a subquery which can't be extracted due to a parent reference.
	 * This field points to a parent/ancestor physical plan that will need to generate a call to
	 * this plan as part of going through every result row of that parent/ancestor physical plan.
	 */
	struct PhysicalPlan *deferred_parent_plan;
	// Points to the parent plan of this plan; we need this so we can resolve
	//   references to parent queries and mark intermediate plans as deferred
	struct PhysicalPlan *parent_plan;
	// If true, we should emit code to ensure only one value gets inserted to the output key
	// for a given set of keys
	boolean_t  emit_duplication_check;
	boolean_t *treat_key_as_null;		/* Set to TRUE for a short period when generating M code for
						 * the case where a left table row did not match any right
						 * table row in an OUTER JOIN (LEFT/RIGHT/FULL). If TRUE, any
						 * references to columns from this table are automatically
						 * treated as NULL (replaced with "") in the generated M code.
						 */
	boolean_t tablejoin_body_group_by_done; /* FALSE initially. Set to TRUE in "tmpl_physical_plan"
						 * once "tmpl_tablejoin_body_group_by" call is done
						 * but "tmpl_group_by" call has not happened.
						 */
	boolean_t	     aggregate_function_or_group_by_specified;
	SetOperType *	     set_oper_list;	  /* Linked list of SET OPERATIONS to do on this plan at the end */
	struct PhysicalPlan *dnf_prev, *dnf_next; /* Linked list of plans that are at the same LP_SET_DNF level */
	LogicalPlan *	     lp_select_query;	  /* The owning LP_SELECT_QUERY or LP_TABLE_VALUE or LP_INSERT_INTO
						   * logical plan corresponding to this physical plan.
						   */
} PhysicalPlan;

/* Below macro returns TRUE if GROUP BY or HAVING have been specified and/or Aggregate functions have been used in plan */
#define IS_GROUP_BY_PLAN(PLAN) (PLAN->aggregate_function_or_group_by_specified)

/* The below macros take into account that multiple physical plans can correspond to the same logical plan in which case
 * the physical plan name would have only been filled in one of those duplicates (the one whose logical plan points
 * back to this physical plan).
 */
#define PRIMARY_PHYSICAL_PLAN(PLAN) PLAN->lp_select_query->extra_detail.lp_select_query.physical_plan
#define PHYSICAL_PLAN_NAME(PLAN)    PRIMARY_PHYSICAL_PLAN(PLAN)->plan_name

#define IS_INSERT_INTO_PHYSICAL_PLAN(PPLAN) ((NULL != PPLAN->lp_select_query) && (LP_INSERT_INTO == PPLAN->lp_select_query->type))
#define HYPHEN_LINE			    "---------------------------------------------------------"

// This provides a convenient way to pass options to subplans
// which need to be aware of a request from a higher level
typedef struct PhysicalPlanOptions {
	struct PhysicalPlan * parent;
	struct PhysicalPlan **last_plan;
	struct PhysicalPlan * dnf_plan_next;
	boolean_t	      stash_columns_in_keys;
	LogicalPlan **	      aggregate; /* Helps maintain a linked list of LP_AGGREGATE_FUNC* plans in each query.
					  * Subqueries inside the query maintain their own linked list.
					  */
	LogicalPlan **function;		 /* Helps maintain a linked list of LP_FUNCTION_CALL plans across entire query */
	LogicalPlan **table;		 /* Helps maintain a linked list of LP_TABLE plans across entire query */
} PhysicalPlanOptions;

PhysicalPlan *generate_physical_plan(LogicalPlan *plan, PhysicalPlanOptions *options);

/* Allocate and initialize (a few fields) a physical plan. Returns the allocated physical plan. */
PhysicalPlan *allocate_physical_plan(LogicalPlan *plan, PhysicalPlan *pplan_from_lp, PhysicalPlanOptions *plan_options,
				     PhysicalPlanOptions *orig_plan_options);

// Outputs physical plans to temporary files located in config.plan_src_dir
//  Names are like ppplanXXXX, where XXXX is a unique number
// Returns TRUE on success
int emit_physical_plan(PhysicalPlan *pplan, char *plan_filename);

// Returns true if the key is a version of this column
int key_equals_column(SqlKey *key, SqlColumn *column);

PhysicalPlan *get_physical_plan_from_unique_id(PhysicalPlan *pplan, int unique_id);
PhysicalPlan *emit_sql_statement(SqlStatement *stmt, char *plan_filename);
int emit_physical_or_xref_plan(char *plan_filename, SqlStatement *stmt, char *tableName, char *columnName, PhysicalPlan *xref_plan);
int emit_xref_plan(char *plan_filename, char *tableName, char *columnName, PhysicalPlan *xref_plan);

#endif

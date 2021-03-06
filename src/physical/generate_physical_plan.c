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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "octo.h"
#include "octo_types.h"
#include "physical_plan.h"
#include "lp_verify_structure.h"

#include "template_helpers.h"

void	     gen_source_keys(PhysicalPlan *out, LogicalPlan *plan);
void	     iterate_keys(PhysicalPlan *out, LogicalPlan *plan);
LogicalPlan *sub_query_check_and_generate_physical_plan(PhysicalPlanOptions *options, LogicalPlan *stmt, LogicalPlan *parent);

PhysicalPlan *generate_physical_plan(LogicalPlan *plan, PhysicalPlanOptions *options) {
	SqlOptionalKeyword *keywords, *keyword;
	LogicalPlan *	    keys, *table_joins, *select, *insert_or_set_operation, *output_key, *output;
	LogicalPlan *	    project, *select_column_list;
	LogicalPlan *	    criteria, *select_options, *select_more_options;
	LogicalPlan *	    where;
	LogicalPlan *	    set_option, *set_plans, *set_output, *set_key;
	PhysicalPlan *	    out, *prev = NULL, *pp_from_lp;
	LPActionType	    set_oper_type, type;
	PhysicalPlanOptions plan_options;
	boolean_t	    is_lp_table_value, is_set_dnf;
	SqlTableAlias *	    root_table_alias;

	plan_options = *options;
	// If this is a union plan, construct physical plans for the two children
	if (LP_SET_OPERATION == plan->type) {
		GET_LP(set_option, plan, 0, LP_SET_OPTION);
		GET_LP(set_plans, plan, 1, LP_PLANS);
		out = generate_physical_plan(set_plans->v.lp_default.operand[1], &plan_options);
		if (NULL == out) {
			return NULL;
		}
		assert(NULL != out->lp_select_query);
		set_oper_type = set_option->v.lp_default.operand[0]->type;
		assert((LP_SET_UNION == set_oper_type) || (LP_SET_UNION_ALL == set_oper_type) || (LP_SET_DNF == set_oper_type)
		       || (LP_SET_EXCEPT == set_oper_type) || (LP_SET_EXCEPT_ALL == set_oper_type)
		       || (LP_SET_INTERSECT == set_oper_type) || (LP_SET_INTERSECT_ALL == set_oper_type));
		is_set_dnf = (LP_SET_DNF == set_oper_type);
		if (is_set_dnf) {
			PhysicalPlan *next, *tmp;

			tmp = out;
			if (LP_SET_OPERATION == set_plans->v.lp_default.operand[1]->type) {
				/* Need to get the left most DNF sibling of out */
				assert(NULL != tmp->dnf_prev);
				do {
					next = tmp;
					tmp = next->dnf_prev;
				} while (NULL != tmp);
			} else {
				next = out;
			}
			plan_options.dnf_plan_next = next; /* this helps set prev->dnf_next inside the below nested call */
		}
		prev = generate_physical_plan(set_plans->v.lp_default.operand[0], &plan_options);
		if (NULL == prev) {
			return NULL;
		}
		assert(NULL != prev->lp_select_query);
		if (!is_set_dnf) {
			int	     input_id1, input_id2;
			SetOperType *set_oper, *prev_oper, *out_oper, *next_oper;

			OCTO_CMALLOC_STRUCT(set_oper, SetOperType);
			prev_oper = prev->set_oper_list;
			input_id1 = (NULL == prev_oper) ? prev->outputKey->unique_id : prev_oper->output_id;
			assert(input_id1 != out->outputKey->unique_id);
			out_oper = out->set_oper_list;
			input_id2 = (NULL == out_oper) ? out->outputKey->unique_id : out_oper->output_id;
			set_oper->set_oper_type = set_oper_type;
			set_oper->input_id1 = input_id1;
			set_oper->input_id2 = input_id2;
			GET_LP(set_output, set_option, 1, LP_OUTPUT);
			GET_LP(set_key, set_output, 0, LP_KEY);
			set_oper->output_id = set_key->v.lp_key.key->unique_id;
			set_oper->prev = NULL;
			next_oper = out->set_oper_list;
			set_oper->next = next_oper;
			if (NULL != next_oper) {
				next_oper->prev = set_oper;
			}
			out->set_oper_list = set_oper;
		}
		return out;
	} else if (LP_INSERT_INTO == plan->type) {
		PhysicalPlan *src, *dst;
		LogicalPlan * lp_insert_into_options;

		/* Generate a separate physical plan for destination table of the INSERT INTO */
		dst = allocate_physical_plan(plan, NULL, &plan_options, options);
		/* And then generate a separate physical plan for source table/query of the INSERT INTO */
		GET_LP(lp_insert_into_options, plan, 1, LP_INSERT_INTO_OPTIONS);
		src = generate_physical_plan(lp_insert_into_options->v.lp_default.operand[1], &plan_options);
		if (NULL == src) {
			return NULL;
		}
		return dst;
	}
	assert((LP_SELECT_QUERY == plan->type) || (LP_TABLE_VALUE == plan->type));
	is_lp_table_value = (LP_TABLE_VALUE == plan->type);
	/* Note that it is possible a physical plan has already been generated for this logical plan.
	 *
	 * For example "select * from names where EXISTS (select * from names) and (id < 3 or id > 4);" would get expanded as
	 *	"select * from names where EXISTS (select * from names) and (id < 3)"
	 *		OR
	 *	"select * from names where EXISTS (select * from names) and (id > 4)"
	 * The sub-query corresponding to "EXISTS (select * from names)" would generate 2 physical plans but point to
	 * the same logical plan.
	 *
	 * We have to generate the physical plan though as that is how we ensure the corresponding M code gets called twice.
	 * So we cannot easily avoid the physical plan duplication. But we note the fact that this is a duplicate that way we
	 * skip code generation for duplicate plans and ensure the M code that gets emitted does not have duplicate code.
	 */
	pp_from_lp = plan->extra_detail.lp_select_query.physical_plan;
	if (NULL == pp_from_lp) {
		if (!is_lp_table_value) {
			/* Make sure LP_SELECT_QUERY plan is in good shape.
			 * Overload this plan verify phase to also fix aggregate function counts.
			 */
			assert(NULL == plan->extra_detail.lp_select_query.first_aggregate);
			plan_options.aggregate = &plan->extra_detail.lp_select_query.first_aggregate;
			if (FALSE == lp_verify_structure(plan, &plan_options)) {
				assert(FALSE);
				ERROR(ERR_PLAN_NOT_WELL_FORMED, "");
				return NULL;
			}
		}
		/* else: LP_TABLE_VALUE should not have aggregate function usages so no need to handle it. */
	} else {
		/* "lp_verify_structure()" would have been already called in a prior call to "generate_physical_plan" for a
		 * different physical plan that points to the same logical plan. This logical plan already has had any GROUP BY
		 * stuff filled in. In that case, should not do "lp_verify_structure" again as GROUP BY related information has
		 * already been filled in and a second invocation would create a linked list with a cycle (e.g. see YDBOcto#456).
		 *
		 * But check if this is a cross-reference plan. If so we do not want to create a duplicate (one enough per query).
		 */
		if (pp_from_lp->is_cross_reference_key) {
			return pp_from_lp;
		}
	}
	out = allocate_physical_plan(plan, pp_from_lp, &plan_options, options);
	/* Note: The below logic is executed multiple times in case multiple physical plans point to the same logical plan.
	 * One might be tempted to think there is no need to execute it. But it is necessary because the below logic could create
	 * other physical plans (through calls to "generate_physical_plan" and/or "sub_query_check_and_generate_physical_plan".
	 * Skipping the logic would mean fewer number of physical plans than needed would be created which is incorrect.
	 */
	root_table_alias = plan->extra_detail.lp_select_query.root_table_alias;
	/* Note: root_table_alias can be NULL for xref plans (which do not correspond to any actual user-specified query) */
	out->aggregate_function_or_group_by_specified
	    = ((NULL == root_table_alias) ? FALSE : root_table_alias->aggregate_function_or_group_by_specified);
	// Set my output key
	GET_LP(output, plan, 1, LP_OUTPUT);
	if (LP_KEY == output->v.lp_default.operand[0]->type) {
		GET_LP(output_key, output, 0, LP_KEY);
		out->outputKey = output_key->v.lp_key.key;
		out->is_cross_reference_key = out->outputKey->is_cross_reference_key;
	} else {
		assert(FALSE);
	}

	// If there is an ORDER BY, note it down
	if (output->v.lp_default.operand[1]) {
		assert(!is_lp_table_value);
		GET_LP(out->order_by, output, 1, LP_ORDER_BY);
	}

	out->projection = lp_get_projection_columns(plan);
	/* Some things are applicable only for a LP_SELECT_QUERY (not for a LP_TABLE_VALUE) hence the if check below */
	if (!is_lp_table_value) {
		// See if there are any tables we rely on in the SELECT tablejoin list. If so, add them as prev records in physical
		// plan.
		select = lp_get_select(plan);
		GET_LP(table_joins, select, 0, LP_TABLE_JOIN);
		out->tablejoin = table_joins;
		do {
			assert(LP_TABLE_JOIN == table_joins->type);
			// If this is a plan that doesn't have a source table,
			//  this will be null and we need to skip this step
			if (NULL == table_joins->v.lp_default.operand[0]) {
				break;
			}
			type = table_joins->v.lp_default.operand[0]->type;
			if ((LP_SELECT_QUERY == type) || (LP_SET_OPERATION == type) || (LP_TABLE_VALUE == type)) {
				PhysicalPlan *	    ret;
				PhysicalPlanOptions tmp_options;

				/* This is a fresh sub-query start so do not inherit any DNF context from parent query. */
				tmp_options = plan_options;
				tmp_options.dnf_plan_next = NULL;
				/* By the same token, do not inherit any "stash_columns_in_keys" context from parent query */
				tmp_options.stash_columns_in_keys = FALSE;
				// This is a sub plan, and should be inserted as prev
				GET_LP(insert_or_set_operation, table_joins, 0, type);
				ret = generate_physical_plan(insert_or_set_operation, &tmp_options);
				if (NULL == ret) {
					return NULL;
				}
			}
			table_joins = table_joins->v.lp_default.operand[1];
		} while (NULL != table_joins);
		// Check GROUP BY and HAVING
		GET_LP(criteria, select, 1, LP_CRITERIA);
		GET_LP(select_options, criteria, 1, LP_SELECT_OPTIONS);
		GET_LP(select_more_options, select_options, 1, LP_SELECT_MORE_OPTIONS);
		if (NULL != select_more_options->v.lp_default.operand[0]) {
			GET_LP(out->aggregate_options, select_more_options, 0, LP_AGGREGATE_OPTIONS);
		}
		// Iterate through the key substructures and fill out the source keys
		keys = lp_get_keys(plan);
		// Either we have some keys already, or we have a list of keys
		assert((0 < out->total_iter_keys) || (NULL != keys->v.lp_default.operand[0]));
		if (keys->v.lp_default.operand[0]) {
			iterate_keys(out, keys);
		}
		// Note: The below do/while loop can be done only after we have initialized "iterKeys[]" for this table_join.
		//       That is why this is not done as part of the previous do/while loop as "iterate_keys()" gets called only
		//       after the previous do/while loop.
		table_joins = out->tablejoin;
		do {
			LogicalPlan *join_on_condition;

			/* See if there are any sub-queries in the ON clause of any JOINs. If so, generate separate physical plans
			 * (deferred and/or non-deferred) for them and add them as prev records in physical plan.
			 */
			join_on_condition = table_joins->extra_detail.lp_table_join.join_on_condition;
			if (NULL != join_on_condition) {
				sub_query_check_and_generate_physical_plan(&plan_options, join_on_condition, NULL);
			}
			table_joins = table_joins->v.lp_default.operand[1];
		} while (NULL != table_joins);
		/* See if there are any sub-queries in the WHERE clause. If so, generate separate physical plans
		 * (deferred and/or non-deferred) for them and add them as prev records in physical plan.
		 */
		where = lp_get_select_where(plan);
		out->where = sub_query_check_and_generate_physical_plan(&plan_options, where, NULL);
		/* Note: If where->v.lp_default.operand[1] is non-NULL, this is the alternate list that
		 * "lp_optimize_where_multi_equals_ands_helper()" built that needs to be checked too for deferred plans which
		 * would have been missed out in case the keys for those had been fixed to keys from parent queries (see
		 * comment above "lp_get_select_where()" function call in "lp_optimize_where_multi_equals_ands_helper()").
		 * One would be tempted to discard that alternate list now that its purpose is served above. But it is possible
		 * the same logical plan is pointed to by multiple physical plans in which case it is still needed. As we have
		 * no easy way of knowing that here, we continue to keep "where->v.lp_default.opreand[1]" set to a non-NULL value.
		 */
		/* See if there are any sub-queries in the SELECT column list. If so, generate separate physical plans
		 * (deferred and/or non-deferred) for them and add them as prev records in physical plan.
		 */
		project = lp_get_project(plan);
		GET_LP(select_column_list, project, 0, LP_COLUMN_LIST);
		sub_query_check_and_generate_physical_plan(&plan_options, select_column_list, NULL);
		out->keywords = lp_get_select_keywords(plan)->v.lp_keywords.keywords;
		/* See if there are any sub-queries in the HAVING clause. If so, generate separate physical plans
		 * (deferred and/or non-deferred) for them and add them as prev records in physical plan.
		 */
		if (NULL != out->aggregate_options) {
			LogicalPlan *having;

			having = out->aggregate_options->v.lp_default.operand[1]; /* cannot use GET_LP as having can be NULL */
			if (NULL != having) {
				out->aggregate_options->v.lp_default.operand[1]
				    = sub_query_check_and_generate_physical_plan(&plan_options, having, NULL);
			}
		}
		/* See if there are any sub-queries in the ORDER BY clause. If so, generate separate physical plans
		 * (deferred and/or non-deferred) for them and add them as prev records in physical plan.
		 */
		if (NULL != out->order_by) {
			sub_query_check_and_generate_physical_plan(&plan_options, out->order_by, NULL);
		}
		// Check the optional words for distinct
		keywords = lp_get_select_keywords(plan)->v.lp_keywords.keywords;
		keyword = get_keyword_from_keywords(keywords, OPTIONAL_DISTINCT);
		if (NULL != keyword) {
			out->distinct_values = TRUE;
			out->maintain_columnwise_index = TRUE;
		}
		keyword = get_keyword_from_keywords(keywords, OPTIONAL_BOOLEAN_EXPANSION);
		if (NULL != keyword) {
			out->emit_duplication_check = TRUE;
			out->maintain_columnwise_index = TRUE; /* needs to be set in all DNF siblings */
			if (NULL != plan_options.dnf_plan_next) {
				/* Caller indicates this plan is part of a set of DNF plans. Maintain linked list of DNF siblings.
				 */
				out->dnf_next = plan_options.dnf_plan_next;
				assert(NULL == plan_options.dnf_plan_next->dnf_prev);
				plan_options.dnf_plan_next->dnf_prev = out;
				assert(NULL == out->dnf_prev);
			}
		}
	} else {
		/* VALUES clause. Check for any sub-queries in the data specified across multiple rows/columns */
		LogicalPlan *lp_table_data, *lp_row_value;

		GET_LP(lp_table_data, plan, 0, LP_TABLE_DATA);
		GET_LP(lp_row_value, lp_table_data, 1, LP_ROW_VALUE);
		do {
			LogicalPlan *lp_column_list;

			assert(LP_ROW_VALUE == lp_row_value->type);
			GET_LP(lp_column_list, lp_row_value, 0, LP_COLUMN_LIST);
			sub_query_check_and_generate_physical_plan(&plan_options, lp_column_list, NULL);
			lp_row_value = lp_row_value->v.lp_default.operand[1];
		} while (NULL != lp_row_value);
	}
	out->stash_columns_in_keys = options->stash_columns_in_keys;
	return out;
}

void iterate_keys(PhysicalPlan *out, LogicalPlan *plan) {
	LogicalPlan *left, *right;

	assert(LP_KEYS == plan->type);
	GET_LP(left, plan, 0, LP_KEY);
	out->iterKeys[out->total_iter_keys] = left->v.lp_key.key;
	out->total_iter_keys++;
	if (NULL != plan->v.lp_default.operand[1]) {
		GET_LP(right, plan, 1, LP_KEYS);
		iterate_keys(out, right);
	}
}

LogicalPlan *sub_query_check_and_generate_physical_plan(PhysicalPlanOptions *options, LogicalPlan *stmt, LogicalPlan *parent) {
	int i;

	if (NULL == stmt) {
		return NULL;
	}
	assert(LP_UNARY_LAST != stmt->type);
	if ((LP_ADDITION <= stmt->type) && (LP_BOOLEAN_LAST > stmt->type)) {
		stmt->v.lp_default.operand[0]
		    = sub_query_check_and_generate_physical_plan(options, stmt->v.lp_default.operand[0], stmt);
		stmt->v.lp_default.operand[1]
		    = sub_query_check_and_generate_physical_plan(options, stmt->v.lp_default.operand[1], stmt);
	} else if ((LP_FORCE_NUM <= stmt->type) && (LP_UNARY_LAST > stmt->type)) {
		stmt->v.lp_default.operand[0]
		    = sub_query_check_and_generate_physical_plan(options, stmt->v.lp_default.operand[0], stmt);
	} else {
		PhysicalPlanOptions plan_options;
		PhysicalPlan *	    cur, *child_plan;
		SqlTableAlias *	    table_alias;
		SqlColumnAlias *    column_alias;
		PhysicalPlan *	    new_plan;
		LogicalPlan *	    plan, *oper1;
		int		    unique_id;
		boolean_t	    set_deferred_plan;

		switch (stmt->type) {
		case LP_KEY:
			/* No action */
			break;
		case LP_WHERE:
		case LP_HAVING:
			stmt->v.lp_default.operand[0]
			    = sub_query_check_and_generate_physical_plan(options, stmt->v.lp_default.operand[0], stmt);
			if (LP_HAVING == stmt->type) {
				break;
			}
			assert(LP_WHERE == stmt->type);
			oper1 = stmt->v.lp_default.operand[1];
			if ((NULL != oper1) && (LP_COLUMN_LIST_ALIAS != oper1->type)) {
				/* Note that it is possible we have `operand[1]` set to a non-NULL value for a LP_WHERE.
				 * In this case, this is the alternate list that "lp_optimize_where_multi_equals_ands_helper()"
				 * built that needs to also be checked for sub-queries.
				 */
				stmt->v.lp_default.operand[1]
				    = sub_query_check_and_generate_physical_plan(options, stmt->v.lp_default.operand[1], stmt);
			}
			break;
		case LP_DERIVED_COLUMN:
		case LP_COLUMN_ALIAS:
			/* Check if this value is a parent reference; if so, mark this physical plan as deferred.
			 * Note that this physical plan might already be marked as deferred from a previous parent query
			 * column reference. In that case, compare the two to see which physical plan is the closest ancestor
			 * to the current physical plan and set that to be the deferred plan for this physical plan.
			 */
			if (LP_COLUMN_ALIAS == stmt->type) {
				column_alias = stmt->v.lp_column_alias.column_alias;
				UNPACK_SQL_STATEMENT(table_alias, column_alias->table_alias_stmt, table_alias);
				unique_id = table_alias->unique_id;
			} else {
				LogicalPlan *output_key;

				assert(LP_DERIVED_COLUMN == stmt->type);
				GET_LP(output_key, stmt, 0, LP_KEY);
				unique_id = output_key->v.lp_key.key->unique_id;
			}
			cur = options->parent;
			child_plan = cur;
			set_deferred_plan = FALSE;
			while (NULL != cur) {
				unsigned int iter_key_index;

				for (iter_key_index = 0; iter_key_index < cur->total_iter_keys; iter_key_index++) {
					if (cur->iterKeys[iter_key_index]->unique_id == unique_id) {
						break;
					}
				}
				if (iter_key_index != cur->total_iter_keys) {
					break;
				}
				cur = cur->parent_plan;
				set_deferred_plan = TRUE;
			}
			if (NULL == cur) {
				assert(FALSE);
				ERROR(ERR_PLAN_OWNER, "");
			}
			if (set_deferred_plan) {
				do {
					PhysicalPlan *parent;

					set_deferred_plan = TRUE; /* for next iteration */
					parent = child_plan->deferred_parent_plan;
					if (NULL != parent) {
						/* We already have set `deferred_parent_plan`. Check if the new deferred parent
						 * is a parent of the already set value. If so do not touch what was set.
						 * Else update it to reflect the newly computed deferred parent.
						 */
						for (; parent != cur;) {
							parent = parent->parent_plan;
							if (NULL == parent) {
								/* The pre existing parent is a closer ancestor than the newly
								 * computed parent so do not modify what was already set.
								 */
								set_deferred_plan = FALSE;
								break;
							}
						}
					}
					if (set_deferred_plan) {
						child_plan->deferred_parent_plan = cur;
					}
					child_plan = child_plan->parent_plan;
				} while (child_plan != cur);
			}
			/* No action */
			break;
		case LP_VALUE:
			/* No action */
			break;
		case LP_SELECT_QUERY:
		case LP_SET_OPERATION:
		case LP_TABLE_VALUE:
			/* Generate a separate physical plan for this sub-query.
			 * This is a fresh sub-query start so do not inherit any DNF context from parent query.
			 */
			plan_options = *options;
			plan_options.dnf_plan_next = NULL;
			if ((NULL != parent) && ((LP_BOOLEAN_EXISTS == parent->type) || (LP_BOOLEAN_NOT_EXISTS == parent->type))) {
				/* If sub-query is due to an EXISTS or NOT EXISTS operation, then do not stash columns in keys.
				 * Keep it as is (it can have any # of columns).
				 */
				plan_options.stash_columns_in_keys = FALSE;
			} else {
				/* Otherwise, we are guaranteed only 1 column in sub-query so stash it in a key (due to the
				 * check done in "src/optimization_transforms/lp_generate_where.c" (search for BOOLEAN_EXISTS).
				 */
				if ((LP_SET_OPERATION == stmt->type) || (!stmt->extra_detail.lp_select_query.to_array)) {
					assert(1 == lp_get_num_cols_in_select_column_list(stmt));
					plan_options.stash_columns_in_keys = TRUE;
				}
			}
			new_plan = generate_physical_plan(stmt, &plan_options);
			if (NULL == new_plan) {
				return NULL;
			}
			break;
		case LP_ARRAY:
			assert(LP_SELECT_QUERY == stmt->v.lp_default.operand[0]->type);
			stmt->v.lp_default.operand[0]
			    = sub_query_check_and_generate_physical_plan(options, stmt->v.lp_default.operand[0], stmt);
			break;
		case LP_COALESCE_CALL:
		case LP_GREATEST:
		case LP_LEAST:
			assert(LP_COLUMN_LIST == stmt->v.lp_default.operand[0]->type);
			stmt->v.lp_default.operand[0]
			    = sub_query_check_and_generate_physical_plan(options, stmt->v.lp_default.operand[0], stmt);
			break;
		case LP_NULL_IF:
			stmt->v.lp_default.operand[0]
			    = sub_query_check_and_generate_physical_plan(options, stmt->v.lp_default.operand[0], stmt);
			stmt->v.lp_default.operand[1]
			    = sub_query_check_and_generate_physical_plan(options, stmt->v.lp_default.operand[1], stmt);
			break;
		case LP_FUNCTION_CALL:
			assert(LP_VALUE == stmt->v.lp_default.operand[0]->type);
			stmt->v.lp_default.operand[0]
			    = sub_query_check_and_generate_physical_plan(options, stmt->v.lp_default.operand[0], stmt);
			assert(LP_COLUMN_LIST == stmt->v.lp_default.operand[1]->type);
			stmt->v.lp_default.operand[1]
			    = sub_query_check_and_generate_physical_plan(options, stmt->v.lp_default.operand[1], stmt);
			break;
		case LP_AGGREGATE_FUNCTION_COUNT_ASTERISK:
		case LP_AGGREGATE_FUNCTION_COUNT:
		case LP_AGGREGATE_FUNCTION_AVG:
		case LP_AGGREGATE_FUNCTION_MIN:
		case LP_AGGREGATE_FUNCTION_MAX:
		case LP_AGGREGATE_FUNCTION_SUM:
		case LP_AGGREGATE_FUNCTION_COUNT_DISTINCT:
		case LP_AGGREGATE_FUNCTION_AVG_DISTINCT:
		case LP_AGGREGATE_FUNCTION_SUM_DISTINCT:
		case LP_CASE:
		case LP_CASE_STATEMENT:
		case LP_CASE_BRANCH:
		case LP_CASE_BRANCH_STATEMENT:
			for (i = 0; i < 2; i++) {
				stmt->v.lp_default.operand[i]
				    = sub_query_check_and_generate_physical_plan(options, stmt->v.lp_default.operand[i], stmt);
			}
			break;
		case LP_COLUMN_LIST:
			/* To avoid a large recursion stack in case of thousands of columns, walk the column list iteratively */
			plan = stmt;
			while (NULL != plan) {
				assert(LP_COLUMN_LIST != plan->v.lp_default.operand[0]->type);
				sub_query_check_and_generate_physical_plan(options, plan->v.lp_default.operand[0], plan);
				plan = plan->v.lp_default.operand[1];
				assert((NULL == plan) || (LP_COLUMN_LIST == plan->type));
			}
			break;
		case LP_ORDER_BY:
			/* In the ORDER BY COLUMN NUM case, the ORDER BY plan (LP_ORDER_BY -> LP_COLUMN_LIST -> LP_WHERE)
			 * points to the same plan as the SELECT column list (LP_PROJECT -> LP_COLUMN_LIST -> ... -> LP_WHERE)
			 * and so do not descend down the LP_COLUMN_LIST plan (operand[0]) to avoid duplicate descents as this
			 * will cause multiple `unique_id` to be created for the same sub-query in case sub-queries are present
			 * in the SELECT column list (one for the SELECT column list reference and one for the ORDER BY COLUMN NUM
			 * reference) which is incorrect. For operand[1], the same logic applies if it is non-NULL and is an
			 * ORDER BY COLUMN NUM reference too but that would be caught by the recursive call below.
			 */
			if (!stmt->extra_detail.lp_order_by.order_by_column_num) {
				stmt->v.lp_default.operand[0]
				    = sub_query_check_and_generate_physical_plan(options, stmt->v.lp_default.operand[0], stmt);
			}
			stmt->v.lp_default.operand[1]
			    = sub_query_check_and_generate_physical_plan(options, stmt->v.lp_default.operand[1], stmt);
			break;
		case LP_TABLE:
			// This should never happen; fall through to error case
		default:
			ERROR(ERR_UNKNOWN_KEYWORD_STATE, "");
			assert(FALSE);
			return NULL;
			break;
		}
	}
	return stmt;
}

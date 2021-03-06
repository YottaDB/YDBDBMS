/****************************************************************
 *								*
 * Copyright (c) 2021 YottaDB LLC and/or its subsidiaries.	*
 * All rights reserved.						*
 *								*
 *	This source code contains the intellectual property	*
 *	of its copyright holder(s), and is made available	*
 *	under a license.  If you do not know the terms of	*
 *	the license, please stop and do not read further.	*
 *								*
 ****************************************************************/

#include <assert.h>

#include "octo.h"
#include "octo_types.h"

/*
 * Called by aggregate_function_STATEMENT case in qualify_statement() to expand table.* in aggregate functions.
 * SqlColumnAlias for TABLE.ASTERISK node is already qualified with the correct table_alias_stmt. We just iterate through its
 * column_list to retrieve all the columns required by table.*. SqlColumnList given by specification_list is updated to have
 * SqlColumnAlias nodes for all columns of table.
 * Paremeter `type` is passed the aggregate function type information. Used to determine if expansion is required.
 */
void process_table_asterisk_cl(SqlStatement *specification_list, SqlAggregateType type) {
	SqlStatement *	table_alias_stmt;
	SqlTableAlias * table_alias;
	SqlColumnList * cl_new;
	SqlColumnAlias *column_alias;

	if ((COUNT_ASTERISK_AGGREGATE == type) || (COUNT_AGGREGATE == type))
		return;
	assert(NULL != specification_list);
	/* Replace TABLENAME.ASTERISK node in specification_list with column list corresponding to TABLENAME table. */
	GET_TABLE_ASTERISK_COLUMN_ALIAS_FROM_COLUMN_LIST(column_alias, specification_list);
	cl_new = NULL;
	table_alias_stmt = column_alias->table_alias_stmt;
	assert(NULL != table_alias_stmt);
	UNPACK_SQL_STATEMENT(table_alias, table_alias_stmt, table_alias);
	if (NULL != table_alias->column_list) {
		SqlColumnListAlias *inner_cur_cla, *inner_start_cla;
		SqlStatement *	    column_alias_stmt;
		SqlColumnAlias *    column_alias;
		SqlColumnList *	    cl;

		UNPACK_SQL_STATEMENT(inner_start_cla, table_alias->column_list, column_list_alias);
		inner_cur_cla = inner_start_cla;
		if (COUNT_AGGREGATE_DISTINCT != type) {
			/* For AVG, SUM, MIN & MAX aggregate functions having a single column is valid.
			 * Check here if it has only a single column. If it does then replace table.* with the column.
			 * If not, return table.* as is and let populate_data_type() issue an error.
			 */
			if (inner_cur_cla->next != inner_cur_cla)
				return;
		}
		do {
			OCTO_CMALLOC_STRUCT(cl, SqlColumnList);
			dqinit(cl);
			column_alias = get_column_alias_for_column_list_alias(inner_cur_cla, table_alias_stmt);
			PACK_SQL_STATEMENT(column_alias_stmt, column_alias, column_alias);
			cl->value = column_alias_stmt;
			if (NULL == cl_new)
				cl_new = cl;
			else
				dqappend(cl_new, cl);

			inner_cur_cla = inner_cur_cla->next;
		} while (inner_cur_cla != inner_start_cla);
		specification_list->v.column_list = cl_new;
	}
}

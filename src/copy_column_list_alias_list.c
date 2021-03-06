/****************************************************************
 *								*
 * Copyright (c) 2020-2021 YottaDB LLC and/or its subsidiaries.	*
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

/* A copy function to copy a given column_list_alias list. Shallow copy is done when `table_alias_stmt` argument is NULL.
 * `table_alias_stmt`, `keywords` and `count` are used by `process_table_asterisk_cla()` to process cla in select column list,
 * ORDER BY and GROUP BY. These require copy of keywords to account for `ASC` and `DESC` usage. When invoked with these arguments
 * `group_by_column_number` and `group_by_column_count` are updated appropriately for GROUP BY usage.
 * `cla` - the list to be copied
 * `table_alias_stmt` - If `table_alias_stmt` is not NULL `get_column_alias_for_column_list_alias()` is used to get `column_alias`.
 * `keywords` - is only considered when `table_alias_stmt` exists. This is currently based on usage and can be enhanced if required.
 */
SqlColumnListAlias *copy_column_list_alias_list(SqlColumnListAlias *cla, SqlStatement *table_alias_stmt, SqlStatement *keywords,
						SqlTableAlias *table_alias) {
	SqlColumnListAlias *cla_copy, *cla_new, *cla_cur;

	cla_copy = NULL;
	cla_cur = cla;
	do {
		OCTO_CMALLOC_STRUCT(cla_new, SqlColumnListAlias);
		if (NULL == table_alias_stmt) {
			assert(NULL == cla_cur->duplicate_of_column);
			*cla_new = *cla_cur;
		} else {
			SqlColumnAlias *column_alias;
			SqlColumnList * cur;

			cla_new->alias = cla_cur->alias;
			cla_new->type = cla_cur->type;
			cla_new->keywords = keywords;
			OCTO_CMALLOC_STRUCT(cur, SqlColumnList);
			dqinit(cur);
			column_alias = get_column_alias_for_column_list_alias(cla_cur, table_alias_stmt);
			assert(NULL != table_alias);
			if (AGGREGATE_DEPTH_GROUP_BY_CLAUSE == table_alias->aggregate_depth) {
				SqlTableAlias *cur_table_alias;
				UNPACK_SQL_STATEMENT(cur_table_alias, table_alias_stmt, table_alias);
				if (cur_table_alias->parent_table_alias == table_alias) {
					/* Columns not belonging to current query are excluded from its GROUPBY list in
					 * `qualify_query()` GROUPBY check. As a result we need to update `group_by_column_count`
					 * only in the case where the parent_table_alias is equal to current table_alias otherwise
					 * `group_by_column_count` related assert fails in `qualify_query()`.
					 */
					column_alias->group_by_column_number = ++table_alias->group_by_column_count;
				}
			}
			PACK_SQL_STATEMENT(cur->value, column_alias, column_alias);
			PACK_SQL_STATEMENT(cla_new->column_list, cur, column_list);
		}
		dqinit(cla_new);
		if (NULL == cla_copy) {
			cla_copy = cla_new;
		} else {
			dqappend(cla_copy, cla_new);
		}
		cla_cur = cla_cur->next;
	} while (cla_cur != cla);
	return cla_copy;
}

/****************************************************************
 *								*
 * Copyright (c) 2019-2020 YottaDB LLC and/or its subsidiaries.	*
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

#include "octo.h"
#include "octo_types.h"
#include "logical_plan.h"

LogicalPlan *lp_drill_to_insert(LogicalPlan *plan) {
	LogicalPlan *cur_plan = plan;

	assert((LP_SELECT_QUERY == cur_plan->type) || (LP_SET_OPERATION == cur_plan->type) || (LP_TABLE_VALUE == cur_plan->type));
	while (LP_SET_OPERATION == cur_plan->type) {
		// Fetch one of the output plans from this set
		assert(LP_SET_OPTION == cur_plan->v.lp_default.operand[0]->type);
		assert(LP_PLANS == cur_plan->v.lp_default.operand[1]->type);
		cur_plan = cur_plan->v.lp_default.operand[1];
		cur_plan = cur_plan->v.lp_default.operand[0];
	}
	assert((LP_SELECT_QUERY == cur_plan->type) || (LP_TABLE_VALUE == cur_plan->type));
	return cur_plan;
}

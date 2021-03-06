/****************************************************************
 *								*
 * Copyright (c) 2020 YottaDB LLC and/or its subsidiaries.	*
 * All rights reserved.						*
 *								*
 *	This source code contains the intellectual property	*
 *	of its copyright holder(s), and is made available	*
 *	under a license.  If you do not know the terms of	*
 *	the license, please stop and do not read further.	*
 *								*
 ****************************************************************/

#include <assert.h>
#include <stdlib.h>

#include "octo.h"
#include "octo_types.h"

// Function invoked by the rule named "data_type" in src/parser.y
SqlStatement *data_type(SqlDataType data_type, SqlStatement *size_or_precision, SqlStatement *scale) {
	SqlStatement *ret;

	SQL_STATEMENT(ret, data_type_struct_STATEMENT);
	ret->v.data_type_struct.data_type = data_type;
	if (NULL == size_or_precision) {
		/* Column SIZE or PRECISION not specified. Set field accordingly. */
		ret->v.data_type_struct.size_or_precision = SIZE_OR_PRECISION_UNSPECIFIED;
		ret->v.data_type_struct.size_or_precision_parameter_index = 0;
	} else {
		/* Even though "atoi()" does not do error checking like "strtol()", it is fine here as most error
		 * checking is already done by the "ddl_int_literal_value" rule in "src/parser.y" (asserted below).
		 */
		assert((value_STATEMENT == size_or_precision->type) && (INTEGER_LITERAL == size_or_precision->v.value->type));
		ret->v.data_type_struct.size_or_precision = atoi(size_or_precision->v.value->v.string_literal);
		ret->v.data_type_struct.size_or_precision_parameter_index = size_or_precision->v.value->parameter_index;
	}
	if (NULL == scale) {
		/* Column SCALE not specified. Set field accordingly. */
		ret->v.data_type_struct.scale = SCALE_UNSPECIFIED;
		ret->v.data_type_struct.scale_parameter_index = 0;
	} else {
		assert(NULL != size_or_precision);
		/* Even though "atoi()" does not do error checking like "strtol()", it is fine here as most error
		 * checking is already done by the "ddl_literal_value" rule in "src/parser.y" (asserted below).
		 */
		assert((value_STATEMENT == scale->type) && (INTEGER_LITERAL == scale->v.value->type));
		ret->v.data_type_struct.scale = atoi(scale->v.value->v.string_literal);
		if ((0 > ret->v.data_type_struct.scale)
		    || (ret->v.data_type_struct.size_or_precision < ret->v.data_type_struct.scale)) {
			ERROR(ERR_NUMERIC_SCALE, ret->v.data_type_struct.scale, ret->v.data_type_struct.size_or_precision);
			return NULL;
		}
		ret->v.data_type_struct.scale_parameter_index = scale->v.value->parameter_index;
	}
	return ret;
}

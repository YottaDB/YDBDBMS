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
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// Used to convert between network and host endian
#include <arpa/inet.h>

#include "rocto.h"
#include "message_formats.h"

static void test_valid_input(void **state) {
	// Test a single startup message
	uint32_t message_length = 0;
	message_length += sizeof(uint32_t); // count length member
	char *message = "SELECT * FROM names;\0";
	message_length += strlen(message) + 1; // count null

	// Populate base message
	BaseMessage *test_data = (BaseMessage *)malloc(message_length + sizeof(BaseMessage) - sizeof(uint32_t));
	test_data->type = PSQL_Query;
	test_data->length = htonl(message_length);
	strncpy(test_data->data, message, message_length - sizeof(uint32_t));

	// The actual test
	Query *query = read_query(test_data);

	// Standard checks
	assert_non_null(query);
	assert_string_equal(message, query->query);

	free(query);
	free(test_data);
}

static void test_non_terminated_input(void **state) {
	// Test a single startup message
	uint32_t message_length = 0;
	message_length += sizeof(uint32_t); // count length member
	char *message = "SELECT * FROM names;";
	message_length += strlen(message); // exclude null for test case

	// Populate base message
	BaseMessage *test_data = (BaseMessage *)malloc(message_length + sizeof(BaseMessage) - sizeof(uint32_t));
	test_data->type = PSQL_Query;
	test_data->length = htonl(message_length);
	strncpy(test_data->data, message, message_length - sizeof(uint32_t));

	// The actual test
	Query *query = read_query(test_data);

	// Standard checks
	assert_null(query);

	free(test_data);
}

static void test_unexpectedly_terminated_input(void **state) {
	// Test a single startup message
	uint32_t message_length = 0;
	message_length += sizeof(uint32_t); // count length member
	char *message = "SELECT * FROM names\0;";
	message_length += strlen(message) + 2; // expecting extra char after null

	// Populate base message
	BaseMessage *test_data = (BaseMessage *)malloc(message_length + sizeof(BaseMessage) - sizeof(uint32_t));
	test_data->type = PSQL_Query;
	test_data->length = htonl(message_length);
	strncpy(test_data->data, message, message_length - sizeof(uint32_t));

	// The actual test
	Query *query = read_query(test_data);

	// Standard checks
	assert_null(query);

	free(test_data);
}

static void test_missing_query(void **state) {
	// Test a single startup message
	uint32_t message_length = 0;
	message_length += sizeof(uint32_t); // count length member

	// Populate base message
	BaseMessage *test_data = (BaseMessage *)malloc(message_length + sizeof(BaseMessage) - sizeof(uint32_t));
	test_data->type = PSQL_Query;
	test_data->length = htonl(message_length);

	// The actual test
	Query *query = read_query(test_data);

	// Standard checks
	assert_null(query);

	free(test_data);
}

static void test_invalid_type(void **state) {
	// Test a single startup message
	uint32_t message_length = 0;
	message_length += sizeof(uint32_t); // count length member
	char *message = "SELECT * FROM names;";
	message_length += strlen(message) + 1; // expecting extra char after null

	// Populate base message
	BaseMessage *test_data = (BaseMessage *)malloc(message_length + sizeof(BaseMessage) - sizeof(uint32_t));
	test_data->type = 'X';
	test_data->length = htonl(message_length);
	strncpy(test_data->data, message, message_length - sizeof(uint32_t));

	// The actual test
	Query *query = read_query(test_data);

	// Standard checks
	assert_null(query);

	free(test_data);
}

int main(void) {
	octo_init(0, NULL);
	const struct CMUnitTest tests[] = {
	    cmocka_unit_test(test_valid_input),
	    cmocka_unit_test(test_non_terminated_input),
	    cmocka_unit_test(test_unexpectedly_terminated_input),
	    cmocka_unit_test(test_missing_query),
	    cmocka_unit_test(test_invalid_type),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

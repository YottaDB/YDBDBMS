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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <string.h>

#include <libyottadb.h>
#include <gtmxc_types.h>

#include "octo.h"
#include "octo_types.h"
#include "physical_plan.h"
#include "parser.h"
#include "lexer.h"
#include "git_hashes.h"

void handle_invalid_option(char *executable_name, char short_option) {
	printf("%s: invalid option -- '%c'\n", executable_name, short_option);
	printf("Please use '%s --help' for more information.\n", executable_name);
	exit(1);
}

int parse_startup_flags(int argc, char **argv, char **config_file_name) {
	enum VERBOSITY_LEVEL prev_error_level = ERROR; // ERROR is the default level
	char *		     octo_usage
	    = "Usage: octo [OPTION]...\n"
	      "Start the Octo SQL server.\n\n"
	      "Mandatory arguments for long options are also mandatory for short options.\n"
	      "  -c, --config-file=<filepath>		Use specified configuration file instead of the default.\n"
	      "  -d, --dry-run				Run the parser in read-only mode and performs basic checks without "
	      "executing any passed SQL statements.\n"
	      "  -f, --input-file=<filepath>		Read commands from specified file instead of opening interactive prompt.\n"
	      "  -h, --help				Display this help message and exit.\n"
	      "  -v, --verbose=<number>		Specify amount of information to output when running commands by adding 'v' "
	      "characters. The mapping of 'v's to severity levels is as follows:\n"
	      "	-v: include INFO and WARNING messages\n"
	      "	-vv: include DEBUG messages\n"
	      "	-vvv: include TRACE messages\n"
	      "  -r, --version				Display version information and exit.\n"
	      "  -r, --release				Display release information and exit.\n";
	char *rocto_usage
	    = "Usage: rocto [OPTION]...\n"
	      "Start the Rocto remote SQL server.\n\n"
	      "Mandatory arguments for long options are also mandatory for short options.\n"
	      "  -a, --allowschemachanges		Allows rocto to make changes to the schema (CREATE TABLE and DROP TABLE)\n"
	      "  -c, --config-file=<filepath>		Use specified configuration file instead of the default.\n"
	      "  -h, --help				Display this help message and exit.\n"
	      "  -p, --port=<number>			Listen on the specified port.\n"
	      "  -v, --verbose=<number>		Specify amount of information to output when running commands by adding 'v' "
	      "characters. The mapping of 'v's to severity levels is as follows:\n"
	      "	-v: include INFO and WARNING messages\n"
	      "	-vv: include DEBUG messages\n"
	      "	-vvv: include TRACE messages\n"
	      "  -r, --release				Display release information and exit.\n";
	int	  c;
	boolean_t verbosity_unset = TRUE, port_unset = TRUE;

	if ((0 < argc) && (NULL != strstr(argv[0], "rocto"))) {
		config->is_rocto = TRUE;
		// Rocto is by default not allowed to make schema changes
		config->allow_schema_changes = FALSE;
	} else {
		config->is_rocto = FALSE;
		config->allow_schema_changes = TRUE;
	}
	config->process_id = getpid();
	optind = 1;

	/* Parse input parameters */
	while (1) {
		// List of valid Octo long options
		static struct option octo_long_options[] = {{"verbose", optional_argument, NULL, 'v'},
							    {"dry-run", no_argument, NULL, 'd'},
							    {"input-file", required_argument, NULL, 'f'},
							    {"config-file", required_argument, NULL, 'c'},
							    {"help", no_argument, NULL, 'h'},
							    {"version", no_argument, NULL, 'r'},
							    {"release", no_argument, NULL, 'r'},
							    {0, 0, 0, 0}};

		// List of valid Rocto long options
		static struct option rocto_long_options[] = {{"verbose", optional_argument, NULL, 'v'},
							     {"config-file", required_argument, NULL, 'c'},
							     {"port", required_argument, NULL, 'p'},
							     {"help", no_argument, NULL, 'h'},
							     {"allowschemachanges", no_argument, NULL, 'a'},
							     {"version", no_argument, NULL, 'r'},
							     {"release", no_argument, NULL, 'r'},
							     {0, 0, 0, 0}};
		int		     option_index = 0;

		if (config->is_rocto) {
			c = getopt_long(argc, argv, "vhc:p:ar", rocto_long_options, &option_index);
		} else {
			c = getopt_long(argc, argv, "vdhf:c:r", octo_long_options, &option_index);
		}
		if (-1 == c)
			break;

		switch (c) {
		case 0:
			break;
		case 'v':
			if (optarg) {
				c = atoi(optarg);
				if (ERROR < c || TRACE > c) {
					ERROR(ERR_INVALID_CLI_OPTION, "--verbose");
					return 1;
				}
				config->verbosity_level = ERROR - c;
			} else {
				config->verbosity_level = ((TRACE < prev_error_level) ? --prev_error_level : prev_error_level);
			}
			verbosity_unset = FALSE;
			break;
		case 'c':
			*config_file_name = optarg;
			break;
		case 'h':
			if (config->is_rocto) {
				printf("%s", rocto_usage);
			} else {
				printf("%s", octo_usage);
			}
			exit(0);
			break;
		case 'f':
			inputFile = fopen(optarg, "r");
			if (NULL == inputFile) {
				ERROR(ERR_FILE_NOT_FOUND, optarg);
				return 1;
			}
			break;
		case 'd':
			config->dry_run = TRUE;
			break;
		case 'p':
			config->rocto_config.port = atoi(optarg);
			if (0 > config->rocto_config.port || 65535 < config->rocto_config.port) {
				printf("Please use a port number between 0 and 65535\n");
				exit(1);
			}
			port_unset = FALSE;
			break;
		case 'a':
			config->allow_schema_changes = TRUE;
			break;
		case 'r':
			printf("YottaDB version: r%s\n", YDB_RELEASE_STRING);
			if (config->is_rocto) {
				printf("Rocto");

			} else {
				printf("Octo");
			}
			printf(" version %d.%d.%d\n", YDBOCTO_MAJOR_VERSION, YDBOCTO_MINOR_VERSION, YDBOCTO_PATCH_VERSION);
			printf("Git commit: %s\n", YDBOCTO_GIT_COMMIT_VERSION);
			printf("Uncommitted changes: %s\n", YDBOCTO_GIT_IS_DIRTY);
			exit(0);
			break;
		default:
			printf("Please use '%s --help' for more information.\n", argv[0]);
			exit(1);
			return 1;
			break;
		}
	}
	/* Mark config settings that may overwrite configuration files as uninitialized
	 * to avoid erroneously overwriting configuration file settings when no command
	 * line value is specified. Note that 0 is a valid verbosity level and
	 * technically a valid port number.
	 */
	if (verbosity_unset) {
		config->verbosity_level = ERROR + 1; // Cannot use negative numbers for enum, so use an invalid value
	}
	if (port_unset) {
		config->rocto_config.port = -1;
	}
	return 0;
}

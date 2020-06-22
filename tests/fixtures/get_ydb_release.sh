#!/bin/bash
#################################################################
#								#
# Copyright (c) 2020 YottaDB LLC and/or its subsidiaries.	#
# All rights reserved.						#
#								#
#	This source code contains the intellectual property	#
#	of its copyright holder(s), and is made available	#
#	under a license.  If you do not know the terms of	#
#	the license, please stop and do not read further.	#
#								#
#################################################################

actual_release=$($ydb_dist/yottadb -run %XCMD 'write +$PIECE($ZYRELEASE," r",2)')
release_accepted=$(awk -v expected=$1 -v actual=$actual_release 'BEGIN { printf (expected <= actual ? "1" : "0") }')
echo -n $actual_release
if [[ "1" == $release_accepted ]]; then
	exit 0
else
	exit 1
fi

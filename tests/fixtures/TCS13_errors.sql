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

-- TCS13 : OCTO546 : CASE does not return NULL if ELSE is omitted and no comparison operation evaluates to TRUE

-- Octo should give a type error if selecting an integer but comparing to string
SELECT CASE WHEN id = '' THEN 'true' ELSE 'false' END
	FROM (SELECT CASE WHEN false THEN 2 END AS id) n1;

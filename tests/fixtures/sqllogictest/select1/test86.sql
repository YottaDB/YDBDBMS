#################################################################
#								#
# Copyright (c) 2019 YottaDB LLC and/or its subsidiaries.	#
# All rights reserved.						#
#								#
#	This source code contains the intellectual property	#
#	of its copyright holder(s), and is made available	#
#	under a license.  If you do not know the terms of	#
#	the license, please stop and do not read further.	#
#								#
#################################################################
# Derived from https://github.com/shivarajugowda/jdbcSQLTest

SELECT a+b*2,
       a+b*2+c*3+d*4+e*5,
       a-b,
       abs(b-c),
       c,
       b,
       e
  FROM t1
 WHERE d>e
 ORDER BY 4,5,3,6,2,1,7
;

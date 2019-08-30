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

SELECT (SELECT count(*) FROM t1 AS x WHERE x.b<t1.b),
       a,
       (a+b+c+d+e)/5,
       c,
       a+b*2+c*3+d*4
  FROM t1
 WHERE (e>c OR e<d)
 /*ORDER BY 3,5,1,4,2*/
;
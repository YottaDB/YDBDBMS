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

SELECT a+b*2+c*3,
       c,
       abs(a),
       a+b*2+c*3+d*4+e*5,
       CASE WHEN c>(SELECT avg(c) FROM t1) THEN a*2 ELSE b*10 END
  FROM t1
 WHERE (e>a AND e<b)
    OR d>e
    OR e+d BETWEEN a+b-10 AND c+130
 ORDER BY 3,4,2,5,1
;
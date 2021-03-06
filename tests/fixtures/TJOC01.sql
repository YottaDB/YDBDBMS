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

-- TJOC01 : OCTO291 JOIN queries with invalid ON conditions

select o1.order_id from orders o1 left join orders o2 on o1.customer_id != o3.customer_id left join orders o3 on o2.customer_id = o3.customer_id;


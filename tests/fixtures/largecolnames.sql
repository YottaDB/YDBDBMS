#################################################################
#								#
# Copyright (c) 2019-2021 YottaDB LLC and/or its subsidiaries.	#
# All rights reserved.						#
#								#
#	This source code contains the intellectual property	#
#	of its copyright holder(s), and is made available	#
#	under a license.  If you do not know the terms of	#
#	the license, please stop and do not read further.	#
#								#
#################################################################

CREATE TABLE largecolnames(a1234567890 int, b1234567890 int, c1234567890 int)
GLOBAL "^LARGECOLNAMES(keys(""a1234567890""),keys(""b1234567890""),keys(""c1234567890""))";

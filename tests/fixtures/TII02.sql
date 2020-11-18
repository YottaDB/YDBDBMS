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

-- TIIT02 : INSERT INTO a xfer table

CREATE TABLE namesLastNameXref (
  lastName VARCHAR(30) PRIMARY KEY,
  id INTEGER KEY NUM 1
);
INSERT INTO namesLastNameXref (SELECT lastName, id FROM NAMES WHERE lastName <> '');
SELECT * FROM namesLastNameXref;
DROP TABLE namesLastNameXref;

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

-- TMF02 : OCTO574 : pastas schema : Using ROUND() on AVG() aggregate function in HAVING clause returns incorrect results

-- Below are various queries randomly generated by QueryGenerator.m that failed in the pipeline over time
-- all of which are related to OCTO574 so these serve as the automated test case for the fix.

SELECT DISTINCT pastas.id, AVG(ALL alias1.id), alias1.pastaName FROM pastas NATURAL JOIN (SELECT alias1.id, alias1.pastaName FROM pastas alias1) AS alias1 WHERE ((pastas.id < pastas.id) AND (NOT (pastas.pastaName <= 'Orechiette"')) OR (NOT (pastas.pastaName > pastas.pastaName))) GROUP BY alias1.pastaName, pastas.id HAVING ROUND(AVG(DISTINCT alias1.id), 11) NOT IN (5);

SELECT ALL alias1.id, AVG(alias1.id), alias1.pastaName FROM pastas  INNER JOIN (SELECT ALL MIN(ALL alias1.pastaName) as pastaName, alias1.id FROM pastas alias1 GROUP BY alias1.id) AS alias1 ON (((pastas.id = alias1.id))) WHERE EXISTS (SELECT alias3.id, alias3.pastaName FROM pastas alias3) GROUP BY alias1.id, alias1.pastaName HAVING ROUND(AVG(DISTINCT alias1.id), 11) IN (4);

SELECT pastas.id, MIN(alias1.id), alias1.pastaName FROM pastas  RIGHT JOIN (SELECT AVG(alias1.id) as id, alias1.pastaName FROM pastas alias1 GROUP BY alias1.pastaName HAVING ROUND(AVG(DISTINCT alias1.id), 11) != SOME (SELECT ALL alias2.id FROM names4 alias2 ORDER BY alias2.id LIMIT 1)) AS alias1 ON (((pastas.pastaName != alias1.pastaName))) GROUP BY alias1.pastaName, pastas.id;

SELECT pastas.pastaName, AVG(alias1.id), alias1.firstName FROM pastas  RIGHT JOIN (SELECT DISTINCT alias1.id, alias1.firstName FROM names4 alias1) AS alias1 ON (((pastas.id <= alias1.id) AND NOT (pastas.id <= ALL (SELECT ALL alias2.id FROM names4 alias2 ORDER BY alias2.id LIMIT 1)))) WHERE (((1) + -(-7)) < pastas.id) GROUP BY alias1.firstName, pastas.pastaName HAVING ROUND(AVG(alias1.id), 11) IN (9);

SELECT (SELECT ALL alias3.id FROM pastas alias3 WHERE NOT EXISTS (SELECT AVG(DISTINCT alias4.id) as id, alias4.pastaName FROM pastas alias4 WHERE alias4.id > SOME (SELECT DISTINCT alias5.id FROM pastas alias5 WHERE (NOT (alias5.pastaName > alias5.pastaName) AND NOT ('Fettucine"' = 'Linguini"')) ORDER BY alias5.id LIMIT 1) GROUP BY alias4.pastaName, alias3.id HAVING ROUND(AVG(alias4.id), 11) IN (1)) ORDER BY alias3.id LIMIT 1) AS alias3, MAX(DISTINCT pastas.id), pastas.pastaName FROM pastas  CROSS JOIN (SELECT ALL alias1.favoritePasta, alias1.id FROM names4 alias1 WHERE (((4) % -(-8)) = alias1.id)) AS alias1 WHERE (('Penne"'||5)<='Shells"') GROUP BY pastas.pastaName;

SELECT DISTINCT (SELECT alias9.pastaName FROM pastas alias9 WHERE alias9.pastaName BETWEEN 'Rotini"' AND 'Linguini"' ORDER BY alias9.pastaName LIMIT 1) AS alias9, AVG(alias6.id) FROM pastas LEFT JOIN (SELECT DISTINCT alias1.pastaName, alias1.id FROM pastas alias1 WHERE alias1.id < SOME (SELECT alias2.id FROM pastas alias2 WHERE EXISTS (SELECT DISTINCT AVG(ALL alias3.id) as id, alias3.pastaName FROM pastas alias3 WHERE alias3.pastaName = ALL (SELECT alias4.favoritePasta FROM names4 alias4 WHERE ((alias4.lastName||'Lasagna')<='BurnCavatelli') ORDER BY alias4.favoritePasta LIMIT 1) GROUP BY alias3.pastaName, alias2.id, alias1.id, alias1.pastaName HAVING alias3.pastaName BETWEEN 'Orechiette"' AND 'Orechiette"' ORDER BY AVG(ALL alias3.id), alias3.pastaName) ORDER BY alias2.id LIMIT 1) ORDER BY alias1.id, alias1.pastaName) AS alias1 ON ((pastas.pastaName = alias1.pastaName) AND NOT ((pastas.pastaName >= alias1.pastaName) AND NOT (pastas.pastaName = alias1.pastaName) AND NOT (pastas.pastaName = alias1.pastaName)) AND NOT (pastas.pastaName <= alias1.pastaName)) LEFT JOIN (SELECT alias6.id FROM names4 alias6 WHERE (((5) % +(+4)) < alias6.id) ORDER BY alias6.id) AS alias6 ON ((alias1.id < SOME (SELECT alias7.id FROM pastas alias7 WHERE alias7.pastaName IN ('Spaghetti"', 'Orechiette"', 'Penne"', 'Shells"', 'Lasagna"', 'Fettucine"') ORDER BY alias7.id LIMIT 1))) WHERE pastas.pastaName LIKE 'pG%_#_6'  HAVING ROUND(AVG(alias6.id), 11) BETWEEN 3 AND 9 ORDER BY AVG(alias6.id), alias9;

SELECT (SELECT DISTINCT alias3.id FROM pastas alias3 WHERE alias3.id IN (5, 7) ORDER BY alias3.id LIMIT 1) AS alias3, AVG(DISTINCT alias1.id) FROM pastas  LEFT OUTER JOIN (SELECT alias1.id FROM pastas alias1 WHERE ((alias1.pastaName||2)<'Penne"')) AS alias1 ON ((pastas.id >= alias1.id)) WHERE (pastas.id < -(+0))  HAVING ROUND(AVG(DISTINCT alias1.id), 11) IN (7);

SELECT pastas.pastaName, AVG(alias1.id), alias1.pastaName FROM pastas  CROSS JOIN pastas AS alias1 WHERE (('Spaghetti"'||'Orechiette"')>'Orechiette"Fettucine"') GROUP BY alias1.pastaName, pastas.pastaName HAVING ROUND(AVG(alias1.id), 11) IN (3) ORDER BY AVG(alias1.id), alias1.pastaName, pastas.pastaName LIMIT 24;

SELECT pastas.pastaName, AVG(alias1.id), alias1.pastaName FROM pastas  CROSS JOIN pastas AS alias1 WHERE (('Spaghetti"'||'Orechiette"')>'Orechiette"Fettucine"') GROUP BY alias1.pastaName, pastas.pastaName HAVING ROUND(AVG(alias1.id), 11) IN (3);

SELECT COUNT(*) FROM pastas  CROSS JOIN pastas AS alias1 WHERE (('Spaghetti"'||'Orechiette"')>'Orechiette"Fettucine"');

SELECT alias1.pastaName, pastas.pastaName, AVG(alias1.id) FROM pastas  CROSS JOIN pastas AS alias1 WHERE (('Spaghetti"'||'Orechiette"')>'Orechiette"Fettucine"') GROUP BY alias1.pastaName, pastas.pastaName;

SELECT ROUND(AVG(alias1.id), 11) FROM pastas  CROSS JOIN pastas AS alias1 WHERE (('Spaghetti"'||'Orechiette"')>'Orechiette"Fettucine"') GROUP BY alias1.pastaName, pastas.pastaName;

SELECT ROUND(AVG(alias1.id), 11) FROM pastas as alias1 WHERE (('Spaghetti"'||'Orechiette"')>'Orechiette"Fettucine"') GROUP BY alias1.pastaName;

SELECT ROUND(AVG(alias1.id), 11) FROM pastas as alias1 WHERE (('Spaghetti"'||'Orechiette"')>'Orechiette"Fettucine"') GROUP BY alias1.pastaName  HAVING ROUND(AVG(alias1.id), 11) IN (3);

SELECT alias1.pastaName, AVG(DISTINCT alias1.id), alias1.pastaName FROM names4  RIGHT JOIN pastas AS alias1 ON ((names4.favoritePasta < 'Lasagna"') OR (('Pappardelle' >= 'Lasagna"'))) WHERE (NOT (names4.favoritePasta >= names4.favoritePasta) OR ((names4.firstName <= names4.firstName)) AND (('Zero' >= 'Zero'))) GROUP BY alias1.pastaName HAVING ROUND(AVG(ALL alias1.id), 11) IN (1) LIMIT 63;


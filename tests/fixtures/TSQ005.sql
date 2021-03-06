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

SELECT NULL                                         AS type_cat,
       n.nspname                                    AS type_schem,
       t.typname                                    AS type_name,
       NULL                                         AS class_name,
       CASE
         WHEN t.typtype = 'c' THEN 2002
         ELSE 2001
       END                                          AS data_type,
       pg_catalog.Obj_description(t.oid, 'pg_type') AS remarks,
       CASE
         WHEN t.typtype = 'd' THEN (SELECT CASE
                                             WHEN typname = 'date' THEN 91
                                             WHEN typname = 'time[]' THEN 2003
                                             WHEN typname = '_name' THEN 2003
                                             WHEN typname = 'numeric[]' THEN
                                             2003
                                             WHEN typname = 'timestamptz[]' THEN
                                             2003
                                             WHEN typname = 'refcursor' THEN
                                             2012
                                             WHEN typname = 'timetz[]' THEN 2003
                                             WHEN typname = 'money[]' THEN 2003
                                             WHEN typname = 'bit' THEN -7
                                             WHEN typname = 'uuid' THEN 1111
                                             WHEN typname = '_money' THEN 2003
                                             WHEN typname = 'int2' THEN 5
                                             WHEN typname = 'name[]' THEN 2003
                                             WHEN typname = 'int4' THEN 4
                                             WHEN typname = '_varchar' THEN 2003
                                             WHEN typname = 'uuid[]' THEN 2003
                                             WHEN typname = 'xml' THEN 2009
                                             WHEN typname = 'int8' THEN -5
                                             WHEN typname = 'text' THEN 12
                                             WHEN typname = '_numeric' THEN 2003
                                             WHEN typname = '_timestamp' THEN
                                             2003
                                             WHEN typname = 'json[]' THEN 2003
                                             WHEN typname = '_bool' THEN 2003
                                             WHEN typname = 'date[]' THEN 2003
                                             WHEN typname = 'varchar[]' THEN
                                             2003
                                             WHEN typname = '_oid' THEN 2003
                                             WHEN typname = '_json' THEN 2003
                                             WHEN typname = 'int2[]' THEN 2003
                                             WHEN typname = 'int4[]' THEN 2003
                                             WHEN typname = 'bpchar' THEN 1
                                             WHEN typname = 'name' THEN 12
                                             WHEN typname = 'bit[]' THEN 2003
                                             WHEN typname = 'int8[]' THEN 2003
                                             WHEN typname = 'timestamptz' THEN
                                             93
                                             WHEN typname = 'refcursor[]' THEN
                                             2003
                                             WHEN typname = '_bpchar' THEN 2003
                                             WHEN typname = 'bpchar[]' THEN 2003
                                             WHEN typname = 'bytea' THEN -2
                                             WHEN typname = 'bool' THEN -7
                                             WHEN typname = 'bool[]' THEN 2003
                                             WHEN typname = '_refcursor' THEN
                                             2003
                                             WHEN typname = 'numeric' THEN 2
                                             WHEN typname = 'oid' THEN -5
                                             WHEN typname = 'point' THEN 1111
                                             WHEN typname = '_timestamptz' THEN
                                             2003
                                             WHEN typname = '_float4' THEN 2003
                                             WHEN typname = 'bytea[]' THEN 2003
                                             WHEN typname = 'json' THEN 1111
                                             WHEN typname = 'timestamp[]' THEN
                                             2003
                                             WHEN typname = '_char' THEN 2003
                                             WHEN typname = '_time' THEN 2003
                                             WHEN typname = '_float8' THEN 2003
                                             WHEN typname = 'timestamp' THEN 93
                                             WHEN typname = '_bytea' THEN 2003
                                             WHEN typname = 'oid[]' THEN 2003
                                             WHEN typname = '_bit' THEN 2003
                                             WHEN typname = '_int8' THEN 2003
                                             WHEN typname = '_date' THEN 2003
                                             WHEN typname = 'varchar' THEN 12
                                             WHEN typname = '_int4' THEN 2003
                                             WHEN typname = 'float8' THEN 8
                                             WHEN typname = '_point' THEN 2003
                                             WHEN typname = '_uuid' THEN 2003
                                             WHEN typname = 'float4' THEN 7
                                             WHEN typname = 'point[]' THEN 2003
                                             WHEN typname = '_int2' THEN 2003
                                             WHEN typname = '_timetz' THEN 2003
                                             WHEN typname = 'char[]' THEN 2003
                                             WHEN typname = 'float8[]' THEN 2003
                                             WHEN typname = 'money' THEN 8
                                             WHEN typname = '_xml' THEN 2003
                                             WHEN typname = 'float4[]' THEN 2003
                                             WHEN typname = 'text[]' THEN 2003
                                             WHEN typname = 'xml[]' THEN 2003
                                             WHEN typname = 'char' THEN 1
                                             WHEN typname = 'time' THEN 92
                                             WHEN typname = '_text' THEN 2003
                                             WHEN typname = 'timetz' THEN 92
                                             ELSE 1111
                                           END
                                    FROM   pg_type
                                    WHERE  oid = t.typbasetype)
         ELSE NULL
       END                                          AS base_type
FROM   pg_catalog.pg_type t,
       pg_catalog.pg_namespace n
WHERE  t.typnamespace = n.oid
       AND n.nspname != 'pg_catalog'
       AND n.nspname != 'pg_toast'
       AND t.typtype IN ( 'c', 'd' )
ORDER  BY data_type,
          type_schem,
          type_name


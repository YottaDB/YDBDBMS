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

select null as type_cat, n.nspname as type_schem, t.typname as type_name,  null as class_name, CASE WHEN t.typtype='c' then 2002 else 2001 end as data_type, pg_catalog.obj_description(t.oid, 'pg_type')  as remarks, CASE WHEN t.typtype = 'd' then  (select CASE when typname = 'date' then 91 when typname = 'time[]' then 2003 when typname = '_name' then 2003 when typname = 'numeric[]' then 2003 when typname = 'timestamptz[]' then 2003 when typname = 'refcursor' then 2012 when typname = 'timetz[]' then 2003 when typname = 'money[]' then 2003 when typname = 'bit' then -7 when typname = 'uuid' then 1111 when typname = '_money' then 2003 when typname = 'int2' then 5 when typname = 'name[]' then 2003 when typname = 'int4' then 4 when typname = '_varchar' then 2003 when typname = 'uuid[]' then 2003 when typname = 'xml' then 2009 when typname = 'int8' then -5 when typname = 'text' then 12 when typname = '_numeric' then 2003 when typname = '_timestamp' then 2003 when typname = 'json[]' then 2003 when typname = '_bool' then 2003 when typname = 'date[]' then 2003 when typname = 'varchar[]' then 2003 when typname = '_oid' then 2003 when typname = '_json' then 2003 when typname = 'int2[]' then 2003 when typname = 'int4[]' then 2003 when typname = 'bpchar' then 1 when typname = 'name' then 12 when typname = 'bit[]' then 2003 when typname = 'int8[]' then 2003 when typname = 'timestamptz' then 93 when typname = 'refcursor[]' then 2003 when typname = '_bpchar' then 2003 when typname = 'bpchar[]' then 2003 when typname = 'bytea' then -2 when typname = 'bool' then -7 when typname = 'bool[]' then 2003 when typname = '_refcursor' then 2003 when typname = 'numeric' then 2 when typname = 'oid' then -5 when typname = 'point' then 1111 when typname = '_timestamptz' then 2003 when typname = '_float4' then 2003 when typname = 'bytea[]' then 2003 when typname = 'json' then 1111 when typname = 'timestamp[]' then 2003 when typname = '_char' then 2003 when typname = '_time' then 2003 when typname = '_float8' then 2003 when typname = 'timestamp' then 93 when typname = '_bytea' then 2003 when typname = 'oid[]' then 2003 when typname = '_bit' then 2003 when typname = '_int8' then 2003 when typname = '_date' then 2003 when typname = 'varchar' then 12 when typname = '_int4' then 2003 when typname = 'float8' then 8 when typname = '_point' then 2003 when typname = '_uuid' then 2003 when typname = 'float4' then 7 when typname = 'point[]' then 2003 when typname = '_int2' then 2003 when typname = '_timetz' then 2003 when typname = 'char[]' then 2003 when typname = 'float8[]' then 2003 when typname = 'money' then 8 when typname = '_xml' then 2003 when typname = 'float4[]' then 2003 when typname = 'text[]' then 2003 when typname = 'xml[]' then 2003 when typname = 'char' then 1 when typname = 'time' then 92 when typname = '_text' then 2003 when typname = 'timetz' then 92 else 1111 end from pg_type where oid=t.typbasetype) else null end as base_type from pg_catalog.pg_type t, pg_catalog.pg_namespace n where t.typnamespace = n.oid and n.nspname != 'pg_catalog' and n.nspname != 'pg_toast' and t.typtype IN ('c','d')  order by data_type, type_schem, type_name

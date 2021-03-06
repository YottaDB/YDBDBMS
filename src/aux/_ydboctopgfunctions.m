;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;								;
; Copyright (c) 2019-2021 YottaDB LLC and/or its subsidiaries.	;
; All rights reserved.						;
;								;
;	This source code contains the intellectual property	;
;	of its copyright holder(s), and is made available	;
;	under a license.  If you do not know the terms of	;
;	the license, please stop and do not read further.	;
;								;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

pgBackendPid()
	QUIT ""

pgCurrentCatalog() ; TODO this is just a placeholder
	QUIT "octo"

pgCurrentDatabase() ; TODO this is just a placeholder
	QUIT "octo"

pgCurrentSchema() ; TODO this is just a placeholder
	; Until schemas (YDBOcto#99 and/or YDBOcto#417) are supported, "public" is the current schema.
	QUIT "public"

pgCurrentSchemas(implicit) ; TODO this is just a placeholder
	QUIT ""

pgGetUserById(a) ; TODO this is just a placeholder
	; Returns a role's name given its OID.
	; Octo currently doesn't distinguish between roles so return "octo" as it is the only role present
        QUIT "octo"

pgIsInRecovery() ; TODO this is just a placeholder
	; This function returns true if the database is in the process of recovering from a failure by restoring a backup.
	; Since Octo doesn't support this feature presently, just return false (0).
	QUIT 0

pgIsXlogReplayPaused() ; TODO this is just a placeholder
	; This function returns true if the database has paused the process of recovering from a failure by restoring a backup.
	; Since Octo doesn't support this feature presently, just return false (0).
	QUIT 0

pgUser()
	; Look up the name of the currently active user. Note that this function currently implements three SQL functions:
	; current_user, current_role, and session_user. This is because Octo presently does not distinguish between roles, users,
	; and session users. If these distinctions are ever implemented, then this function will need to be broken up into several
	; different functions accordingly.
	;
	; Similarly, note also that currently no session_id is needed to look up the username for the current session. This is because the
	; session_id will always be the same, as described in rocto.c where the session_id is created by ydb_incr_s. However, if
	; sessions are distinguished in the future and, accordingly, the session_id may vary, then this code will need to be updated
	; to include the session_id as a subscript for the lookup of the username.
	QUIT $GET(%ydboctosession("user"))

pgEncodingToChar(encnum)
	; Convert encoding from INTEGER representation to VARCHAR.
	; Until pg_database is fully implemented (#417), always return SQL_ASCII.
	QUIT "SQL_ASCII"

pgGetExpr(a,b,c,d,e,f) ; TODO this is just a placeholder
	QUIT ""

pgHasDatabasePrivilege(user,database,privilege)
	; Since Octo currently does not implement privileges, always return TRUE.
	; Note that there is a two-argument version of this function as well. If Octo ever implements privileges,
	; logic for that version must be implemented as well.
	QUIT 1

pgObjDescription(a,b)
	QUIT "No description"

pgRowNumber()
	QUIT $I(%ydboctopgRowNumber)

pgTableIsVisible(a) ; TODO this is just a placeholder
	; This function returns true i.e 1 if a certain object
	; is visible in the current schema search path.
	; For example, a table is said to be visible if its containing
	; schema is in the search path and no table of the same name appears earlier in the search path.
        QUIT 1

setConfig(setting,value,local)
	SET %ydboctosession(0,"variables",setting)=value
	QUIT %ydboctosession(0,"variables",setting)

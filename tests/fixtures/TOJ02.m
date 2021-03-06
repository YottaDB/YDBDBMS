;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;								;
; Copyright (c) 2019-2020 YottaDB LLC and/or its subsidiaries.	;
; All rights reserved.						;
;								;
;	This source code contains the intellectual property	;
;	of its copyright holder(s), and is made available	;
;	under a license.  If you do not know the terms of	;
;	the license, please stop and do not read further.	;
;								;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; -----------------------------------------------------------------------------------------------------
; Below are all queries that were found in the following files.
;	test_inner_join_order_by.bats.in
;	test_inner_join_where.bats.in
;	test_inner_join_where_order_by.bats.in
; Each of those queries are run with a LEFT or RIGHT or FULL JOIN and the output is compared against Postgres.
; This simplifies the task of maintaining reference files for these.
; -----------------------------------------------------------------------------------------------------

genouterjoinonpastas;
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta order by pastas.id, firstName;"
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta order by pastas.id asc, firstName;"
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta order by pastas.id desc, firstName;"
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta order by lastName, firstName, pastas.id;"
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta order by lastName asc, firstName, pastas.id;"
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta order by lastName desc, firstName, pastas.id;"
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta order by firstName, lastname, pastas.id;"
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta order by firstName asc, lastname, pastas.id;"
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta order by firstName desc, lastname, pastas.id;"
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta order by pastaName, firstName, pastas.id;"
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta order by pastaName asc, firstName, pastas.id;"
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta order by pastaName desc, firstName, pastas.id;"
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta where lastName = 'Buttons';"
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta where firstName = 'Zero';"
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta where firstName = 'Zero' and names4.favoritePasta = 'Lasagna';"
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta where names4.favoritePasta = 'Penne';"
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta where names4.favoritePasta = 'Spaghetti';"
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta where names4.favoritePasta = 'Cavatelli';"
	set query($increment(query))="select distinct pastas.id, favoritePasta from names4 inner join pastas on pastas.pastaName = names4.favoritePasta where names4.favoritePasta = 'Cavatelli';"
	set query($increment(query))="select pastas.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta where lastName = 'Buttons' order by firstName, 1;"
	set query($increment(query))="select names4.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta where firstName = 'Zero' order by names4.id, 2;"
	set query($increment(query))="select names4.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta where firstName = 'Zero' order by lastName, 1;"
	set query($increment(query))="select names4.id, firstName, lastName, pastaName from names4 inner join pastas on pastas.pastaName = names4.favoritePasta where favoritePasta = 'Cavatelli' order by firstName, 1;"
	set query($increment(query))="select distinct pastas.id, favoritePasta from names4 inner join pastas on pastas.pastaName = names4.favoritePasta where favoritePasta = 'Cavatelli' order by pastas.id, 2;"
	for i=1:1:query do
	. for type="left","right","full" do
	. . set file="jointest"_$translate($justify(i,2)," ","0")_type_".sql"
	. . open file:(newversion)  use file
	. . write $piece(query(i),"inner",1)_type_$piece(query(i),"inner",2),!	; replace inner join with left join or right join or full join (assumes only one "inner join" usage)
	. . close file
	quit
	;

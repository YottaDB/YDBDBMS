select id from names where '\\' like '\';
select id from names where '\\' not like '\';
select id from names where '\\' ~~ '\';
select id from names where '\\' !~~ '\';
select id from names where '\' like '\';
select id from names where '\' not like '\';
select id from names where '\' ~~ '\';
select id from names where '\' !~~ '\';
select firstname from (select 8 as id,'ey' as firstname, '\' as lastname)n1 where firstname LIKE lastname;

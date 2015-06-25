\pset pager off
\set ECHO queries

drop extension if exists osrmint;
create extension if not exists osrmint with schema public;

-- 1
SELECT id, tdist, ttime FROM osrmint_route(
'SELECT CAST(1 AS integer) AS id, CAST(-56.0961 AS float) AS xf, CAST(-34.8496 AS float) AS yf, CAST(-56.1296 AS float) AS xt, CAST(-34.8993 AS float) AS yt',
'http://127.0.0.1:5000/viaroute');
SELECT id, tdist, ttime INTO TEMP pepe FROM osrmint_route(
'SELECT CAST(1 AS integer) AS id, CAST(-56.0961 AS float) AS xf, CAST(-34.8496 AS float) AS yf, CAST(-56.1296 AS float) AS xt, CAST(-34.8993 AS float) AS yt',
'http://127.0.0.1:5000/viaroute');
-- 2
SELECT id, tdist, ttime FROM osrmint_route('SELECT * FROM route ORDER BY random() LIMIT 50','http://127.0.0.1:5000/viaroute');
SELECT id, tdist, ttime INTO TEMP pepe2 FROM osrmint_route('SELECT * FROM route ORDER BY random() LIMIT 50','http://127.0.0.1:5000/viaroute');
-- 3
SELECT id, tdist, ttime FROM osrmint_route('SELECT * FROM route WHERE id=-99999 ORDER BY random() LIMIT 50','http://127.0.0.1:5000/viaroute');

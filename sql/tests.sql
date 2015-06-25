-- Testeo osrmint_route
-- http://localhost:5000/viaroute?loc=-34.8496,-56.0961&loc=-34.8993,-56.1296&alt=false&geometry=false&instructions=false

-- 1
SELECT id, tdist, ttime FROM osrmint_route(
'SELECT CAST(1 AS integer) AS id, CAST(-56.0961 AS float) AS xf, CAST(-34.8496 AS float) AS yf, CAST(-56.1296 AS float) AS xt, CAST(-34.8993 AS float) AS yt',
'http://127.0.0.1:5000/viaroute');
SELECT id, tdist, ttime INTO TEMP pepe FROM osrmint_route(
'SELECT CAST(1 AS integer) AS id, CAST(-56.0961 AS float) AS xf, CAST(-34.8496 AS float) AS yf, CAST(-56.1296 AS float) AS xt, CAST(-34.8993 AS float) AS yt',
'http://127.0.0.1:5000/viaroute');
-- 2
SELECT id, tdist, ttime FROM osrmint_route('SELECT * from route LIMIT 50','http://127.0.0.1:5000/viaroute');
SELECT id, tdist, ttime INTO TEMP pepe2 FROM osrmint_route('SELECT * from route LIMIT 50','http://127.0.0.1:5000/viaroute');

-- Testeo osrmint_viaroute
-- http://localhost:5000/viaroute?loc=-34.8496,-56.0961&loc=-34.8993,-56.1296&alt=false&geometry=false&instructions=false
SELECT id, tdist, ttime FROM osrmint_route('SELECT * from viaroute LIMIT 50','http://127.0.0.1:5000/viaroute');
-- with insert into temporal table
SELECT id, tdist, ttime INTO TEMP pepe FROM osrmint_route(
'SELECT CAST(1 AS integer) AS id, CAST(-56.0961 AS float) AS xf, CAST(-34.8496 AS float) AS yf, CAST(-56.1296 AS float) AS xt, CAST(-34.8993 AS float) AS yt',
'http://127.0.0.1:5000/viaroute');


COPY (SELECT id, longitud, latitud  FROM contenedor)
TO '/tmp/contenedor.csv'
WITH DELIMITER ';'
CSV HEADER;

COPY viaroute(id, y, x)
FROM '/tmp/contenedor.csv'
WITH DELIMITER ';'
CSV HEADER;

\pset pager off
\set ECHO queries

-- Testeo osrmint_viaroute
-- http://localhost:5000/viaroute?loc=-34.8496,-56.0961&loc=-34.8993,-56.1296&alt=false&geometry=false&instructions=false
SELECT cjson FROM osrmint_viaroute('SELECT id AS seq, id AS nodeid, x, y FROM viaroute ORDER BY random() LIMIT 50','http://127.0.0.1:5000/viaroute');
SELECT cjson FROM osrmint_viaroute('SELECT id AS seq, id AS nodeid, x, y FROM viaroute WHERE id=-999 ORDER BY random() LIMIT 50','http://127.0.0.1:5000/viaroute');
-- with insert into temporal table
SELECT cjson INTO TEMP pepe3 FROM osrmint_viaroute('SELECT id AS seq, id AS nodeid, x, y FROM viaroute ORDER BY random() LIMIT 50','http://127.0.0.1:5000/viaroute');
-- with insert into temporal table
INSERT INTO rjson(rjson) SELECT cjson FROM osrmint_viaroute('SELECT id AS seq, id AS nodeid, x, y FROM viaroute ORDER BY random() LIMIT 50','http://127.0.0.1:5000/viaroute');

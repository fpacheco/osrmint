-- DROP TABLE route;
CREATE TABLE route
(
  id serial NOT NULL,
  ncorrida text NOT NULL,
  observaciones text,
  dt timestamp with time zone NOT NULL DEFAULT now(),
  CONSTRAINT route_pkey PRIMARY KEY (id),
  CONSTRAINT route_ncorrida_key UNIQUE (ncorrida)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE route
  OWNER TO postgres;
GRANT ALL ON TABLE route TO postgres;
GRANT SELECT, INSERT ON TABLE route TO desa;

-- DROP TABLE route_vrptools;
CREATE TABLE route_vrptools
(
  id serial NOT NULL,
  routeid integer,
  seq integer,
  vehicle_id integer,
  node_id integer,
  node_type integer,
  delta_time float,
  cargo integer,
  geom geometry,
  dt timestamp with time zone NOT NULL DEFAULT now(),
  CONSTRAINT route_vrptools_pkey PRIMARY KEY (id),
  CONSTRAINT route_vrptools_routeid_fkey FOREIGN KEY (routeid)
      REFERENCES route (id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE  
)
WITH (
  OIDS=FALSE
);
ALTER TABLE route_vrptools
  OWNER TO postgres;
GRANT ALL ON TABLE route_vrptools TO postgres;
GRANT SELECT, INSERT ON TABLE route_vrptools TO desa;


--DROP TABLE route_json;
CREATE TABLE route_json
(
  id serial NOT NULL,
  routeid integer,
  vehic integer,  
  rjson json,
  CONSTRAINT route_json_pkey PRIMARY KEY (id),
  CONSTRAINT route_json_routeid_fkey FOREIGN KEY (routeid)
      REFERENCES route (id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE route_json
  OWNER TO postgres;
GRANT ALL ON TABLE route_json TO postgres;
GRANT SELECT, INSERT ON TABLE route_json TO desa;


-- DROP TABLE route_point;
CREATE TABLE route_point
(
  id serial NOT NULL,
  rjid integer NOT NULL,
  geom geometry,
  CONSTRAINT route_point_pkey PRIMARY KEY (id),
  CONSTRAINT route_point_rjid_fkey FOREIGN KEY (rjid)
      REFERENCES route_json (id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE route_point
  OWNER TO postgres;
GRANT ALL ON TABLE route_point TO postgres;
GRANT SELECT, INSERT ON TABLE route_point TO desa;


-- DROP TABLE route_line;
CREATE TABLE route_line
(
  id serial NOT NULL,
  rjid integer NOT NULL,
  geom geometry,
  CONSTRAINT route_line_pkey PRIMARY KEY (id),
  CONSTRAINT route_line_rjid_fkey FOREIGN KEY (rjid)
      REFERENCES route_json (id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE route_line
  OWNER TO postgres;
GRANT ALL ON TABLE route_line TO postgres;
GRANT SELECT, INSERT ON TABLE route_line TO desa;


-- DROP TABLE route_instruction;
CREATE TABLE route_instruction
(
  id serial NOT NULL,
  rjid integer NOT NULL,
  seq integer,                -- El orden de la ruta 
  dd  text,                   -- 0: Direccion de sentido (10 = salida, 9 = Punto inermedio de la ruta!!, 15 = llegamos!!)
  wname text,                 -- 1: Nombre de la calle
  tlong integer,              -- 2: Distancia en metros
  ttime integer,              -- 4: Tiempo en segundos
  azim float,                 -- 7: Azimut
  CONSTRAINT route_instruction_pkey PRIMARY KEY (id),
  CONSTRAINT route_instruction_rjid_fkey FOREIGN KEY (rjid)
      REFERENCES route_json (id) MATCH SIMPLE
      ON UPDATE CASCADE ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE route_instruction
  OWNER TO postgres;
GRANT ALL ON TABLE route_instruction TO postgres;
GRANT SELECT, INSERT ON TABLE route_instruction TO desa;


-- Creo una rueva corrida
CREATE OR REPLACE FUNCTION new_route(
    nomcorrida text,    -- Nombre de la corrida
    obs text            -- Observacioens de la corrida
) RETURNS integer AS
$BODY$
DECLARE
    id integer;
BEGIN
    -- Insertar
    EXECUTE format('INSERT INTO route(ncorrida,observaciones) VALUES(%L,%L)', nomcorrida, obs);
    EXECUTE format('SELECT id FROM route WHERE ncorrida=%L AND observaciones=%L', nomcorrida, obs) INTO id;
    RETURN id;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

-- Para las rutas
CREATE OR REPLACE FUNCTION update_routes(
    nomcorrida text,    -- Nombre de la corrida
    url text DEFAULT 'http://localhost:5000/viaroute'
) RETURNS void AS
$BODY$
DECLARE
    ln integer;
    ssql text;
    lns integer[]; 
    i integer;
    routeid integer;
    tmpid integer;
    tmpjson json;
BEGIN
    --RAISE NOTICE 'nomcorrida: %', nomcorrida;
    --RAISE NOTICE 'nomtabla: %', nomtabla;
    --RAISE NOTICE 'url: %', url;
    -- encontrar id de la ruta
    EXECUTE format('SELECT id FROM route WHERE ncorrida = %L', nomcorrida) INTO routeid;
    -- Correr vrptools e insertar 
    EXECUTE 'INSERT INTO route_vrptools(route_id, seq, vehicle_id, node_id, node_type, delta_time, cargo)
    (
        SELECT '|| routeid ||', v.*
        FROM vrp_trashCollection(
        ''select id, x, y, open, close, service, demand, street_id from containers'',
        ''select id, x, y, tw_open as open, tw_close as close from other_locs'',
        ''select vid, start_id, dump_id, end_id, cast(capacity as float), cast(dump_service_time as float) as dumpservicetime, tw_open as starttime, tw_close as endtime from vehicles'',
        ''select from_id, to_id, ttime from tiempos''
        )
    )';
    -- Update de geom     
    EXECUTE format('UPDATE route_vrptools
        SET geom = gg.geom
        FROM
            (
                SELECT *
                FROM
                (
                    -- de contenedores
                    (
                        SELECT v.node_id, c.geom
                        FROM route_vrptools v, contenedores_geom c
                        WHERE v.node_id=c.id AND v.routeid=%L
                    )
                    UNION
                    -- de other_locs
                    (
                        SELECT v.node_id, o.geom
                        FROM route_vrptools v, other_locs o
                        WHERE v.node_id=o.id AND v.routeid=%L
                    )
                ) AS g
            ) AS gg
        WHERE
            gg.node_id = vrptools_majusamat201503271226.node_id', routeid, routeid);

    -- Los vehiculos de esa corrida al array
    EXECUTE format('SELECT ARRAY(SELECT DISTINCT vehicle_id FROM route_vrptools WHERE routeid=%L)', routeid) INTO lns;
    -- loop para todos los vehiculos de esta corrida
    FOR i IN array_lower(lns, 1) .. array_upper(lns, 1)
    LOOP    
        -- inserto el json de esta linea y esta corrida (ORDER BY v.seq!!!!!!)
        ssql := 'SELECT v.seq, v.node_id as nodeid, ST_X(v.geom) as x, ST_Y(v.geom) AS y FROM route_vrptools v WHERE v.vehicle_id='|| lns[i] ||' ORDER BY v.seq;';
        -- RAISE NOTICE 'ssql: %', ssql;
        EXECUTE 'INSERT INTO route_json(ncorrida, ntabla, vehic, rjson)
        (
            SELECT '|| routeid ||','|| lns[i] ||', CAST(r.cjson AS json)
            FROM
                ( SELECT * FROM osrmint_viaroute('''||ssql||''','''||url||''') ) AS r
        )';

        -- obtengo datos necesarios 
        EXECUTE format('SELECT id FROM route_json WHERE routeid=%L AND vehic=%L', routeid, lns[i]) INTO tmpid;
        -- RAISE NOTICE 'tmpid: %', tmpid;
        EXECUTE format('SELECT rjson FROM route_json WHERE routeid=%L AND vehic=%L', routeid, lns[i]) INTO tmpjson;
        -- RAISE NOTICE 'tmpjson: %', tmpjson;
        
        -- inserto los puntos de la ruta
        EXECUTE 'INSERT INTO route_point(rjid, geom)
        (
            SELECT '|| tmpid ||', r.geom
            FROM
                ( SELECT * FROM osrmint_getRoutePoints('''||tmpjson||''') ) AS r
        )';

        -- inserto la linea de la ruta
        EXECUTE 'INSERT INTO route_line(rjid, geom)
        (
            SELECT '|| tmpid ||', r.geom
            FROM
                ( SELECT * FROM osrmint_getRouteLine('''||tmpjson||''') ) AS r
        )';

        -- inserto las instrucciones de la ruta
        EXECUTE 'INSERT INTO route_instruction(rjid, seq, dd, wname, tlong, ttime, azim)
        (
            SELECT '|| tmpid ||', r.*
            FROM
                ( SELECT * FROM osrmint_getRouteInstructions('''||tmpjson||''') ) AS r
        )';
        -- fin de esta linea ... a la siguiente
    END LOOP;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

INSERT INTO containers(id,x,y,demand) (
     SELECT
     id,
     ST_X(ST_TRANSFORM(geom,4326)),
     ST_Y(ST_TRANSFORM(geom,4326)),
     cast(capacidad as float)
     FROM contenedores_matuD_DU_RM_CL_07
     WHERE turno_hora='MARTES JUEVES Y SABADOS CON FERIADOS LABORABLES: Matutino (06 a 14 hrs.)'
);


-- Todas las distancias de esta corrida
SELECT *
FROM
(
(SELECT n.* FROM node_distance n, containers c WHERE n.nfrom=c.id)
UNION
(SELECT n.* FROM node_distance n, containers c WHERE n.nto=c.id)
UNION
(SELECT n.* FROM node_distance n, containers c, other_locs o WHERE n.nfrom=o.id AND c.id=n.nto)
UNION
(SELECT n.* FROM node_distance n, containers c, other_locs o WHERE n.nto=o.id AND c.id=n.nfrom)
) AS nd
ORDER BY nd.nfrom;

-- para pasarle a osrmint !!
SELECT nd.id, ST_X(nd.geomf), ST_Y(nd.geomf), ST_X(nd.geomt), ST_Y(nd.geomt)
FROM
    -- Solo los nodos de esa corrida y entre las other_locs que son pocas (si estuvieran cerca)
    (SELECT *
        FROM
        (
            -- de contenedores a cotenedor y viceversa
            (
                SELECT n.*, g.geom AS geomf, gg.geom as geomt
                FROM node_distance n, containers c, containers cc, contenedores_geom g, contenedores_geom gg
                WHERE n.nfrom=c.id AND n.nto=cc.id AND n.nfrom=g.id AND n.nto=gg.id
            )
            UNION
            -- de contenedores (FROM) a other_locs (to)
            (
                SELECT n.*, g.geom AS geomf, o.geom as geomt
                FROM node_distance n, containers c, contenedores_geom g, other_locs o
                WHERE n.nfrom=c.id AND n.nto=o.id AND n.nfrom=g.id AND n.nto=o.id
            )
            UNION
            -- de other_locs (FROM) a contenedores (to)
            (
                SELECT n.*, o.geom AS geomf, g.geom as geomt
                FROM node_distance n, containers c, contenedores_geom g, other_locs o
                WHERE n.nfrom=o.id AND n.nto=c.id AND n.nfrom=o.id AND n.nto=g.id
            )
            UNION
            -- entre las others_locs
            (
                SELECT n.*, o.geom AS geomf, oo.geom as geomt
                FROM node_distance n, other_locs o, other_locs oo
                WHERE n.nfrom=o.id AND n.nto=oo.id
            )
        ) AS d
    ) as nd
ORDER BY nd.id;



-- Corro el VRPTOOLS
-- DROP TABLE vrptools_majusamat201503271226;
SELECT *
INTO vrptools_majusamat201503271226
FROM
  vrp_trashCollection(
    'select id, x, y, open, close, service, demand, street_id from containers',
    'select id, x, y, tw_open as open, tw_close as close from other_locs',
    'select vid, start_id, dump_id, end_id, cast(capacity as float), cast(dump_service_time as float) as dumpservicetime, tw_open as starttime, tw_close as endtime from vehicles',
    'select from_id, to_id, ttime from tiempos'
);
-- Agrego geometria a la salida
ALTER TABLE vrptools_majusamat201503271226 ADD COLUMN geom geometry(Point,4326);
-- Update de geometria de la salida
UPDATE vrptools_majusamat201503271226
SET geom = gg.geom
FROM
    (
        SELECT *
        FROM
        (
            -- de contenedores
            (
                SELECT v.node_id, c.geom
                FROM vrptools_majusamat201503271226 v, contenedores_geom c
                WHERE v.node_id=c.id
            )
            UNION
            -- de other_locs
            (
                SELECT v.node_id, o.geom
                FROM vrptools_majusamat201503271226 v, other_locs o
                WHERE v.node_id=o.id
            )
        ) AS g
    ) AS gg
WHERE
    gg.node_id = vrptools_majusamat201503271226.node_id;


-- El json del OSRM para la ruta
SELECT *
FROM
    osrmint_viaroute(
        'SELECT v.seq, v.node_id as nodeid, ST_X(v.geom) as x, ST_Y(v.geom) AS y FROM vrptools_majusamat201503271226 v ORDER BY v.seq;',
        'http://localhost:5000/viaroute'
    )

-- La línea
SELECT * FROM osrmint_getRouteLine( (select * FROM osrmint_viaroute('SELECT c.id AS seq, c.id as nodeid, ST_X(c.geom) as x, ST_Y(c.geom) AS y FROM contenedores_geom c ORDER BY c.id LIMIT 10;', 'http://localhost:5000/viaroute')) );


-- Obtengo el json
INSERT INTO vrptools_json(nombre, camion, cjson)
(
    SELECT 'majusamat201503271226', vehicle_id, cjson
    FROM
        
    (SELECT DISTINCT vehicle_id FROM majusamat201503271226)
)



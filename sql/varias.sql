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
  seq integer NOT NULL,
  calle text,
  pseq integer,
  osrmlong float,
  osrmtime float,
  osrmazim float,
  osrmfisint boolean,
  gacumdist float,
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


-- Function: update_node_distance(double precision)
-- RFPV: Para acortar tiempos, solo tiene en cuenta datos en containers
-- DROP FUNCTION update_node_distance(double precision);
CREATE OR REPLACE FUNCTION update_node_distance(distancia_m double precision DEFAULT 1000)
  RETURNS void AS
$BODY$
DECLARE
    muni VARCHAR(3);
BEGIN
    FOR muni IN SELECT DISTINCT municipio FROM sig_municipios
    LOOP
      -- compute partial distance matrix
      INSERT INTO node_distance (municipio, nfrom, nto, dist)
        SELECT muni, nfrom, nto, dist FROM (
           -- De contenedor a contenedor a una distancia = distancia_m
           SELECT a.id AS nfrom, b.id AS nto, st_distance(a.geom::geography, b.geom::geography) AS dist
            FROM contenedores_geom a, contenedores_geom b, containers cs, containers ccs
            WHERE st_dwithin(a.geom::geography, b.geom::geography, distancia_m) AND not a.id = b.id and
                 left(a.cod_recorr,1)=muni AND left(b.cod_recorr,1)=muni AND a.id=cs.id AND b.id=ccs.id
         UNION
           -- De contenedores a sitios
           SELECT a.id AS nfrom, b.id AS nto, st_distance(a.geom::geography, b.geom::geography) AS dist
            FROM contenedores_geom a, other_locs b, containers cs
            WHERE left(a.cod_recorr,1)=muni AND a.id=cs.id
         UNION
           -- De sitios a contenedores
           SELECT b.id AS nfrom, a.id AS nto, st_distance(a.geom::geography, b.geom::geography) AS dist
            FROM contenedores_geom a, other_locs b, containers cs
            WHERE left(a.cod_recorr,1)=muni AND a.id=cs.id
         ) AS foo
         ORDER BY nfrom, nto;
    END LOOP;

    -- Entre sitios municipio NULL
    INSERT INTO node_distance (nfrom, nto, dist)
      SELECT a.id AS nfrom, b.id AS nto, st_distance(a.geom::geography, b.geom::geography) AS dist
        FROM other_locs a, other_locs b
        WHERE not a.id = b.id;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION update_node_distance(double precision)
  OWNER TO postgres;


-- Sustituye a tiempos
-- DROP TABLE node_osrm_data;
CREATE TABLE node_osrm_data
(
  id integer NOT NULL,
  nfrom integer,
  nto integer,
  osrmdist double precision,
  osrmtime double precision,
  CONSTRAINT tiempos_pkey PRIMARY KEY (id)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE node_osrm_data
  OWNER TO postgres;
GRANT ALL ON TABLE node_osrm_data TO postgres;
GRANT SELECT, INSERT ON TABLE node_osrm_data TO desa;


-- Function: update_node_osrm_data(text)
-- RFPV: Para acortar tiempos, solo tiene en cuenta datos en containers
-- DROP FUNCTION update_node_osrm_data(text);
CREATE OR REPLACE FUNCTION update_node_osrm_data(base_url text DEFAULT 'http://localhost:5000/viaroute')
  RETURNS void AS
$BODY$
DECLARE
    -- Nada para declarar?
    sqlp text;
BEGIN
    DELETE FROM node_osrm_data;
    -- para pasarle a osrmint !!
    sqlp := 'SELECT nd.id AS id, ST_X(nd.geomf) AS xf, ST_Y(nd.geomf) AS yf, ST_X(nd.geomt) AS xt, ST_Y(nd.geomt) AS yt
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
    ORDER BY nd.id';
    -- osrmtime en segundos. VRPTOOLS debe ser en minutos => /60!.
    EXECUTE format('INSERT INTO node_osrm_data(id,osrmdist,osrmtime)
    (SELECT id, tdist, ttime/60.0 FROM osrmint_route(%L,%L))', sqlp, base_url);
    -- Add nfrom, nto
    UPDATE node_osrm_data SET nfrom=nd.nfrom, nto=nd.nto FROM node_distance AS nd
    WHERE node_osrm_data.id=nd.id;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION update_node_osrm_data(text)
  OWNER TO postgres;

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
    sqlcont text;
    sqlol text;
    sqlve text;
    sqlt text; 
BEGIN
    --RAISE NOTICE 'nomcorrida: %', nomcorrida;
    --RAISE NOTICE 'nomtabla: %', nomtabla;
    --RAISE NOTICE 'url: %', url;

    -- Update node_distance solo con containers
    RAISE NOTICE '1 - Actualizando node_distance';
    DELETE FROM node_distance;
    PERFORM SELECT update_node_distance();

    -- Update tiempos
    RAISE NOTICE '2 - Actualizando node_osrm_data';
    DELETE FROM node_osrm_data;
    PERFORM SELECT update_node_osrm_data();
    
    -- encontrar id de la ruta
    EXECUTE format('SELECT id FROM route WHERE ncorrida = %L', nomcorrida) INTO routeid;

    -- Correr vrptools e insertar
    RAISE NOTICE '3 - Ejecutando vrp_trashCollection';
    sqlcont := 'select id, x, y, open, close, service, demand, street_id from containers';
    sqlol := 'select id, x, y, tw_open as open, tw_close as close from other_locs';
    sqlve := 'select vid, start_id, dump_id, end_id, cast(capacity as float), cast(dump_service_time as float) as dumpservicetime, tw_open as starttime, tw_close as endtime from vehicles';
    sqlt := 'select nfrom AS from_id, nto AS to_id, osrmtime AS ttime from node_osrm_data';    
    EXECUTE format('INSERT INTO route_vrptools(route_id, seq, vehicle_id, node_id, node_type, delta_time, cargo)
    (
        SELECT '|| routeid ||', v.*
        FROM vrp_trashCollection(%L, %L, %L, %L)
    )',sqlcont,sqlol,sqlve, sqlt);
    
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
            gg.node_id = v.node_id', routeid, routeid);

    -- Los vehiculos de esa corrida al array
    EXECUTE format('SELECT ARRAY(SELECT DISTINCT vehicle_id FROM route_vrptools WHERE routeid=%L)', routeid) INTO lns;

    -- loop para todos los vehiculos de esta corrida
    RAISE NOTICE '4 - Insertando json y lineas';
    FOR i IN array_lower(lns, 1) .. array_upper(lns, 1)
    LOOP    
        -- inserto el json de esta linea y esta corrida (ORDER BY v.seq!!!!!!)
        ssql := 'SELECT v.seq, v.node_id as nodeid, ST_X(v.geom) as x, ST_Y(v.geom) AS y FROM route_vrptools v WHERE v.vehicle_id='|| lns[i] ||' ORDER BY v.seq;';
        -- RAISE NOTICE 'ssql: %', ssql;
        EXECUTE 'INSERT INTO route_json(routeid, vehic, rjson)
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
        
        -- inserto la linea de la ruta
        EXECUTE format('INSERT INTO route_line(rjid, seq, calle, pseq, osrmlong, osrmtime, osrmazim, osrmfisint, gacumdist, geom)
        (
            SELECT r.*
            FROM
                ( SELECT * FROM osrmint_getRouteLine(%L,%L) ) AS r
        )', tmpjson, tmpid);
        
        -- fin de esta linea ... a la siguiente
    END LOOP;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;

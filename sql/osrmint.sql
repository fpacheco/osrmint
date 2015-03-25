-- Functions in osrmint

-- Not load this file from from psql 
\echo Use "CREATE EXTENSION osrmint;" to load this file. \quit

---------------------------------------------------------------------
-- Core functions to access OSRM from postgresql
-- Author: Fernando Pacheco <fernando.pacheco@ingesur.com.uy>
-- Date: 2015-03-20
---------------------------------------------------------------------

CREATE OR REPLACE FUNCTION osrmint_route (        
        IN datapoint_sql text, -- sql
        IN base_url text, -- http://localhost:5000/viaroute
        OUT id integer,
        OUT tdist float8,
        OUT ttime float8
    ) RETURNS RECORD
    AS 'MODULE_PATHNAME', 'osrmint_route'
    LANGUAGE c STABLE STRICT;
--COMMENT ON FUNCTION osrmint_route(geometry) IS 'args: datapoint_sql: valid SQL (id,xf,yf,xt,yt), base_url: comething like http://localhost:5000/viaroute - Return records with id, travel distance,travel time from OSRM.';

CREATE OR REPLACE FUNCTION osrmint_viaroute (        
        IN dataviaroute_sql text, -- sql
        IN base_url text, -- http://localhost:5000/viaroute
        OUT cjson text
    ) RETURNS text
    AS 'MODULE_PATHNAME', 'osrmint_viaroute'
    LANGUAGE c STABLE STRICT;
--COMMENT ON FUNCTION osrmint_viaroute(geometry) IS 'args: datapoint_sql: valid SQL (id,xf,yf,xt,yt), base_url: comething like http://localhost:5000/viaroute - Return records with id, travel distance, travel time from OSRM.';


-- SELECT * FROM osrmint_getRoutePoints( (select * FROM osrmint_viaroute('SELECT c.id AS seq, c.id as nodeid, ST_X(c.geom) as x, ST_Y(c.geom) AS y FROM contenedores_geom c ORDER BY c.id LIMIT 10;', 'http://localhost:5000/viaroute')) );
CREATE OR REPLACE FUNCTION osrmint_getRoutePoints(
    IN json_osrm text, -- Texto de ruta OSRM
    IN SRID int DEFAULT 4326, -- SRID 
    OUT geom geometry -- Geometria de salida    
) RETURNS SETOF geometry AS
$BODY$
DECLARE
  i integer;
  alen integer;
  sjson json;
  X float;
  Y float;
BEGIN
    sjson := json_osrm::json->'route_geometry';
    alen := json_array_length(sjson)-1;
    FOR i in 0 .. alen LOOP
        X := CAST(sjson->i->>1 AS float);
        Y := CAST(sjson->i->>0 AS float);
        geom := ST_SetSRID( ST_MakePoint(X, Y), SRID);
        RETURN NEXT;
    END LOOP;
END;
$BODY$
  LANGUAGE plpgsql STABLE STRICT
  COST 100
  ROWS 1000;

-- SELECT * FROM osrmint_getRouteLine( (select * FROM osrmint_viaroute('SELECT c.id AS seq, c.id as nodeid, ST_X(c.geom) as x, ST_Y(c.geom) AS y FROM contenedores_geom c ORDER BY c.id LIMIT 10;', 'http://localhost:5000/viaroute')) );
CREATE OR REPLACE FUNCTION osrmint_getRouteLine(
    IN json_osrm text, -- Texto de ruta OSRM
    IN SRID int DEFAULT 4326, -- SRID 
    OUT geom geometry -- Geometria de salida    
) RETURNS SETOF geometry AS
$BODY$
DECLARE
  --nada para declarar!
BEGIN
    geom := ST_MakeLine(ARRAY(SELECT osrmint_getRoutePoints(json_osrm, SRID)));
    RETURN NEXT;
END;
$BODY$
  LANGUAGE plpgsql STABLE STRICT
  COST 100
  ROWS 1000;

-- SELECT * FROM osrmint_getRouteInstructions( (select * FROM osrmint_viaroute('SELECT c.id AS seq, c.id as nodeid, ST_X(c.geom) as x, ST_Y(c.geom) AS y FROM contenedores_geom c ORDER BY c.id LIMIT 10;', 'http://localhost:5000/viaroute')) );
CREATE OR REPLACE FUNCTION osrmint_getRouteInstructions(
    IN json_osrm text, -- Texto de ruta OSRM
    OUT seq integer, -- Orden (1,2,3,4,5,6)
    OUT dd integer, -- 0: Direccion de sentido (10 = salida, 9 = Punto inermedio de la ruta!!, 15 = llegamos!!)
    OUT wname text, -- 1: Nombre de la calle
    OUT tlong integer, -- 2: Distancia en metros
    OUT ttime integer, -- 4: Tiempo en segundos
    OUT azim float -- 7: Azimut
) RETURNS SETOF RECORD AS
$BODY$
DECLARE
  i integer;
  alen integer;
  sjson json;
BEGIN
    sjson := json_osrm::json->'route_instructions';
    alen := json_array_length(sjson)-1;
    FOR i in 0 .. alen LOOP
        seq := i+1;
        dd := CAST(sjson->i->>0 AS integer);
        wname := sjson->i->>1;
        tlong := CAST(sjson->i->>2 AS float);
        ttime := CAST(sjson->i->>4 AS float);
        azim := CAST(sjson->i->>7 AS float);
        RETURN NEXT;
    END LOOP;
END;
$BODY$
  LANGUAGE plpgsql STABLE STRICT
  COST 100
  ROWS 1000;


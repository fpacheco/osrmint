-- Functions in osrmint

-- Not load this file from from psql
\echo Use "CREATE EXTENSION osrmint;" to load this file. \quit

---------------------------------------------------------------------
-- Core functions to access OSRM from postgresql
-- Author: Fernando Pacheco <fernando.pacheco@ingesur.com.uy>
-- Date: 2015-03-20
---------------------------------------------------------------------

CREATE OR REPLACE FUNCTION osrmint_route (
        IN datapoint_sql text,                  -- sql
        IN base_url text,                       -- http://localhost:5000/viaroute
        OUT id integer,
        OUT tdist float8,
        OUT ttime float8
    ) RETURNS RECORD
    AS 'MODULE_PATHNAME', 'osrmint_route'
    LANGUAGE c STABLE STRICT;
--COMMENT ON FUNCTION osrmint_route(geometry) IS 'args: datapoint_sql: valid SQL (id,xf,yf,xt,yt), base_url: comething like http://localhost:5000/viaroute - Return records with id, travel distance,travel time from OSRM.';

CREATE OR REPLACE FUNCTION osrmint_viaroute (
        IN dataviaroute_sql text,               -- sql
        IN base_url text,                       -- http://localhost:5000/viaroute
        OUT cjson text
    ) RETURNS text
    AS 'MODULE_PATHNAME', 'osrmint_viaroute'
    LANGUAGE c STABLE STRICT;
--COMMENT ON FUNCTION osrmint_viaroute(geometry) IS 'args: datapoint_sql: valid SQL (id,xf,yf,xt,yt), base_url: comething like http://localhost:5000/viaroute - Return records with id, travel distance, travel time from OSRM.';


-- SELECT * FROM osrmint_getRoutePoints( (select * FROM osrmint_viaroute('SELECT c.id AS seq, c.id as nodeid, ST_X(c.geom) as x, ST_Y(c.geom) AS y FROM contenedores_geom c ORDER BY c.id LIMIT 10;', 'http://localhost:5000/viaroute')) );
CREATE OR REPLACE FUNCTION osrmint_getRoutePoints(
    IN json_osrm text,                      -- Texto de ruta OSRM
    IN routeid integer DEFAULT 1,           -- Identificador de los puntos
    IN SRID integer DEFAULT 4326,           -- SRID,
    OUT rid integer,                        -- Identificador de los puntos
    OUT seq integer,                        -- Orden
    OUT geom geometry                       -- Geometria de salida
) RETURNS SETOF RECORD AS
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
        rid := routeid;
        seq := i+1;
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
    IN json_osrm text,                      -- Texto de ruta OSRM
    IN routeid integer DEFAULT 1,           -- Identificador de los puntos
    IN SRID integer DEFAULT 4326,           -- SRID
    OUT rid integer,                        -- Identificador de los puntos
    OUT seq integer,                        -- Orden
    OUT calle text,                         -- instruccion
    OUT pseq integer,                       -- sequencia del punto de la ruta
    OUT osrmlong float,                     -- longitud de la linea
    OUT osrmtime float,                     -- osrm time
    OUT osrmazim float,                     -- osrm time
    OUT osrmfisint boolean,                 -- Node from is intermediate point in osrm route
    OUT gacumdist float,                    -- distancia acumulada
    OUT geom geometry                       -- Geometria de salida
) RETURNS SETOF RECORD AS
$BODY$
DECLARE
  i integer;
  alen integer;
  sjson json;
  ijson json;
  Xf float;                                 -- Xfrom
  Yf float;                                 -- Yfrom
  Xt float;                                 -- Xto
  Yt float;                                 -- Yto
  ncid integer;
  j integer;
  k integer;
  acumdist float;
BEGIN
    -- OJO: length(sjson)!=length(ijson) !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    -- length(ijson) <= length(sjson)
    sjson := json_osrm::json->'route_geometry';
    ijson := json_osrm::json->'route_instructions';
    alen := json_array_length(sjson)-2;
    j := 0;
    k := 1;
    acumdist := 0;
    FOR i in 0 .. alen LOOP
        rid := routeid;
        seq := i+1;
        ncid := CAST( ijson->j->>3 AS integer);
        pseq := NULL;
        osrmlong := NULL;
        osrmtime := NULL;
        osrmazim := NULL;
        osrmfisint := FALSE;
        IF i >= ncid THEN
            calle := ijson->j->>1;                                      -- Nombre de la calle
            osrmlong := CAST(ijson->j->>2 AS float);                    -- En metros
            osrmtime := CAST(ijson->j->>4 AS float);                    -- En segundos
            osrmazim := CAST(ijson->j->>7 AS float);                    -- En grados
            IF (CAST( left(ijson->j->>0, 1) AS integer) = 9) OR (CAST( left(ijson->j->>0, 2) AS integer) = 10) OR (CAST( left(ijson->j->>0, 2) AS integer) = 15) THEN
                pseq := k;                                              -- Secuencia de levante de punto
                k := k+1;
            END IF;
            IF CAST( left(ijson->j->>0, 1) AS integer) = 9 THEN         -- Punto intermedio de ruta
                osrmfisint := TRUE;
            END IF;
            j := j + 1;
        ELSE
            calle := ijson->(j-1)->>1;                                  -- Nombre de la calle
        END IF;
        Xf := CAST(sjson->i->>1 AS float);
        Yf := CAST(sjson->i->>0 AS float);
        Xt := CAST(sjson->(i+1)->>1 AS float);
        Yt := CAST(sjson->(i+1)->>0 AS float);
        geom := ST_MakeLine( ST_SetSRID( ST_MakePoint(Xf, Yf), SRID), ST_SetSRID( ST_MakePoint(Xt, Yt), SRID) );
        acumdist := acumdist + ST_Length(geom::geography);
        gacumdist := acumdist;
        RETURN NEXT;
    END LOOP;
END;
$BODY$
  LANGUAGE plpgsql STABLE STRICT
  COST 100
  ROWS 1000;


-- SELECT * FROM osrmint_getRouteInstructions( (select * FROM osrmint_viaroute('SELECT c.id AS seq, c.id as nodeid, ST_X(c.geom) as x, ST_Y(c.geom) AS y FROM contenedores_geom c ORDER BY c.id LIMIT 10;', 'http://localhost:5000/viaroute')) );
CREATE OR REPLACE FUNCTION osrmint_getRouteInstructions (
    IN json_osrm text,                      -- Texto de ruta OSRM
    OUT seq integer,                        -- Orden (1,2,3,4,5,6)
    OUT dd varchar(4),                      -- 0: Direccion de sentido (10 = salida, 9 = Punto intermedio de la ruta!!, 15 = llegamos!!)
    OUT wname text,                         -- 1: Nombre de la calle
    OUT tlong integer,                      -- 2: Distancia en metros
    OUT ttime integer,                      -- 4: Tiempo en segundos
    OUT azim float,                         -- 7: Azimut
    OUT gseq integer                        -- 3: Geometry sequence
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
        dd := sjson->i->>0;
        wname := sjson->i->>1;
        tlong := CAST(sjson->i->>2 AS float);
        ttime := CAST(sjson->i->>4 AS float);
        azim := CAST(sjson->i->>7 AS float);
        -- Is 0 based then add one for one based
        gseq := CAST(sjson->i->>3 AS integer) + 1;
        RETURN NEXT;
    END LOOP;
END;
$BODY$
  LANGUAGE plpgsql STABLE STRICT
  COST 100
  ROWS 1000;


--SELECT * FROM osrmint_getRouteInstructionsWT( (select osrmint_viaroute('SELECT c.id AS seq, c.id as nodeid, ST_X(c.geom) as x, ST_Y(c.geom) AS y FROM contenedores_geom c ORDER BY c.id LIMIT 10;', 'http://localhost:5000/viaroute')) );
CREATE OR REPLACE FUNCTION osrmint_getRouteInstructionsWT (
    IN json_osrm text,                      -- Texto de ruta OSRM
    OUT seq integer,                        -- Orden (1,2,3,4,5,6)
    OUT ddt text,                           -- 0: Direccion de sentido (10 = salida, 9 = Punto inermedio de la ruta!!, 15 = llegamos!!)
    OUT wname text,                         -- 1: Nombre de la calle
    OUT tlong integer,                      -- 2: Distancia en metros
    OUT ttime integer,                      -- 4: Tiempo en segundos
    OUT azim float                          -- 7: Azimut
) RETURNS SETOF RECORD AS
$BODY$
DECLARE
BEGIN
    RETURN QUERY SELECT g.seq, f.instruction AS ddt, g.wname, g.tlong, g.ttime, g.azim
    FROM
        (SELECT * FROM osrmint_getRouteInstructions(json_osrm)) AS g,
        (SELECT * FROM tinstruction) AS f
    WHERE
        g.dd=f.dd;
END;
$BODY$
  LANGUAGE plpgsql STABLE STRICT
  COST 100
  ROWS 1000;

-- SELECT * FROM osrmint_getViaIndices( (select rjson FROM ruteojson WHERE ruteorun=102), (SELECT ARRAY(SELECT nodo FROM ruteosecuencia WHERE ruteorun=102 ORDER BY secuencia)) );
CREATE OR REPLACE FUNCTION osrmint_getViaIndices (
    IN json_osrm text,                      -- Texto de ruta OSRM
    IN ids integer[],                       -- Id de los indices
    OUT id integer,                         -- Id de salida
    OUT gseq integer                        -- 3: Geometry sequence (base 1)
) RETURNS SETOF RECORD AS
$BODY$
DECLARE
  i integer;
  idslen integer;
  jalen integer;
  sjson json;
BEGIN
    sjson := json_osrm::json->'via_indices';
    jalen := json_array_length(sjson);
    idslen := array_length(ids, 1);
    IF jalen != idslen THEN
      RAISE EXCEPTION 'Las dimensiones de los ids de entrada y del elemento via_indices son distintas'
      USING HINT = 'Por favor verifique';
    END IF;
    -- Para el loop
    jalen := jalen - 1;
    FOR i in 0 .. jalen LOOP
        id := ids[i+1];
        -- Is 0 based then add one for one based
        gseq := CAST(sjson->>i AS integer) + 1;
        RETURN NEXT;
    END LOOP;
END;
$BODY$
  LANGUAGE plpgsql STABLE STRICT
  COST 100
  ROWS 1000;

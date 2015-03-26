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
    IN json_osrm text,              -- Texto de ruta OSRM
    IN SRID int DEFAULT 4326,       -- SRID 
    OUT geom geometry               -- Geometria de salida    
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
CREATE OR REPLACE FUNCTION osrmint_getRouteInstructions (
    IN json_osrm text,              -- Texto de ruta OSRM
    OUT seq integer,                -- Orden (1,2,3,4,5,6)
    OUT dd varchar(4),              -- 0: Direccion de sentido (10 = salida, 9 = Punto inermedio de la ruta!!, 15 = llegamos!!)
    OUT wname text,                 -- 1: Nombre de la calle
    OUT tlong integer,              -- 2: Distancia en metros
    OUT ttime integer,              -- 4: Tiempo en segundos
    OUT azim float                  -- 7: Azimut
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
        RETURN NEXT;
    END LOOP;
END;
$BODY$
  LANGUAGE plpgsql STABLE STRICT
  COST 100
  ROWS 1000;

--SELECT * FROM osrmint_getRouteInstructionsWT( (select osrmint_viaroute('SELECT c.id AS seq, c.id as nodeid, ST_X(c.geom) as x, ST_Y(c.geom) AS y FROM contenedores_geom c ORDER BY c.id LIMIT 10;', 'http://localhost:5000/viaroute')) );
CREATE OR REPLACE FUNCTION osrmint_getRouteInstructionsWT (
    IN json_osrm text,              -- Texto de ruta OSRM
    OUT seq integer,                -- Orden (1,2,3,4,5,6)
    OUT ddt text,                   -- 0: Direccion de sentido (10 = salida, 9 = Punto inermedio de la ruta!!, 15 = llegamos!!)
    OUT wname text,                 -- 1: Nombre de la calle
    OUT tlong integer,              -- 2: Distancia en metros
    OUT ttime integer,              -- 4: Tiempo en segundos
    OUT azim float                  -- 7: Azimut
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

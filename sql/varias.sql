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
 



function turn_function (angle)
    -- compute turn penalty as angle^2, with a left/right bias
    k = turn_penalty/(90.0*90.0)
    if angle>=0 then
        return angle*angle*k/turn_bias
    end
    return angle*angle*k*turn_bias
end

def turn_function (angle):
    k = turn_penalty/(90.0*90.0)
    if angle>=0:
        return angle*angle*k/turn_bias
    return angle*angle*k*turn_bias

-- function turn_function (angle)
--   -- print ("called at angle " .. angle )
--   local index = math.abs(math.floor(angle/10+0.5))+1 -- +1 'coz LUA starts as idx 1
--   local penalty = turn_cost_table[index]
--   -- print ("index: " .. index .. ", bias: " .. penalty )
--   return penalty
-- end



-- Para las rutas
CREATE OR REPLACE FUNCTION update_routes(nomcorrida text, nomtabla regclass, url text DEFAULT 'http://localhost:5000/viaroute')
  RETURNS void AS
$BODY$
DECLARE
    ln integer;
    ssql text;
    lns integer[]; 
    i integer;
    tmpid integer;
    tmpjson json;
BEGIN
    --RAISE NOTICE 'nomcorrida: %', nomcorrida;
    --RAISE NOTICE 'nomtabla: %', nomtabla;
    --RAISE NOTICE 'url: %', url;
    -- Los vehiculos al array
    EXECUTE 'SELECT ARRAY(SELECT DISTINCT vehicle_id FROM ' || nomtabla || ')' INTO lns;
    -- loop para todos los vehiculos de esta corrida    
    FOR i IN array_lower(lns, 1) .. array_upper(lns, 1)
    LOOP    
        -- inserto el json
        ssql := 'SELECT v.seq, v.node_id as nodeid, ST_X(v.geom) as x, ST_Y(v.geom) AS y FROM '|| nomtabla ||' v WHERE v.vehicle_id='|| lns[i] ||' ORDER BY v.seq;';
        RAISE NOTICE 'ssql: %', ssql;
        EXECUTE 'INSERT INTO route_json(ncorrida, ntabla, vehic, rjson)
        (
            SELECT '''|| nomcorrida ||''','''|| nomtabla ||''','|| lns[i] ||', CAST(r.cjson AS json)
            FROM
                ( SELECT * FROM osrmint_viaroute('''||ssql||''','''||url||''') ) AS r
        )';

        -- inserto los puntos de la ruta
        EXECUTE format('SELECT id FROM route_json WHERE ncorrida=%L AND ntabla=%L AND vehic=%L', nomcorrida, nomtabla, lns[i]) INTO tmpid;
        RAISE NOTICE 'tmpid: %', tmpid;
        EXECUTE format('SELECT rjson FROM route_json WHERE ncorrida=%L AND ntabla=%L AND vehic=%L', nomcorrida, nomtabla, lns[i]) INTO tmpjson;
        RAISE NOTICE 'tmpjson: %', tmpjson;
        EXECUTE 'INSERT INTO route_point(routeid, geom)
        (
            SELECT '|| tmpid ||', r.geom
            FROM
                ( SELECT * FROM osrmint_getRoutePoints('''||tmpjson||''') ) AS r
        )';

        -- inserto la linea de la ruta

        -- inserto las instrucciones de la ruta
        
    END LOOP;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;




















def turn_function2 (angle):
    index = abs((angle/10+0.5))+1
    penalty = turn_cost_table[index]
    return penalty

http://127.0.0.1:5000/viaroute?loc=-34.87180,-56.13643&loc=-34.87224,-56.13716&loc=-34.87255&-56.13691
http://127.0.0.1:5000/viaroute?loc=-34.87180,-56.13643&loc=-34.87224,-56.13716&loc=-34.87255,-56.13691&alt=true&geometry=true&compression=false&instructions=true
1 - 60

[
    ["10","Doctor Silvestre Pérez",65,0,14,"64m","SE",129,1],
    ["3","Avenida 8 de Octubre",101,1,15,"101m","SW",225,1],
    ["3","General Félix Laborde",79,2,6,"79m","NW",326,1],
    ["9","General Félix Laborde",42,3,2,"41m","SE",145,1],
    ["15","",0,4,0,"0m","N",0]
]

[
    ["10","Doctor Silvestre Pérez",65,0,15,"64m","SE",129,1],
    ["3","Avenida 8 de Octubre",101,1,15,"101m","SW",225,1],
    ["3","General Félix Laborde",79,2,6,"79m","NW",326,1],
    ["9","General Félix Laborde",27,3,17,"26m","NW",326,1],
    ["7","Joanico",137,4,19,"136m","SW",234,1],
    ["7","Lindoro Forteza",254,5,30,"253m","SE",146,1],
    ["7","José Antonio Cabrera",137,7,19,"137m","NE",52,1],
    ["7","General Félix Laborde",179,9,16,"178m","NW",325,1],
    ["15","",0,11,0,"0m","N",0]
]

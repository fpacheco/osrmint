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

-- Functions in osrmint

-- Not load this file from from psql 
\echo Use "CREATE EXTENSION osrmint" to load this file. \quit

---------------------------------------------------------------------
-- Core functions to access OSRM from postgresql
-- Author: Fernando Pacheco <fernando.pacheco@ingesur.com.uy>
-- Date: 2015-03-20
---------------------------------------------------------------------

CREATE OR REPLACE FUNCTION osrmint_route (
        IN base_url text, -- http://localhost:5000/viaroute
        IN datapoint_sql text, -- sql
        OUT id integer,
        OUT tdist float8,
        OUT ttime float8
    ) RETURNS RECORD
    AS 'MODULE_PATHNAME', 'osrmint_route'
    LANGUAGE c STABLE STRICT;


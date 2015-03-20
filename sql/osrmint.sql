-- Function in osrmint
CREATE FUNCTION osrmint_route (text osrm_path, text datapoint_sql) RETURNS int AS '$libdir/osrmint' LANGUAGE C;

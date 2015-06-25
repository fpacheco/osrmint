-- DROP TABLE route;
CREATE TABLE route
(
  id serial NOT NULL,
  xf float NOT NULL,
  yf float NOT NULL,
  xt float NOT NULL,
  yt float NOT NULL
);

CREATE TABLE viaroute
(
  id serial NOT NULL,
  x float NOT NULL,
  y float NOT NULL
);

-- En la ruteo
COPY (SELECT id, longitud, latitud  FROM rrsint.contenedor)
TO '/tmp/viaroute.csv'
WITH DELIMITER ';'
CSV HEADER;

-- En la ruteo
COPY (SELECT id, longitud, latitud  FROM rrsint.contenedor)
TO '/tmp/route.csv'
WITH DELIMITER ';'
CSV HEADER;


-- En esta base
COPY viaroute(id, x, y)
FROM '/tmp/viaroute.csv'
WITH DELIMITER ';'
CSV HEADER;

COPY route(id, xf, yf, xt, yt)
FROM '/tmp/route.csv'
WITH DELIMITER ';'
CSV HEADER;

-- DROP TABLE tinstruction;
CREATE TABLE tinstruction (
    id serial NOT NULL PRIMARY KEY,
    dd varchar(5) NOT NULL UNIQUE,
    instruction text NOT NULL
);

-- Instrucciones de OSRM en español
INSERT INTO tinstruction VALUES(1,'0','Instrucción desconocida en ');
INSERT INTO tinstruction VALUES(2,'1','Continue por ');
INSERT INTO tinstruction VALUES(3,'2','Doble levemente a la derecha en ');
INSERT INTO tinstruction VALUES(4,'3','Doble a la derecha en ');
INSERT INTO tinstruction VALUES(5,'4','Doble fuertemente a la derecha en ');
INSERT INTO tinstruction VALUES(6,'5','Gire en U en ');
INSERT INTO tinstruction VALUES(7,'6','Doble fuertemente a la izquierda en ');
INSERT INTO tinstruction VALUES(8,'7','Doble a la izquierda en ');
INSERT INTO tinstruction VALUES(9,'8','Doble levemente a la izquierda en ');
INSERT INTO tinstruction VALUES(10,'9','Se alcanzo punto de ruta ');
INSERT INTO tinstruction VALUES(11,'10','Inicio del viaje en ');
INSERT INTO tinstruction VALUES(12,'11-1','Entrar en ronda, salir en primer salida en ');
INSERT INTO tinstruction VALUES(13,'11-2','Entrar en ronda, salir en segunda salida en ');
INSERT INTO tinstruction VALUES(14,'11-3','Entrar en ronda, salir en tercer salida en ');
INSERT INTO tinstruction VALUES(15,'11-4','Entrar en ronda, salir en cuarta salida en ');
INSERT INTO tinstruction VALUES(16,'11-5','Entrar en ronda, salir en quinta salida en ');
INSERT INTO tinstruction VALUES(17,'11-6','Entrar en ronda, salir en sexta salida en ');
INSERT INTO tinstruction VALUES(18,'11-7','Entrar en ronda, salir en séptima salida en ');
INSERT INTO tinstruction VALUES(19,'11-8','Entrar en ronda, salir en octava salida en ');
INSERT INTO tinstruction VALUES(20,'11-9','Entrar en ronda, salir en novena salida en ');
INSERT INTO tinstruction VALUES(21,'11-x','Entrar en ronda, salir en ');
INSERT INTO tinstruction VALUES(22,'15','Fin del viaje en ');


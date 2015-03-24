#ifndef TYPES_H
#define TYPES_H

// Input structure
typedef struct datapoint_ {
  int id;       // Identificador unico
  double xf;    // Longitud from
  double yf;    // Latitud from
  double xt;    // Longitud to
  double yt;    // Latitud to
} datapoint_t;

// Output structure
typedef struct datadt_ {
  int id;       // Identificador unico
  double tdist; // Travel distance
  double ttime; // Travel time
} datadt_t;

// Node sequence structure (viaroute)
typedef struct dataviaroute_ {
  int seq;      // Orden de puntos
  int nodeid;   // Identificador unico del nodo
  double x;     // Longitud from
  double y;     // Latitud from
} dataviaroute_t;

// Viaroute JSON
typedef struct dataroutejson_ {
  char* json;      // string with json result from OSRM
} dataroutejson_t;

// Route sequence structure
typedef struct dataroutegeom_ {
  int seq;      // Orden de puntos
  double x;     // Longitud from
  double y;     // Latitud from
} dataroutegeom_t;

// Route instructions
typedef struct datarouteinst_ {
  int seq;      // Orden de puntos
  int dd;       // Driving direction (10 = salida, 9 = Punto inermedio de la ruta!!, 15 = llegamos!!)
  char* wname;  // Way name
  int tlen;     // Lenght in m
  int posc;     // Position????
  int ttime;    // Time in seconds
  char* tlenm;  // Lenght in string with units
  char* dir;    // Direction abreviation
  double azim;  // Azimut
} datarouteinst_t;

// Punto Longitud, Latitud
typedef struct lolapoint_ {
    double x;   // Longitude
    double y;   // Latitude
} lolapoint_t;

// Columnas de datos
typedef struct datapoint_columns {
    int id;
    int xf;
    int yf;
    int xt;
    int yt;
} datapoint_columns_t;

// Columnas de datos
typedef struct dataviaroute_columns {
    int seq;
    int nodeid;
    int x;
    int y;
} dataviaroute_columns_t;

#endif // TYPES_H

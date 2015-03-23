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

// Route sequence structure
typedef struct dataviaroute_ {
  int seq;
  int nodeid;   // Identificador unico del nodo
  double x;     // Longitud from
  double y;     // Latitud from
} dataviaroute_t;

#endif // TYPES_H

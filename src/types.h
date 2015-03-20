#ifndef TYPES_H
#define TYPES_H

// Input
typedef struct datapoint_ {
  int id;
  double xs;
  double ys;
  double xe;
  double ye;
} datapoint_t;

// Output
typedef struct datadt_ {
  int id;
  double tdist;
  double ttime;
} datadt_t;

#endif // TYPES_H

#ifndef OSRMINT_H
#define OSRMINT_H

#include "pg_config.h"
#include "postgres.h"
#include "utils/builtins.h"
#include "executor/spi.h"
#include "funcapi.h"
#include "catalog/pg_type.h"

#if PG_VERSION_NUM/100 > 902
#include "access/htup_details.h"
#endif

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

#include "types.h"

// From C++ wrapper (osrmint_wrapper.cpp)
extern int c_wrapper_route(datapoint_t *datapoints,char *baseURL,int ndatapoints,
    datadt_t **result,int *result_count,char **err_msg_out);

extern int c_wrapper_viaroute(dataviaroute_t *datapoints, char *baseURL, int ndatapoints, char** result);

// Definicion de funciones
// Distancia entre puntos
Datum osrmint_route( PG_FUNCTION_ARGS );

// Puntos de una ruta de OSRM para sequencia de puntos ordenados (seq, nodeid, x, y) devuelve JSON!!!
Datum osrmint_viaroute( PG_FUNCTION_ARGS );

#endif // OSRMINT_H

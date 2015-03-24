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

// From C++ wrapper
extern int c_wrapper_route(
    char *baseURL,
    datapoint_t *datapoints,
    int ndatapoints,
    datadt_t **result,
    int *result_count,
    char **err_msg_out
);

extern int c_wrapper_viaroute(
    char *baseURL,
    dataviaroute_t *datapoints,
    int ndatapoints,
    dataroutegeom_t **result,
    int *result_count,
    char **err_msg
);

// Definicion de funciones
// Distancia entre puntos
Datum osrmint_route( PG_FUNCTION_ARGS );

// Puntos de una ruta de OSRM para sequencia de puntos ordenados (seq, nodeid, x, y) devuelve JSON!!!
Datum osrmint_viaroute( PG_FUNCTION_ARGS );

#endif // OSRMINT_H

#ifndef OSRMINT_H
#define OSRMINT_H

// Tipos de datos de la biblioteca
#include "types.h"

#include "pg_config.h"
#include "postgres.h"
#include "executor/spi.h"
#include "funcapi.h"
#include "catalog/pg_type.h"
#include "utils/builtins.h"

#if PG_VERSION_NUM/100 > 902
#include "access/htup_details.h"
#endif

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

// Definicion de funciones
Datum osrmint_route( PG_FUNCTION_ARGS );

#undef DEBUG
#define DEBUG 1

#ifdef DEBUG
#include <stdio.h>
#define DBG(format, arg...)                     \
    elog(NOTICE, format , ## arg)
#else
#define DBG(format, arg...) do { ; } while (0)
#endif

#define TUPLIMIT 1000

// Columnas de datos
typedef struct datapoint_columns {
    int id;
    int xf;
    int yf;
    int xt;
    int yt;
} datapoint_columns_t;

#endif // OSRMINT_H

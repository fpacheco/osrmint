#ifndef OSRMINTVIAROUTE_H
#define OSRMINTVIAROUTE_H

#include "pg_config.h"
#include "postgres.h"
#include "executor/spi.h"
#include "funcapi.h"
#include "catalog/pg_type.h"
#include "utils/builtins.h"

#include "types.h"

int fetch_dataviaroute_columns( SPITupleTable *tuptable,
                                    dataviaroute_columns_t *dataviaroute_columns );

void fetch_dataviaroute( HeapTuple *tuple, TupleDesc *tupdesc,
                             dataviaroute_columns_t *columns, dataviaroute_t *data );

#endif // OSRMINTVIAROUTE_H

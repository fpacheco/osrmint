#ifndef OSRMINTROUTE_H
#define OSRMINTROUTE_H

//#include "pg_config.h"
#include "postgres.h"
#include "executor/spi.h"
#include "funcapi.h"
#include "catalog/pg_type.h"
#include "utils/builtins.h"

#include "types.h"

int fetch_datapoint_columns( SPITupleTable *tuptable,
                                    datapoint_columns_t *datapoint_columns );

void fetch_datapoint( HeapTuple *tuple, TupleDesc *tupdesc,
                             datapoint_columns_t *columns, datapoint_t *data );

int route(
    char *datapoint_sql,
    char *baseURL,
    datadt_t **result,
    int *result_count,
    char **err_msg_out);

#endif // OSRMINTROUTE_H

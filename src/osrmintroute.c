#include "osrmintutils.h"
#include "osrmintroute.h"

#include "pg_config.h"
#include "postgres.h"
#include "executor/spi.h"
#include "funcapi.h"
#include "catalog/pg_type.h"
#include "utils/builtins.h"

#if PG_VERSION_NUM/100 > 902
#include "access/htup_details.h"
#endif

#define TUPLIMIT 1000

/*******************************************************************************/
int fetch_datapoint_columns( SPITupleTable *tuptable,
                                    datapoint_columns_t *datapoint_columns ) {
    datapoint_columns->id        = SPI_fnumber( SPI_tuptable->tupdesc, "id" );
    datapoint_columns->xf        = SPI_fnumber( SPI_tuptable->tupdesc, "xf" );
    datapoint_columns->yf        = SPI_fnumber( SPI_tuptable->tupdesc, "yf" );
    datapoint_columns->xt        = SPI_fnumber( SPI_tuptable->tupdesc, "xt" );
    datapoint_columns->yt        = SPI_fnumber( SPI_tuptable->tupdesc, "yt" );

    if (    datapoint_columns->id       == SPI_ERROR_NOATTRIBUTE
            || datapoint_columns->xf        == SPI_ERROR_NOATTRIBUTE
            || datapoint_columns->yf        == SPI_ERROR_NOATTRIBUTE
            || datapoint_columns->xt        == SPI_ERROR_NOATTRIBUTE
            || datapoint_columns->yt        == SPI_ERROR_NOATTRIBUTE
       ) {
        elog( ERROR, "Error: points query must return columns "
              "'id', 'xf', 'yf', 'xt', 'yt'"
            );
        return -1;
    }

    if (    SPI_gettypeid( SPI_tuptable->tupdesc, datapoint_columns->id ) != INT4OID
            || SPI_gettypeid( SPI_tuptable->tupdesc, datapoint_columns->xf ) != FLOAT8OID
            || SPI_gettypeid( SPI_tuptable->tupdesc, datapoint_columns->yf ) != FLOAT8OID
            || SPI_gettypeid( SPI_tuptable->tupdesc, datapoint_columns->xt ) != FLOAT8OID
            || SPI_gettypeid( SPI_tuptable->tupdesc, datapoint_columns->yt ) != FLOAT8OID
       ) {
        elog( ERROR, "Error, points column types must be: int4 id"
              ", float8 xf, float8 yf, float8 xt, float8 yt"
            );
        return -1;
    }

    return 0;
}

void fetch_datapoint( HeapTuple *tuple, TupleDesc *tupdesc,
                             datapoint_columns_t *columns, datapoint_t *data ) {
    Datum binval;
    bool isnull;

    binval = SPI_getbinval( *tuple, *tupdesc, columns->id, &isnull );
    if ( isnull ) elog( ERROR, "container.id contains a null value" );
    data->id = DatumGetInt32( binval );

    binval = SPI_getbinval( *tuple, *tupdesc, columns->xf, &isnull );
    if ( isnull ) elog( ERROR, "datapoints.xf contains a null value" );
    data->xf = DatumGetFloat8( binval );

    binval = SPI_getbinval( *tuple, *tupdesc, columns->yf, &isnull );
    if ( isnull ) elog( ERROR, "datapoints.yf contains a null value" );
    data->yf = DatumGetFloat8( binval );

    binval = SPI_getbinval( *tuple, *tupdesc, columns->xt, &isnull );
    if ( isnull ) elog( ERROR, "datapoints.xt contains a null value" );
    data->xt = DatumGetFloat8( binval );

    binval = SPI_getbinval( *tuple, *tupdesc, columns->yt, &isnull );
    if ( isnull ) elog( ERROR, "datapoints.yt contains a null value" );
    data->yt = DatumGetFloat8( binval );
}

/****************************************************************************/
/*                           route                                          */
/****************************************************************************/
int route(
    char *datapoint_sql,
    char *baseURL,
    datadt_t **result,
    int *result_count,
    char **err_msg_out) {

    int SPIcode;
    SPIPlanPtr SPIplan;
    Portal SPIportal;
    bool moredata = TRUE;
    int ntuples;

    datapoint_t *datapoints = NULL;
    int datapoint_count = 0;
    datapoint_columns_t datapoint_columns = {
        .id = -1, .xf = -1, .yf = -1, .xt = -1, .yt = -1
    };

    char *err_msg=NULL;
    int ret = -1;

    DBG( "Enter route\n" );

    SPIcode = SPI_connect();

    if ( SPIcode  != SPI_OK_CONNECT ) {
        elog( ERROR, "route: couldn't open a connection to SPI" );
        return -1;
    }

    DBG( "Fetching datapoint tuples\n" );

    SPIplan = SPI_prepare( datapoint_sql, 0, NULL );

    if ( SPIplan  == NULL ) {
        elog( ERROR,
              "route: couldn't create query plan for datapoint via SPI" );
        return -1;
    }

    if ( ( SPIportal = SPI_cursor_open( NULL, SPIplan, NULL, NULL,
                                        true ) ) == NULL ) {
        elog( ERROR, "route: SPI_cursor_open('%s') returns NULL",
              datapoint_sql );
        return -1;
    }

    while ( moredata == TRUE ) {
        //DBG("calling SPI_cursor_fetch");
        SPI_cursor_fetch( SPIportal, TRUE, TUPLIMIT );

        if ( SPI_tuptable == NULL ) {
            elog( ERROR, "SPI_tuptable is NULL" );
            return finish( SPIcode, -1 );
        }

        if ( datapoint_columns.id == -1 ) {
            if ( fetch_datapoint_columns( SPI_tuptable, &datapoint_columns ) == -1 )
                return finish( SPIcode, ret );
        }

        ntuples = SPI_processed;

        datapoint_count += ntuples;

        if ( ntuples > 0 ) {
            int t;
            SPITupleTable *tuptable = SPI_tuptable;
            TupleDesc tupdesc = SPI_tuptable->tupdesc;

            if ( !datapoints )
                datapoints = palloc( datapoint_count * sizeof( datapoint_t ) ); //C
            else
                datapoints = repalloc( datapoints, datapoint_count * sizeof( datapoint_t ) ); //C

            if ( datapoints == NULL ) {
                elog( ERROR, "Out of memory" );
                return finish( SPIcode, ret );
            }

            for ( t = 0; t < ntuples; t++ ) {
                HeapTuple tuple = tuptable->vals[t];
                fetch_datapoint( &tuple, &tupdesc, &datapoint_columns,
                                 &datapoints[datapoint_count - ntuples + t] );
            }

            SPI_freetuptable( tuptable );

        } else {
            moredata = FALSE;
        }
    }

    SPI_cursor_close( SPIportal );

    /*********************************************************************
        We finally have loaded all the data via the SQL and have it in
        structs so call the C++ wrapper and solve the problem.
    **********************************************************************/

    DBG( "Calling route\n" );
    ret = c_wrapper_route(
        datapoints,
        baseURL,
        datapoint_count,
        result,
        result_count,
        &err_msg
    );

    /*
    DBG( "osrmint_route returned status: %i", ret );
    DBG( "result_count = %i", *result_count );

    DBG( "Message received from inside:" );
    DBG( "%s", err_msg );
    */

    if ( ret < 0 ) {
        ereport( ERROR, ( errcode( ERRCODE_E_R_E_CONTAINING_SQL_NOT_PERMITTED ),
                          errmsg( "Error finding route: %s:\n", err_msg ) ) );
    }

    pfree(datapoints);
    return finish( SPIcode, ret );
}


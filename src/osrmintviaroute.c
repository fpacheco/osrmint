#include "osrmintutils.h"
#include "osrmintviaroute.h"

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

/*
#undef DEBUG
#define DEBUG 1

#ifdef DEBUG
#include <stdio.h>
#define DBG(format, arg...)                     \
    elog(NOTICE, format , ## arg)
#else
#define DBG(format, arg...) do { ; } while (0)
#endif

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif
*/



/*******************************************************************************/
int fetch_dataviaroute_columns( SPITupleTable *tuptable,
                                    dataviaroute_columns_t *dataviaroute_columns ) {
    dataviaroute_columns->seq       = SPI_fnumber( SPI_tuptable->tupdesc, "seq" );
    dataviaroute_columns->nodeid    = SPI_fnumber( SPI_tuptable->tupdesc, "nodeid" );
    dataviaroute_columns->x         = SPI_fnumber( SPI_tuptable->tupdesc, "x" );
    dataviaroute_columns->y         = SPI_fnumber( SPI_tuptable->tupdesc, "y" );

    if (    dataviaroute_columns->seq       == SPI_ERROR_NOATTRIBUTE
            || dataviaroute_columns->nodeid        == SPI_ERROR_NOATTRIBUTE
            || dataviaroute_columns->x        == SPI_ERROR_NOATTRIBUTE
            || dataviaroute_columns->y        == SPI_ERROR_NOATTRIBUTE
       ) {
        elog( ERROR, "Error: points query must return columns "
              "'seq', 'nodeid', 'x', 'y'"
            );
        return -1;
    }

    if (    SPI_gettypeid( SPI_tuptable->tupdesc, dataviaroute_columns->seq ) != INT4OID
            || SPI_gettypeid( SPI_tuptable->tupdesc, dataviaroute_columns->nodeid ) != INT4OID
            || SPI_gettypeid( SPI_tuptable->tupdesc, dataviaroute_columns->x ) != FLOAT8OID
            || SPI_gettypeid( SPI_tuptable->tupdesc, dataviaroute_columns->y ) != FLOAT8OID
       ) {
        elog( ERROR, "Error, points column types must be: int4 id"
              ", int4 nodeid, float8 x, float8 y"
            );
        return -1;
    }

    return 0;
}

void fetch_dataviaroute( HeapTuple *tuple, TupleDesc *tupdesc,
                             dataviaroute_columns_t *columns, dataviaroute_t *data ) {
    Datum binval;
    bool isnull;

    binval = SPI_getbinval( *tuple, *tupdesc, columns->seq, &isnull );
    if ( isnull ) elog( ERROR, "container.id contains a null value" );
    data->seq = DatumGetInt32( binval );

    binval = SPI_getbinval( *tuple, *tupdesc, columns->nodeid, &isnull );
    if ( isnull ) elog( ERROR, "container.id contains a null value" );
    data->nodeid = DatumGetInt32( binval );

    binval = SPI_getbinval( *tuple, *tupdesc, columns->x, &isnull );
    if ( isnull ) elog( ERROR, "datapoints.x contains a null value" );
    data->x = DatumGetFloat8( binval );

    binval = SPI_getbinval( *tuple, *tupdesc, columns->y, &isnull );
    if ( isnull ) elog( ERROR, "datapoints.yf contains a null value" );
    data->y = DatumGetFloat8( binval );
}

/****************************************************************************/
/*                           viaroute                                       */
/****************************************************************************/
int viaroute(char *dataviaroute_sql, char *baseURL, char** result) {

    int SPIcode;
    SPIPlanPtr SPIplan;
    Portal SPIportal;
    bool moredata = TRUE;
    int ntuples;

    dataviaroute_t *dataviaroutes = NULL;
    int dataviaroute_count = 0;
    dataviaroute_columns_t dataviaroute_columns = {
        .seq = -1, .nodeid = -1, .x = -1, .y = -1
    };

    char *err_msg=NULL;
    int ret = -1;

    DBG( "Connecting\n" );

    SPIcode = SPI_connect();

    if ( SPIcode  != SPI_OK_CONNECT ) {
        elog( ERROR, "route: couldn't open a connection to SPI" );
        return -1;
    }

    DBG( "Fetching datapoint tuples\n" );

    SPIplan = SPI_prepare( dataviaroute_sql, 0, NULL );

    if ( SPIplan  == NULL ) {
        elog( ERROR,
              "route: couldn't create query plan for datapoint via SPI" );
        return -1;
    }

    if ( ( SPIportal = SPI_cursor_open( NULL, SPIplan, NULL, NULL,
                                        true ) ) == NULL ) {
        elog( ERROR, "route: SPI_cursor_open('%s') returns NULL",
              dataviaroute_sql );
        return finish( SPIcode, ret );
    }

    while ( moredata == TRUE ) {
        //DBG("calling SPI_cursor_fetch");
        SPI_cursor_fetch( SPIportal, TRUE, TUPLIMIT );

        if ( SPI_tuptable == NULL ) {
            elog( ERROR, "SPI_tuptable is NULL" );
            return finish( SPIcode, ret );
        }

        if ( dataviaroute_columns.seq == -1 ) {
            if ( fetch_dataviaroute_columns( SPI_tuptable, &dataviaroute_columns ) == -1 )
                return finish( SPIcode, ret );
        }

        ntuples = SPI_processed;

        dataviaroute_count += ntuples;

        if ( ntuples > 0 ) {
            int t;
            SPITupleTable *tuptable = SPI_tuptable;
            TupleDesc tupdesc = SPI_tuptable->tupdesc;

            if ( !dataviaroutes )
                dataviaroutes = palloc( dataviaroute_count * sizeof( dataviaroute_t ) ); //C
            else
                dataviaroutes = repalloc( dataviaroutes, dataviaroute_count * sizeof( dataviaroute_t ) ); //C

            if ( dataviaroutes == NULL ) {
                elog( ERROR, "Out of memory" );
                return finish( SPIcode, ret );
            }

            for ( t = 0; t < ntuples; t++ ) {
                HeapTuple tuple = tuptable->vals[t];
                fetch_dataviaroute( &tuple, &tupdesc, &dataviaroute_columns,
                                 &dataviaroutes[dataviaroute_count - ntuples + t] );
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

    DBG( "c_wrapper_viaroute for %i records\n", dataviaroute_count );

    ret = c_wrapper_viaroute(
        dataviaroutes,
        baseURL,
        dataviaroute_count,
        result
    );

    if ( ret<0 ) {
        ereport( ERROR, ( errcode( ERRCODE_E_R_E_CONTAINING_SQL_NOT_PERMITTED ),
                          errmsg( "Error finding route from OSRM. Check sql and URL\n") ) );
    }

    pfree(dataviaroutes);
    return finish( SPIcode, ret );
}

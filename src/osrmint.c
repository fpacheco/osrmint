/*
* Pure C file
*/
#include "osrmint.h"

#define TUPLIMIT 1000

// From C++ wrapper
extern int c_wrapper_route(
    char *osrm_data_path,
    datapoint_t *datapoints,
    int ndatapoints,
    datadt_t **result,
    int *result_count,
    char **err_msg_out
);

/* Util functions */
static int finish( int code, int ret ) {
    DBG( "In finish, trying to disconnect from spi %d", ret );
    code = SPI_finish();

    if ( code  != SPI_OK_FINISH ) {
        elog( ERROR, "couldn't disconnect from SPI" );
        return -1 ;
    }
    DBG( "In finish, disconnect from spi %d successfull", ret );

    return ret;
}

static char *text2char( text *in ) {
    char *out = ( char * )palloc( VARSIZE( in ) );

    memcpy( out, VARDATA( in ), VARSIZE( in ) - VARHDRSZ );
    out[VARSIZE( in ) - VARHDRSZ] = '\0';
    return out;
}
/* End util functions */


static int fetch_datapoint_columns( SPITupleTable *tuptable,
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

static void fetch_datapoint( HeapTuple *tuple, TupleDesc *tupdesc,
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

static int route(
    char *baseURL,
    char *datapoint_sql,
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
        baseURL,
        datapoints,
        datapoint_count,
        result=NULL,
        result_count,
        err_msg_out
    );

    DBG( "osrmint_route returned status: %i", ret );
    DBG( "result_count = %i", *result_count );

    DBG( "Message received from inside:" );
    DBG( "%s", err_msg );

    if ( ret < 0 ) {
        ereport( ERROR, ( errcode( ERRCODE_E_R_E_CONTAINING_SQL_NOT_PERMITTED ),
                          errmsg( "Error finding route: %s:\n", err_msg ) ) );
    }

    pfree(datapoints);
    return finish( SPIcode, ret );
}

/* by value */
PG_FUNCTION_INFO_V1(osrmint_route);
Datum osrmint_route(PG_FUNCTION_ARGS) {

    FuncCallContext     *funcctx;
    int                  call_cntr;
    int                  max_calls;
    TupleDesc            tuple_desc;
    datadt_t            *result;
    char                *err_msg = NULL;
    char                *pmsg;
    int                  ret;

    // stuff done only on the first call of the function
    if ( SRF_IS_FIRSTCALL() ) {
        MemoryContext   oldcontext;
        int result_count = 0;

        // create a function context for cross-call persistence
        funcctx = SRF_FIRSTCALL_INIT();

        // switch to memory context appropriate for multiple function calls
        oldcontext = MemoryContextSwitchTo( funcctx->multi_call_memory_ctx );

        ret = route(
                    text2char( PG_GETARG_TEXT_P( 0 ) ), // baseURL
                    text2char( PG_GETARG_TEXT_P( 1 ) ), // datapoint_sql
                    &result,
                    &result_count,
                    &err_msg
         );

        DBG( "Search routes returned status %i", ret );

        if (err_msg) {
            DBG( "err_msg: '%s'", err_msg );
            pmsg = pstrdup( err_msg );
            DBG( "after pstrdup" );
            free( err_msg );
            DBG( "after free" );
        } else {
            pmsg = "Unknown Error!";
        }
        DBG( "ret=%d", ret );
        if ( ret < 0 ) {
            if ( result ) {
                free( result );
            }
            ereport( ERROR, ( errcode( ERRCODE_E_R_E_CONTAINING_SQL_NOT_PERMITTED ),
                              errmsg( "%s", pmsg ) ) );
        }

        #ifdef OSRMINTDEBUG
        /*
        DBG( "   Result count: %i", result_count );

        if ( ret >= 0 ) {
            double total_time = 0.0;
            int i;

            for ( i = 0; i < result_count; i++ ) {
                total_time += result[i].deltatime;
        DBG("reult[%i] (seq,vid,nid,ntype,deltatime,cargo)=(%i,%i,%i,%i,%f,%f)",i,result[i].seq,result[i].vid,result[i].nid,result[i].ntype,result[i].deltatime,result[i].cargo);
            }

            DBG( "Total Travel Time: %f", total_time );
        }
        */
        #endif

        // total number of tuples to be returned
        funcctx->max_calls = result_count;
        funcctx->user_fctx = result;
        /* Build a tuple descriptor for our result type */
        if ( get_call_result_type( fcinfo, NULL, &tuple_desc ) != TYPEFUNC_COMPOSITE ) {
            ereport( ERROR,
                ( errcode( ERRCODE_FEATURE_NOT_SUPPORTED ),
                errmsg( "function returning record called in context "
                        "that cannot accept type record" )
                )
            );
        }
        funcctx->tuple_desc = BlessTupleDesc( tuple_desc );
        MemoryContextSwitchTo( oldcontext );
    }

    // stuff done on every call of the function
    funcctx = SRF_PERCALL_SETUP();

    call_cntr = funcctx->call_cntr;
    max_calls = funcctx->max_calls;
    tuple_desc = funcctx->tuple_desc;
    result = ( datadt_t * ) funcctx->user_fctx;

    if ( call_cntr < max_calls ) {
        // do when there is more left to send
        HeapTuple    tuple;
        Datum        result_data;
        Datum       *values;
        char        *nulls;

        values = palloc( 6 * sizeof( Datum ) );
        nulls = palloc( 6 * sizeof( bool ) );

        values[0] = Int32GetDatum( result[call_cntr].id );
        nulls[0] = false;
        values[1] = Float8GetDatum( result[call_cntr].tdist );
        nulls[1] = false;
        values[2] = Float8GetDatum( result[call_cntr].ttime );
        nulls[2] = false;

        tuple = heap_form_tuple( tuple_desc, values, nulls );

        // make the tuple into a datum
        result_data = HeapTupleGetDatum( tuple );
        // clean up (this is not really necessary)
        pfree( values );
        pfree( nulls );

        SRF_RETURN_NEXT( funcctx, result_data );
    } else {
        // do when there is no more left
        DBG( "Going to free path" );
        if ( result ) {
            free( result );
        }
        SRF_RETURN_DONE( funcctx );
    }
}

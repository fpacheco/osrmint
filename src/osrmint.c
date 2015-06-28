#include "osrmint.h"

// Otros includes
#include "osrmintutils.h"
#include "osrmintroute.h"
#include "osrmintviaroute.h"

#define TUPLIMIT 1000

/************************************************************************/
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

    text*                   baseURL;
    text*                   sql;

    // stuff done only on the first call of the function
    if ( SRF_IS_FIRSTCALL() ) {
        MemoryContext   oldcontext;
        int result_count = 0;

        // create a function context for cross-call persistence
        funcctx = SRF_FIRSTCALL_INIT();

        // switch to memory context appropriate for multiple function calls
        oldcontext = MemoryContextSwitchTo( funcctx->multi_call_memory_ctx );

        sql = PG_GETARG_TEXT_P(0);
        if (PG_NARGS()==1 || PG_ARGISNULL(2)) {
            baseURL = cstring2text("http://127.0.0.1:5000/viaroute");
        } else {
            baseURL = PG_GETARG_TEXT_P(1);
        }

        ret = route(
                    text2char( sql ), // datapoint_sql
                    text2char( baseURL ), // baseURL
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

        values = palloc( 3 * sizeof( Datum ) );
        nulls = palloc( 3 * sizeof( bool ) );

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

/************************************************************************/
PG_FUNCTION_INFO_V1(osrmint_viaroute);
Datum osrmint_viaroute(PG_FUNCTION_ARGS) {

    text*                   baseURL;
    text*                   sql;
    text*                   result;
    int                     ret;
    char*                   cjson;

    sql = PG_GETARG_TEXT_P(0);

    if (PG_NARGS()==2 && PG_ARGISNULL(2)) {
        baseURL = PG_GETARG_TEXT_P(1);
    } else {
        baseURL = cstring2text("http://127.0.0.1:5000/viaroute");
    }

    ret = viaroute(
                text2char( PG_GETARG_TEXT_P(0) ),       // dataviaroute_sql
                text2char( PG_GETARG_TEXT_P(1) ),       // baseURL
                &cjson
     );

    if( cjson==NULL ) {
        PG_RETURN_NULL();
    }

    result = cstring2text(cjson);
    free(cjson);
    PG_RETURN_POINTER(result);
}

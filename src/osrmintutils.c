#include "osrmintutils.h"
#include "executor/spi.h"

/* Util functions */
int finish( int code, int ret ) {
    DBG( "In finish, trying to disconnect from spi %d", ret );
    code = SPI_finish();

    if ( code  != SPI_OK_FINISH ) {
        elog( ERROR, "couldn't disconnect from SPI" );
        return -1 ;
    }
    DBG( "In finish, disconnect from spi %d successfull", ret );

    return ret;
}

char *text2char( text *in ) {
    char *out = ( char * )palloc( VARSIZE( in ) );

    memcpy( out, VARDATA( in ), VARSIZE( in ) - VARHDRSZ );
    out[VARSIZE( in ) - VARHDRSZ] = '\0';
    return out;
}
/* End util functions */

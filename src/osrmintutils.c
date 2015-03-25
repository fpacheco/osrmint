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

text* cstring2text(const char *cstring)
{
    text *output;
    size_t sz;

    /* Guard against null input */
    if( cstring==NULL )
        return NULL;

    sz = strlen(cstring);
    output = palloc(sz + VARHDRSZ);
    if ( ! output )
        return NULL;
    SET_VARSIZE(output, sz + VARHDRSZ);
    if ( sz )
        memcpy(VARDATA(output),cstring,sz);
    return output;
}

char* text2cstring(const text *textptr)
{
    size_t size = VARSIZE(textptr) - VARHDRSZ;
    char *str = malloc(size+1);
    memcpy(str, VARDATA(textptr), size);
    str[size]='\0';
    return str;
}


/* End util functions */

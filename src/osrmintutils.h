#ifndef OSRMINTUTILS_H
#define OSRMINTUTILS_H

#include "postgres.h"

#undef DEBUG
#define DEBUG 1

#ifdef DEBUG
#include <stdio.h>
#define DBG(format, arg...)                     \
    elog(NOTICE, format , ## arg)
#else
#define DBG(format, arg...) do { ; } while (0)
#endif

int finish( int code, int ret );
char *text2char( text *in );

#endif // OSRMINTUTILS_H

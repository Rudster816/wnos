/* $Id: putc.c 517 2011-06-13 10:03:13Z solar $ */

/* putc( int, FILE * )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#include <stdio.h>

#ifndef REGTEST

int putc( int c, struct _PDCLIB_file_t * stream )
{
    return fputc( c, stream );
}

#endif

#ifdef TEST
#include <_PDCLIB_test.h>

int main( void )
{
    /* Testing covered by ftell.c */
    return TEST_RESULTS;
}

#endif

/* $Id: fillbuffer.c 509 2010-12-30 22:46:02Z solar $ */

/* _PDCLIB_fillbuffer( struct _PDCLIB_file_t * stream )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

/* This is an example implementation of _PDCLIB_fillbuffer() fit for
   use with POSIX kernels.
*/

#include <stdio.h>

#ifndef REGTEST
#include <_PDCLIB_glue.h>

int _PDCLIB_fillbuffer( struct _PDCLIB_file_t * stream )
{
    return EOF;
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


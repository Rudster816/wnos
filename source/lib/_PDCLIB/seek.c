/* $Id: seek.c 516 2011-03-17 05:59:31Z solar $ */

/* int64_t _PDCLIB_seek( FILE *, int64_t, int )

   This file is part of the Public Domain C Library (PDCLib).
   Permission is granted to use, modify, and / or redistribute at will.
*/

#include <stdio.h>

#ifndef _PDCLIB_GLUE_H
#define _PDCLIB_GLUE_H
#include <_PDCLIB_glue.h>
#endif

_PDCLIB_int64_t _PDCLIB_seek( struct _PDCLIB_file_t * stream, _PDCLIB_int64_t offset, int whence )
{
	return -1;
}

#ifdef TEST
#include <_PDCLIB_test.h>

int main( void )
{
    /* Testing covered by ftell.c */
    return TEST_RESULTS;
}

#endif


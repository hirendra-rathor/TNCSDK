/*
 * output.c
 *
 * TNC SDK Output Functions
 *
 * Copyright 2004-2013 Juniper Networks, Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * o Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * o Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the  
 *   distribution.
 * o Neither the name of Juniper Networks nor the names of its
 *   contributors may be used to endorse or promote products 
 *   derived from this software without specific prior written 
 *   permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "output.h"
#include <stdio.h>
#include <stdarg.h>


extern unsigned g_nVerbose;
extern unsigned g_nAsciiOutput;


int outfmt( eOUT_LEVEL level, char *fmt, ... )
{
    va_list a;
    int rc = -1;


    va_start( a, fmt );
    if( level <= (int)g_nVerbose )
    {
        vprintf( fmt, a );
        fflush( stdout );
    }

    va_end( a );
    return rc;
}

int outmessage( eOUT_LEVEL level, unsigned char *p, unsigned nSize )
{
    size_t i, n;
    const size_t nBytes = 16;
    const size_t nSpace = 2;

    static char szHex[] = "0123456789ABCDEF";
    int nOffset = 0;
    char buf[4 * 16/*nBytes*/ + 2/*nSpace*/ + 1];
    char sz[256];


    if( nSize == 0 )
        return 0;

    if( g_nAsciiOutput )
        return outfmt( level, "%.*s\n\n", nSize, p );

    while( nSize != 0 )
    {
        for( i = 0; i < (sizeof(buf) / sizeof(buf[0]) - 1); i++ )
            buf[i] = ' ';

        buf[sizeof(buf) - 1] = 0;

        n = nSize < nBytes ? nSize : nBytes;
        for( i = 0; i < n; i++ )
        {
            buf[3*i] = szHex[p[i] >> 4];
            buf[3*i+1] = szHex[p[i] & 0x0F];
            buf[3*nBytes + nSpace + i] = p[i] >= 32 && p[i] < 128 ? p[i] : '.';
        }

        sprintf(sz, "   %08X: %s\n", nOffset, buf);
        outfmt( level, "%s", sz );

        nOffset += n;
        p += n;
        nSize -= n;
    }

    outfmt( level, "\n\n" );

    return 0;
}

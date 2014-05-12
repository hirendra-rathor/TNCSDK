/*
 * IMCIMVTester.c
 *
 * TNC SDK IMC/IMV Tester
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

#include "tncifimv.h"
#include "IMCIMVTester.h"
#include "IMCIMVTNCC.h"
#include "IMCIMVTNCS.h"
#include "msgqueue.h"
#include "output.h"

#ifdef WIN32
#define _WIN32_WINNT 0x0400
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ParseCommandLine(int argc, char * argv[]);

#if !defined(_MAX_PATH)
#define _MAX_PATH 256
#endif

#ifdef WIN32
static char g_pszImcPathName[_MAX_PATH] = {".\\SimpleIMC.dll"};
static char g_pszImvPathName[_MAX_PATH] = {".\\SimpleIMV.dll"};
#else
static char g_pszImcPathName[_MAX_PATH] = {"./SimpleIMC.dll"};
static char g_pszImvPathName[_MAX_PATH] = {"./SimpleIMV.dll"};
#endif

unsigned g_nAsciiOutput = 1;
unsigned g_nVerbose = 0;
TNC_ConnectionID g_nCID = 0;


int main(int argc, char * argv[])
{
    extern char *g_pszConnStates[];
    unsigned state, result;


#ifdef WIN32
    HRESULT hr;
    hr = CoInitializeEx( 0, COINIT_MULTITHREADED );
#endif

    outfmt( OUT_LEVEL_NORMAL, "TNC SDK IMC/IMV Tester v1.3 r1 \n\n");
    ParseCommandLine( argc, argv );
    do
    {
        result = LoadIMC( g_pszImcPathName );
        if (result != TNC_RESULT_SUCCESS) 
            break;

        result = LoadIMV( g_pszImvPathName );
        if (result != TNC_RESULT_SUCCESS) 
            break;

        outfmt( OUT_LEVEL_NORMAL, "IMC and IMV DLLs loaded successfully. Press Enter to continue\n\n" );
        getchar();

        result = InitializeIMC();
        if (result != TNC_RESULT_SUCCESS) 
            break;

        result = InitializeIMV();
        if (result != TNC_RESULT_SUCCESS) 
            break;

        outfmt( OUT_LEVEL_NORMAL, "Establishing new connection (CID: %d)\n", g_nCID );
        NotifyImcConnectionState( g_nCID, TNC_CONNECTION_STATE_CREATE );
        NotifyImvConnectionState( g_nCID, TNC_CONNECTION_STATE_CREATE );

        outfmt( OUT_LEVEL_NORMAL, "Beginning new handshake on connection %d\n", g_nCID );
        NotifyImcConnectionState( g_nCID, TNC_CONNECTION_STATE_HANDSHAKE );
        NotifyImvConnectionState( g_nCID, TNC_CONNECTION_STATE_HANDSHAKE );

        QueueClearMessages();
        ImcBeginHandshake( g_nCID );
        ImcBatchEnding( g_nCID );

        while( 0 == IsQueueEmpty() )
        {
            QueueSaveState();
            outfmt( OUT_LEVEL_NORMAL, "Deliver queued messages to IMVs\n" );
            DeliverImvMessages( g_nCID );
            ImvBatchEnding( g_nCID );

            if( IsQueueEmpty() )
                break;

            QueueSaveState();
            outfmt( OUT_LEVEL_NORMAL, "Deliver queued messages to IMCs\n" );
            DeliverImcMessages( g_nCID );
            ImcBatchEnding( g_nCID );
        }

        QueueClearMessages();
        outfmt( OUT_LEVEL_NORMAL, "No more messages to deliver. Get results from IMVs\n" );

        // 5. IMV solicit recommendations
        state = ImvGetRecommendation( g_nCID, &result );

        NotifyImcConnectionState( g_nCID, state );
        NotifyImvConnectionState( g_nCID, state );

        outfmt( OUT_LEVEL_NORMAL, "Handshake on connection %d completed with result `%s'\n", g_nCID, g_pszConnStates[ state ] );

        outfmt( OUT_LEVEL_NORMAL, "Deleting connection (CID: %d)\n", g_nCID );
        NotifyImcConnectionState( g_nCID, TNC_CONNECTION_STATE_DELETE );
        NotifyImvConnectionState( g_nCID, TNC_CONNECTION_STATE_DELETE );

        outfmt( OUT_LEVEL_NORMAL, "Handshake complete. Press Enter to unload IMC and IMV modules.\n" );
        getchar();

        TerminateIMC();
        TerminateIMV();

    }while( 0 );

    outfmt( OUT_LEVEL_NORMAL, "Test complete. Press Enter to exit.\n");
    getchar();

#ifdef WIN32
    CoUninitialize();
#endif

    return 0;
}

int PrintUsage(void)
{
    outfmt( OUT_LEVEL_NORMAL, 
        "ImcImvTester [-?] [-imc path] [-imv path] [-v] [-b] [-u username] [-p policy] [-l language]\n"
        "   -?\t\tPrint this message.\n"
        "   -imc path\tPath to the IMC DLL. (Default \"%s\")\n"
        "   -imv path\tPath to the IMV DLL. (Default \"%s\")\n"
        "   -v\t\tVerbose output\n"
        "   -b\t\tPrint IMC/IMV messages in binary format (default: ASCII)\n"
        "\n", g_pszImcPathName, g_pszImvPathName
        );
    exit( 0 );
}

#ifndef WIN32
#define strcmpi strcasecmp
#endif

int ParseCommandLine(int argc, char * argv[])
{
    static char *pOpts[] = {"?", "imc", "imv", "v", "b"};
    char *p;
    unsigned i;
    const unsigned n = sizeof( pOpts ) / sizeof( char* );


    while( argc-- )
    {
        p = argv[ argc ];

        if( p[0] == '-' )
        {
            for( i=0; i < n && strcmpi( p+1, pOpts[ i ] ); ++i );
            switch( i )
            {
            default:
                PrintUsage();

            case 1:
                if( argv[ argc + 1 ] )
                    strncpy( g_pszImcPathName, argv[ argc + 1 ], _MAX_PATH - 1 ); 
                else
                    PrintUsage();

                break;

            case 2:
                if( argv[ argc + 1 ] )
                    strncpy( g_pszImvPathName, argv[ argc + 1 ], _MAX_PATH - 1 ); 
                else
                    PrintUsage();

                break;

            case 3:
                g_nVerbose = 1;
                break;

            case 4:
                g_nAsciiOutput = 0;
                break;
            }
        }
    }

    return 0;
}


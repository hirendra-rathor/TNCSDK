/*
 * IMCIMVTNCSWin.c
 *
 * Windows-specific portions of IMCIMVTester TNCS Code
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

#include <windows.h>
#include "IMCIMVTNCS.h"


static HMODULE g_imvDLL = 0;

/* LoadEntrypoint
 *
 * Load a DLL entrypoint into a function table.
 *
 * Load the entrypoint named "entryName" from the DLL with
 * module "dll" and store the function pointer at the location
 * pointed to by "funcPtr". In case of error, store NULL as
 * the function pointer.
 */

void LoadEntryPoint(HMODULE dll, char *entryName, void **funcPtr);


/* LoadImvDLL
 *
 * Load an IMV from DLL with path dllPath, filling in function
 * table at funcTable.
 * Return a Windows error code in case of error, 0 for success
 */

int LoadImvDLL(const char *dllPath, IMVFuncs *funcTable) 
{
    g_imvDLL = LoadLibrary( dllPath );
    if (!g_imvDLL)
        return GetLastError();
    
    LoadEntryPoint(g_imvDLL, "TNC_IMV_Initialize", (void**)&funcTable->pfnInitialize );
    if( NULL == funcTable->pfnInitialize )
        return GetLastError();

    LoadEntryPoint(g_imvDLL, "TNC_IMV_SolicitRecommendation", (void**)&funcTable->pfnSolicitRecommendation );
    if( NULL == funcTable->pfnSolicitRecommendation )
        return GetLastError();

    LoadEntryPoint(g_imvDLL, "TNC_IMV_ProvideBindFunction", (void**)&funcTable->pfnProvideBind );
    LoadEntryPoint(g_imvDLL, "TNC_IMV_NotifyConnectionChange", (void**)&funcTable->pfnNotifyConnectionChange );
    LoadEntryPoint(g_imvDLL, "TNC_IMV_ReceiveMessage", (void**)&funcTable->pfnReceiveMessage );
    LoadEntryPoint(g_imvDLL, "TNC_IMV_ReceiveMessageSOH", (void**)&funcTable->pfnReceiveMessageSOH );
    LoadEntryPoint(g_imvDLL, "TNC_IMV_ReceiveMessageLong", (void**)&funcTable->pfnReceiveMessageLong );
    LoadEntryPoint(g_imvDLL, "TNC_IMV_Terminate", (void**)&funcTable->pfnTerminate );
    LoadEntryPoint(g_imvDLL, "TNC_IMV_BatchEnding", (void**)&funcTable->pfnBatchEnding );
    return 0;
}


void UnloadImvDLL(void)
{
    if( NULL != g_imvDLL )
        FreeLibrary( g_imvDLL );
}

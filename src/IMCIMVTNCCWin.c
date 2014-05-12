/*
 * IMCIMVTNCCWin.c
 *
 * Windows-specific portions of IMCIMVTester TNCC Code
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
#include "IMCIMVTNCC.h"


static HMODULE g_imcDLL = 0;


/* LoadEntrypoint
 *
 * Load a DLL entrypoint into a function table.
 *
 * Load the entrypoint named "entryName" from the DLL with
 * module "dll" and store the function pointer at the location
 * pointed to by "funcPtr". In case of error, store NULL as
 * the function pointer.
 */

void LoadEntryPoint(HMODULE dll, char *entryName, void **funcPtr) 
{
    *funcPtr = GetProcAddress( dll, entryName );
}


/* LoadIMC
 *
 * Load an IMC from DLL with path dllPath, filling in function
 * table at funcTable.
 * Return a Windows error code in case of error, 0 for success
 */

int LoadImcDLL(const char *dllPath, IMCFuncs *funcTable) 
{
    g_imcDLL = LoadLibrary( dllPath );
    if( !g_imcDLL )
        return GetLastError();
    
    LoadEntryPoint( g_imcDLL, "TNC_IMC_Initialize", (void**)&funcTable->pfnInitialize );
    if( NULL == funcTable->pfnInitialize )
        return GetLastError();

    LoadEntryPoint( g_imcDLL, "TNC_IMC_BeginHandshake", (void**)&funcTable->pfnBeginHandshake );
    if( NULL == funcTable->pfnBeginHandshake )
        return GetLastError();

    LoadEntryPoint( g_imcDLL, "TNC_IMC_NotifyConnectionChange", (void**)&funcTable->pfnNotifyConnChg );
    LoadEntryPoint( g_imcDLL, "TNC_IMC_ReceiveMessage", (void**)&funcTable->pfnReceiveMessage );
	LoadEntryPoint( g_imcDLL, "TNC_IMC_ReceiveMessageSOH", (void**)&funcTable->pfnReceiveMessageSOH );
	LoadEntryPoint( g_imcDLL, "TNC_IMC_ReceiveMessageLong", (void**)&funcTable->pfnReceiveMessageLong );
    LoadEntryPoint( g_imcDLL, "TNC_IMC_BatchEnding", (void**)&funcTable->pfnBatchEnding );
    LoadEntryPoint( g_imcDLL, "TNC_IMC_Terminate", (void**)&funcTable->pfnTerminate );
    LoadEntryPoint( g_imcDLL, "TNC_IMC_ProvideBindFunction", (void**)&funcTable->pfnProvideBind );
    return 0;
}

void UnloadImcDLL(void)
{
    if( NULL != g_imcDLL )
        FreeLibrary( g_imcDLL );
}

/*
 * SimpleIMC.c
 *
 * Simple Integrity Measurement Collector
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

#include <stdio.h>
#include <string.h>
#include "tncifimc.h"

/* This message type is for experimental purposes only. You MUST change
 * this message type for production use. Use your own vendor ID.
 */
#define TCG_EXPERIMENTAL_MESSAGE_TYPE ((TNC_VENDORID_TCG_NEW<<8) | 254)

static TNC_TNCC_SendMessagePointer sendMessage = NULL;

TNC_IMC_API TNC_Result TNC_IMC_Initialize(
/*in*/  TNC_IMCID imcID,
/*in*/  TNC_Version minVersion,
/*in*/  TNC_Version maxVersion,
/*out*/ TNC_Version *pOutActualVersion) {

    if ((minVersion > TNC_IFIMC_VERSION_1) || (maxVersion < TNC_IFIMC_VERSION_1))
        return TNC_RESULT_NO_COMMON_VERSION;

    *pOutActualVersion = TNC_IFIMC_VERSION_1;
    return TNC_RESULT_SUCCESS;
}

TNC_IMC_API TNC_Result TNC_IMC_ProvideBindFunction(
/*in*/  TNC_IMCID imcID,
/*in*/  TNC_TNCC_BindFunctionPointer bindFunction) {
    TNC_Result result;

    result = bindFunction(imcID, "TNC_TNCC_SendMessage", (void **) &sendMessage);
    if (result != TNC_RESULT_SUCCESS)
        result = TNC_RESULT_OTHER;
    return result;
}

TNC_IMC_API TNC_Result TNC_IMC_BeginHandshake(
/*in*/  TNC_IMCID imcID,
/*in*/  TNC_ConnectionID connectionID) {
    char *message;
#ifdef WIN32
    char *path = "c:\\OK";
#else
    char *path = "/OK";
#endif
    FILE *f;

    f = fopen(path, "r");
    if (f == NULL) {
        message = "Problem";
    } else {
        fclose(f);
        message = "OK";
    }

    if (sendMessage == NULL)
        return TNC_RESULT_OTHER;

    return sendMessage(imcID, connectionID,
        (TNC_BufferReference) message,
        (TNC_UInt32) strlen(message)+1, TCG_EXPERIMENTAL_MESSAGE_TYPE);
}

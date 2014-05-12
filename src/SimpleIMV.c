/*
 * SimpleIMV.c
 *
 * Simple Integrity Measurement Verifier
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
#include "tncifimv.h"

/* This message type is for experimental purposes only. You MUST change
 * this message type for production use. Use your own vendor ID.
 */
#define TCG_EXPERIMENTAL_MESSAGE_TYPE ((TNC_VENDORID_TCG_NEW<<8) | 254)

static TNC_TNCS_ProvideRecommendationPointer provideRec = NULL;
static TNC_IMV_Action_Recommendation lastRecommendation = TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION;
static TNC_IMV_Evaluation_Result lastEvaluation = TNC_IMV_EVALUATION_RESULT_DONT_KNOW;


TNC_IMV_API TNC_Result TNC_IMV_Initialize(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_Version minVersion,
/*in*/  TNC_Version maxVersion,
/*out*/ TNC_Version *pOutActualVersion) {

    if ((minVersion > TNC_IFIMV_VERSION_1) || (maxVersion < TNC_IFIMV_VERSION_1))
        return TNC_RESULT_NO_COMMON_VERSION;

    *pOutActualVersion = TNC_IFIMV_VERSION_1;
    return TNC_RESULT_SUCCESS;
}

TNC_IMV_API TNC_Result TNC_IMV_ProvideBindFunction(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_TNCS_BindFunctionPointer bindFunction) {
    TNC_Result result;
    TNC_TNCS_ReportMessageTypesPointer reportTypes;
    TNC_MessageType tcgType = TCG_EXPERIMENTAL_MESSAGE_TYPE;

    result = bindFunction(imvID, "TNC_TNCS_ReportMessageTypes", (void **) &reportTypes);
    if (result != TNC_RESULT_SUCCESS)
        return TNC_RESULT_OTHER;

    result = bindFunction(imvID, "TNC_TNCS_ProvideRecommendation", (void **) &provideRec);
    if (result != TNC_RESULT_SUCCESS)
        return TNC_RESULT_OTHER;

    result = reportTypes(imvID, &tcgType, 1);
    if (result != TNC_RESULT_SUCCESS)
        return TNC_RESULT_OTHER;

    return TNC_RESULT_SUCCESS;
}

TNC_IMV_API TNC_Result TNC_IMV_ReceiveMessage(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_BufferReference messageBuffer,
/*in*/  TNC_UInt32 messageLength,
/*in*/  TNC_MessageType messageType) {

    TNC_IMV_Action_Recommendation recommendation;
    TNC_IMV_Evaluation_Result evaluation;

    if (messageType != TCG_EXPERIMENTAL_MESSAGE_TYPE)
        return TNC_RESULT_OTHER;

    /* Make sure the message is a NUL-terminated string */
    if ((messageLength == 0) || (*(messageBuffer+messageLength-1) != '\0'))
        recommendation = TNC_IMV_ACTION_RECOMMENDATION_NO_ACCESS;
    else {
        if (!strcmp((char *) messageBuffer, "OK"))
            recommendation = TNC_IMV_ACTION_RECOMMENDATION_ALLOW;
        else
            recommendation = TNC_IMV_ACTION_RECOMMENDATION_NO_ACCESS;
    }
    if (recommendation == TNC_IMV_ACTION_RECOMMENDATION_ALLOW)
        evaluation = TNC_IMV_EVALUATION_RESULT_COMPLIANT;
    else
        evaluation = TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MAJOR;

    lastRecommendation = recommendation;
    lastEvaluation = evaluation;

    if (provideRec == NULL)
        return TNC_RESULT_OTHER;

    return provideRec(imvID, connectionID, recommendation, evaluation);
}

TNC_IMV_API TNC_Result TNC_IMV_SolicitRecommendation(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID) {

    /* This approach to implementing TNC_IMV_SolicitRecommendation won't work
     * if there can be more than one simultaneous handshake. To handle that case,
     * you could implement a thread-safe table indexed by connectionID.
     */

    if (provideRec == NULL)
        return TNC_RESULT_OTHER;

    return provideRec(imvID, connectionID, lastRecommendation, lastEvaluation);
}

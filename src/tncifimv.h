/* tncifimv.h
 *
 * Trusted Network Connect IF-IMV API version 1.30
 * Microsoft Windows DLL Platform Binding C Header
 * February 25, 2013
 *
 * Copyright(c) 2005-2013, Trusted Computing Group, Inc. All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the  
 *   distribution.
 * - Neither the name of the Trusted Computing Group nor the names of
 *   its contributors may be used to endorse or promote products 
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
 *
 * Contact the Trusted Computing Group at 
 * admin@trustedcomputinggroup.org for information on specification 
 * licensing through membership agreements.
 *
 * Any marks and brands contained herein are the property of their 
 * respective owners.
 */

#ifndef _TNCIFIMV_H
#define _TNCIFIMV_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#ifdef TNC_IMV_EXPORTS
#define TNC_IMV_API __declspec(dllexport)
#else
#define TNC_IMV_API __declspec(dllimport)
#endif
#else
#define TNC_IMV_API
#endif

/* Basic Types */

typedef unsigned long TNC_UInt32;
typedef unsigned char *TNC_BufferReference;

/* Derived Types */

typedef TNC_UInt32 TNC_IMVID;
typedef TNC_UInt32 TNC_ConnectionID;
typedef TNC_UInt32 TNC_ConnectionState;
typedef TNC_UInt32 TNC_RetryReason;
typedef TNC_UInt32 TNC_IMV_Action_Recommendation;
typedef TNC_UInt32 TNC_IMV_Evaluation_Result;
typedef TNC_UInt32 TNC_MessageType;
typedef TNC_MessageType *TNC_MessageTypeList;
typedef TNC_UInt32 TNC_VendorID;
typedef TNC_VendorID *TNC_VendorIDList;
typedef TNC_UInt32 TNC_MessageSubtype;
typedef TNC_MessageSubtype *TNC_MessageSubtypeList;
typedef TNC_UInt32 TNC_Version;
typedef TNC_UInt32 TNC_Result;
typedef TNC_UInt32 TNC_AttributeID;

/* Function pointers */

typedef TNC_Result (*TNC_IMV_InitializePointer)(
    TNC_IMVID imvID,
    TNC_Version minVersion,
    TNC_Version maxVersion,
    TNC_Version *pOutActualVersion);
typedef TNC_Result (*TNC_IMV_NotifyConnectionChangePointer)(
    TNC_IMVID imvID,
    TNC_ConnectionID connectionID,
    TNC_ConnectionState newState);
typedef TNC_Result (*TNC_IMV_ReceiveMessagePointer)(
    TNC_IMVID imvID,
    TNC_ConnectionID connectionID,
    TNC_BufferReference message,
    TNC_UInt32 messageLength,
    TNC_MessageType messageType);
typedef TNC_Result (*TNC_IMV_ReceiveMessageSOHPointer)(
    TNC_IMVID imvID,
    TNC_ConnectionID connectionID,
    TNC_BufferReference sohReportEntry,
    TNC_UInt32 sohRELength,
    TNC_MessageType systemHealthID);
typedef TNC_Result (*TNC_IMV_ReceiveMessageLongPointer)(
    TNC_IMVID imvID,
    TNC_ConnectionID connectionID,
    TNC_UInt32 messageFlags,
    TNC_BufferReference message,
    TNC_UInt32 messageLength,
    TNC_VendorID messageVendorID,
    TNC_MessageSubtype messageSubtype,
    TNC_UInt32 sourceIMCID,
    TNC_UInt32 destinationIMVID);
typedef TNC_Result (*TNC_IMV_SolicitRecommendationPointer)(
    TNC_IMVID imvID,
    TNC_ConnectionID connectionID);
typedef TNC_Result (*TNC_IMV_BatchEndingPointer)(
    TNC_IMVID imvID,
    TNC_ConnectionID connectionID);
typedef TNC_Result (*TNC_IMV_TerminatePointer)(
    TNC_IMVID imvID);
typedef TNC_Result (*TNC_TNCS_ReportMessageTypesPointer)(
    TNC_IMVID imvID,
    TNC_MessageTypeList supportedTypes,
    TNC_UInt32 typeCount);
typedef TNC_Result (*TNC_TNCS_ReportMessageTypesLongPointer)(
    TNC_IMVID imvID,
    TNC_VendorIDList supportedVendorIDs,
    TNC_MessageSubtypeList supportedSubtypes,
    TNC_UInt32 typeCount);
typedef TNC_Result (*TNC_TNCS_SendMessagePointer)(
    TNC_IMVID imvID,
    TNC_ConnectionID connectionID,
    TNC_BufferReference message,
    TNC_UInt32 messageLength,
    TNC_MessageType messageType);
typedef TNC_Result (*TNC_TNCS_SendMessageSOHPointer)(
    TNC_IMVID imvID,
    TNC_ConnectionID connectionID,
    TNC_BufferReference sohrReportEntry,
    TNC_UInt32 sohrRELength);
typedef TNC_Result (*TNC_TNCS_SendMessageLongPointer)(
    TNC_IMVID imvID,
    TNC_ConnectionID connectionID,
    TNC_UInt32 messageFlags,
    TNC_BufferReference message,
    TNC_UInt32 messageLength,
    TNC_VendorID messageVendorID,
    TNC_MessageSubtype messageSubtype,
    TNC_UInt32 destinationIMCID);
typedef TNC_Result (*TNC_TNCS_RequestHandshakeRetryPointer)(
    TNC_IMVID imvID,
    TNC_ConnectionID connectionID,
    TNC_RetryReason reason);
typedef TNC_Result (*TNC_TNCS_ProvideRecommendationPointer)(
    TNC_IMVID imvID,
    TNC_ConnectionID connectionID,
    TNC_IMV_Action_Recommendation recommendation,
    TNC_IMV_Evaluation_Result evaluation);
typedef TNC_Result (*TNC_TNCS_GetAttributePointer)(
    TNC_IMVID imvID,
TNC_ConnectionID connectionID,
TNC_AttributeID attributeID,
    TNC_UInt32 bufferLength,
    TNC_BufferReference buffer,
    TNC_UInt32 *pOutValueLength);
typedef TNC_Result (*TNC_TNCS_SetAttributePointer)(
    TNC_IMVID imvID,
    TNC_ConnectionID connectionID,
TNC_AttributeID attributeID,
    TNC_UInt32 bufferLength,
    TNC_BufferReference buffer);
typedef TNC_Result (*TNC_TNCS_ReserveAdditionalIMVIDPointer)(
    TNC_IMVID imvID,
    TNC_UInt32 *pOutIMVID);
typedef TNC_Result (*TNC_TNCS_BindFunctionPointer)(
    TNC_IMVID imvID,
    char *functionName,
    void **pOutfunctionPointer);
typedef TNC_Result (*TNC_IMV_ProvideBindFunctionPointer)(
    TNC_IMVID imvID,
    TNC_TNCS_BindFunctionPointer bindFunction);

/* Result Codes */

#define TNC_RESULT_SUCCESS 0
#define TNC_RESULT_NOT_INITIALIZED 1
#define TNC_RESULT_ALREADY_INITIALIZED 2
#define TNC_RESULT_NO_COMMON_VERSION 3
#define TNC_RESULT_CANT_RETRY 4
#define TNC_RESULT_WONT_RETRY 5
#define TNC_RESULT_INVALID_PARAMETER 6
/* reserved for TNC_RESULT_CANT_RESPOND: 7 */
#define TNC_RESULT_ILLEGAL_OPERATION 8
#define TNC_RESULT_OTHER 9
#define TNC_RESULT_FATAL 10
#define TNC_RESULT_EXCEEDED_MAX_ROUND_TRIPS 0x00559700
#define TNC_RESULT_EXCEEDED_MAX_MESSAGE_SIZE 0x00559701
#define TNC_RESULT_NO_LONG_MESSAGE_TYPES 0x00559702
#define TNC_RESULT_NO_SOH_SUPPORT 0x00559703



/* Version Numbers */

#define TNC_IFIMV_VERSION_1 1

/* Network Connection ID Values */

#define TNC_CONNECTIONID_ANY 0xFFFFFFFF

/* Network Connection State Values */

#define TNC_CONNECTION_STATE_CREATE 0
#define TNC_CONNECTION_STATE_HANDSHAKE 1
#define TNC_CONNECTION_STATE_ACCESS_ALLOWED 2
#define TNC_CONNECTION_STATE_ACCESS_ISOLATED 3
#define TNC_CONNECTION_STATE_ACCESS_NONE 4
#define TNC_CONNECTION_STATE_DELETE 5

/* Handshake Retry Reason Values */

/* reserved for TNC_RETRY_REASON_IMC_REMEDIATION_COMPLETE: 0 */
/* reserved for TNC_RETRY_REASON_IMC_SERIOUS_EVENT: 1 */
/* reserved for TNC_RETRY_REASON_IMC_INFORMATIONAL_EVENT: 2 */
/* reserved for TNC_RETRY_REASON_IMC_PERIODIC: 3 */
#define TNC_RETRY_REASON_IMV_IMPORTANT_POLICY_CHANGE 4
#define TNC_RETRY_REASON_IMV_MINOR_POLICY_CHANGE 5
#define TNC_RETRY_REASON_IMV_SERIOUS_EVENT 6
#define TNC_RETRY_REASON_IMV_MINOR_EVENT 7
#define TNC_RETRY_REASON_IMV_PERIODIC 8

/* IMV Action Recommendation Values */

#define TNC_IMV_ACTION_RECOMMENDATION_ALLOW 0
#define TNC_IMV_ACTION_RECOMMENDATION_NO_ACCESS 1
#define TNC_IMV_ACTION_RECOMMENDATION_ISOLATE 2
#define TNC_IMV_ACTION_RECOMMENDATION_NO_RECOMMENDATION 3

/* IMV Evaluation Result Values */

#define TNC_IMV_EVALUATION_RESULT_COMPLIANT 0
#define TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MINOR 1
#define TNC_IMV_EVALUATION_RESULT_NONCOMPLIANT_MAJOR 2
#define TNC_IMV_EVALUATION_RESULT_ERROR 3
#define TNC_IMV_EVALUATION_RESULT_DONT_KNOW 4

/* IMC/IMV ID Values */

#define TNC_IMVID_ANY ((TNC_UInt32) 0xffff)
#define TNC_IMCID_ANY ((TNC_UInt32) 0xffff)

/* Vendor ID Values */

#define TNC_VENDORID_TCG 0
#define TNC_VENDORID_TCG_NEW 0x005597
#define TNC_VENDORID_ANY ((TNC_VendorID) 0xffffff)

/* Message Subtype Values */

#define TNC_SUBTYPE_ANY ((TNC_MessageSubtype) 0xff)

/* Message Flags Values */

#define TNC_MESSAGE_FLAGS_EXCLUSIVE ((TNC_UInt32) 0x80000000)

/* Message Attribute ID Values */

#define TNC_ATTRIBUTEID_PREFERRED_LANGUAGE ((TNC_AttributeID) 0x00000001)
#define TNC_ATTRIBUTEID_REASON_STRING ((TNC_AttributeID) 0x00000002)
#define TNC_ATTRIBUTEID_REASON_LANGUAGE ((TNC_AttributeID) 0x00000003)
#define TNC_ATTRIBUTEID_MAX_ROUND_TRIPS ((TNC_AttributeID) 0x00559700)
#define TNC_ATTRIBUTEID_MAX_MESSAGE_SIZE ((TNC_AttributeID) 0x00559701)
#define TNC_ATTRIBUTEID_DHPN ((TNC_AttributeID) 0x00559702)
#define TNC_ATTRIBUTEID_HAS_LONG_TYPES  ((TNC_AttributeID) 0x00559703)
#define TNC_ATTRIBUTEID_HAS_EXCLUSIVE ((TNC_AttributeID) 0x00559704)
#define TNC_ATTRIBUTEID_HAS_SOH ((TNC_AttributeID) 0x00559705)
#define TNC_ATTRIBUTEID_SOH ((TNC_AttributeID) 0x00559706)
#define TNC_ATTRIBUTEID_SSOH ((TNC_AttributeID) 0x00559707)
#define TNC_ATTRIBUTEID_IFTNCCS_PROTOCOL ((TNC_AttributeID) 0x0055970A)
#define TNC_ATTRIBUTEID_IFTNCCS_VERSION ((TNC_AttributeID) 0x0055970B)
#define TNC_ATTRIBUTEID_IFT_PROTOCOL ((TNC_AttributeID) 0x0055970C)
#define TNC_ATTRIBUTEID_IFT_VERSION ((TNC_AttributeID) 0x0055970D)
#define TNC_ATTRIBUTEID_TLS_UNIQUE ((TNC_AttributeID) 0x0055970E)
#define TNC_ATTRIBUTEID_PRIMARY_IMV_ID ((TNC_AttributeID) 0x00559710)


/* IMV Functions */

TNC_IMV_API TNC_Result TNC_IMV_Initialize(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_Version minVersion,
/*in*/  TNC_Version maxVersion,
/*in*/  TNC_Version *pOutActualVersion);

TNC_IMV_API TNC_Result TNC_IMV_NotifyConnectionChange(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_ConnectionState newState);

TNC_IMV_API TNC_Result TNC_IMV_ReceiveMessage(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_BufferReference messageBuffer,
/*in*/  TNC_UInt32 messageLength,
/*in*/  TNC_MessageType messageType);

TNC_IMV_API TNC_Result TNC_IMV_ReceiveMessageSOH(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_BufferReference sohReportEntry,
/*in*/  TNC_UInt32 sohRELength,
/*in*/  TNC_MessageType systemHealthID);

TNC_IMV_API TNC_Result TNC_IMV_ReceiveMessageLong(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_UInt32 messageFlags,
/*in*/  TNC_BufferReference message,
/*in*/  TNC_UInt32 messageLength,
/*in*/  TNC_VendorID messageVendorID,
/*in*/  TNC_MessageSubtype messageSubtype,
/*in*/  TNC_UInt32 sourceIMCID,
/*in*/  TNC_UInt32 destinationIMVID);

TNC_IMV_API TNC_Result TNC_IMV_SolicitRecommendation(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID);

TNC_IMV_API TNC_Result TNC_IMV_BatchEnding(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID);

TNC_IMV_API TNC_Result TNC_IMV_Terminate(
/*in*/  TNC_IMVID imvID);

TNC_IMV_API TNC_Result TNC_IMV_ProvideBindFunction(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_TNCS_BindFunctionPointer bindFunction);

/* TNC Server Functions */

TNC_Result TNC_TNCS_ReportMessageTypes(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_MessageTypeList supportedTypes,
/*in*/  TNC_UInt32 typeCount);

TNC_Result TNC_TNCS_ReportMessageTypesLong(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_VendorIDList supportedVendorIDs,
/*in*/  TNC_MessageSubtypeList supportedSubtypes,
/*in*/  TNC_UInt32 typeCount);

TNC_Result TNC_TNCS_SendMessage(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_BufferReference message,
/*in*/  TNC_UInt32 messageLength,
/*in*/  TNC_MessageType messageType);

TNC_Result TNC_TNCS_SendMessageSOH(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_BufferReference sohrReportEntry,
/*in*/  TNC_UInt32 sohrRELength);

TNC_Result TNC_TNCS_SendMessageLong(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_UInt32 messageFlags,
/*in*/  TNC_BufferReference message,
/*in*/  TNC_UInt32 messageLength,
/*in*/  TNC_VendorID messageVendorID,
/*in*/  TNC_MessageSubtype messageSubtype,
/*in*/  TNC_UInt32 destinationIMCID);

TNC_Result TNC_TNCS_RequestHandshakeRetry(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_RetryReason reason);

TNC_Result TNC_TNCS_ProvideRecommendation(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_IMV_Action_Recommendation recommendation,
/*in*/  TNC_IMV_Evaluation_Result evaluation);

TNC_Result TNC_TNCS_GetAttribute(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_AttributeID attributeID,
/*in*/  TNC_UInt32 bufferLength,
/*out*/ TNC_BufferReference buffer,
/*out*/ TNC_UInt32 *pOutValueLength);

TNC_Result TNC_TNCS_ReserveAdditionalIMVID(
/*in*/  TNC_IMVID imvID,
/*out*/ TNC_UInt32 *pOutIMVID);

TNC_Result TNC_TNCS_SetAttribute(
/*in*/  TNC_IMVID imvID,
/*in*/ TNC_ConnectionID connectionID,
/*in*/ TNC_AttributeID attributeID,
/*in*/ TNC_UInt32 bufferLength,
/*in*/ TNC_BufferReference buffer);
TNC_Result TNC_TNCS_BindFunction(
/*in*/  TNC_IMVID imvID,
/*in*/  char *functionName,
/*in*/  void **pOutfunctionPointer);

#ifdef __cplusplus
}
#endif

#endif
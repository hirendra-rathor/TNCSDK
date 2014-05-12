/*
 * IMCIMVTNCS.c
 *
 * Platform-independent portions of IMCIMVTester TNCS Code
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

#include "IMCIMVTester.h"
#include "IMCIMVTNCS.h"
#include "msgqueue.h"
#include "output.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* TNCS will assign this IMV ID after loading the IMV */
#define IMV_ID  0

/* List of functions implemented by IMV */
static IMVFuncs imvFuncs;

/* List of message types supported by IMV in TNC_TNCS_ReportMessageTypes */
static TNC_MessageTypeList g_pImvMessageTypes = NULL;
static TNC_UInt32 g_nImvMessageTypesCount = 0;

/* List of message types supported by IMV in TNC_TNCS_ReportMessageTypesLong */
static TNC_MessageSubtypeList g_pImvMessageLongSubtypes = NULL;
static TNC_VendorIDList g_pImvVendorIDs = NULL;
static TNC_UInt32 g_nImvMessageLongSubtypesCount = 0;

/* Related to evaluation of collected data */
static TNC_IMV_Evaluation_Result g_nEvaluation = 0;
static TNC_IMV_Action_Recommendation g_nRecommendation = 0;
static unsigned g_bRecommendationProvided = 0;

/* Forward declarations */
int LoadImvDLL(const char *dllPath, IMVFuncs *funcTable);
void UnloadImvDLL(void);
int IsMessageTypeSupported( TNC_MessageType type, TNC_MessageTypeList list, TNC_UInt32 count );
int IsMessageLongTypeSupported( TNC_MessageSubtype subtype, 
							    TNC_VendorID vendorID,
								TNC_MessageSubtypeList listOfSubTypes,
							    TNC_VendorIDList listOfVendorIDs, 
								TNC_UInt32 count );

int LoadIMV(const char *dllPath)
{
    int err;

	outfmt( OUT_LEVEL_NORMAL, "Loading IMV DLL: \"%s\"...", dllPath );
    err = LoadImvDLL( dllPath, &imvFuncs );
    if (err) 
    {
        outfmt( OUT_LEVEL_NORMAL, " Error %d.\n", err);
        return TNC_RESULT_OTHER;
    }

    outfmt( OUT_LEVEL_NORMAL, " Ok\n" );
    return TNC_RESULT_SUCCESS;
}

int InitializeIMV(void)
{
    TNC_Result result;
    TNC_Version actualVersion;

    outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_Initialize\n" );
    result = (imvFuncs.pfnInitialize)(IMV_ID, TNC_IFIMV_VERSION_1, TNC_IFIMV_VERSION_1, &actualVersion);
    outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_Initialize result = %d.\n", result);
    if (result != TNC_RESULT_SUCCESS) 
        return TNC_RESULT_OTHER;

    if (imvFuncs.pfnProvideBind != NULL) 
    {
        outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_ProvideBindFunction\n" );
        result = (imvFuncs.pfnProvideBind)(IMV_ID, &TNC_TNCS_BindFunction);
        outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_ProvideBindFunction result = %d.\n", result);
        if (result != TNC_RESULT_SUCCESS) 
            return TNC_RESULT_OTHER;
    }

    outfmt( OUT_LEVEL_NORMAL, "IMV initialized successfully\n\n" );
    return result;
}

void TerminateIMV(void)
{
    TNC_Result result;

    if( NULL != imvFuncs.pfnTerminate )
    {
        outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_Terminate (IMV %d)\n", IMV_ID );
        result = imvFuncs.pfnTerminate( IMV_ID );
        outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_Terminate result: %d\n", result );
    }

    free( g_pImvMessageTypes );
    g_pImvMessageTypes = NULL;
    g_nImvMessageTypesCount = 0;

	free( g_pImvMessageLongSubtypes );
	free( g_pImvVendorIDs );
	g_pImvMessageLongSubtypes = NULL;
	g_pImvVendorIDs = NULL;
	g_nImvMessageLongSubtypesCount = 0;

    UnloadImvDLL();
}

unsigned DeliverImvMessages( TNC_ConnectionID cid )
{
	/* TNCS may receive messages belonging to different categories. Either of
	   these pointers will refer to the current message depending on its category */
	unsigned messageCategory = MESSAGE_CATEGORY_UNKNOWN;
	MESSAGE_BASIC  * basicMessage = NULL;
	MESSAGE_SOH    * sohMessage = NULL;
	MESSAGE_LONG   * longTypeMessage = NULL;
	TNC_MessageType sohType = 0;
	TNC_MessageType longMessageType = 0;
    TNC_Result rc;
    unsigned i;

	/* Deliver each message to the IMV */
	for (i=0; i < QueueGetMessageCount(); ++i)
	{
		/* Depending on the message category, it needs to be delivered differently */
		messageCategory = QueueGetMessageCategory(i);

		if (messageCategory == MESSAGE_CATEGORY_BASIC) 
		{
			/* Now that we know the message type, retrieve the message */
			QueueGetMessage(i, &basicMessage);

			outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_ReceiveMessage (type: %#x, length: %d)\n", 
				basicMessage->messageType, basicMessage->messageLength );

			if( imvFuncs.pfnReceiveMessage )
			{
				if( IsMessageTypeSupported( basicMessage->messageType, g_pImvMessageTypes, g_nImvMessageTypesCount ) )
				{
					rc = imvFuncs.pfnReceiveMessage( IMV_ID, cid, basicMessage->message, 
						basicMessage->messageLength, basicMessage->messageType );

					outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_ReceiveMessage result: %d\n", rc );
				}
				else
				{
					outfmt( OUT_LEVEL_NORMAL, "> Message type not registered; message not delivered!\n" );
				}
			}
			else
			{
				outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_ReceiveMessage TYPE NOT SUPPORTED!\n" );
			}
		}
		else if (messageCategory == MESSAGE_CATEGORY_SOH) 
		{
			/* Now that we know the message type, retrieve the message */
			QueueGetMessageSOH(i, &sohMessage);

			/* This is the preferred way of delivery */
			if (imvFuncs.pfnReceiveMessageSOH) 
			{
				/* The received buffer is complete SOHReportEntry. TNCS will parse
				   it into individual SOHRReportEntry buffers and deliver each buffer
				   to the IMV if IMV is supposed to receive it. Since logic for 
				   parsing SOH message is somewhat involved, it is not implemented.

				   Deliver the parsed SOHRReportEntries using imvFuncs.pfnReceiveMessageSOH.
				*/
				outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_ReceiveMessageSOH (length: %d)\n", 
					sohMessage->sohRELength);
				outfmt( OUT_LEVEL_NORMAL, "> Dispatching SOH messages to IMV **NOT IMPLEMENTED**\n");
			} 
			else if (imvFuncs.pfnReceiveMessage)
			{
				/* IMV didn't implement TNC_IMV_ReceiveMessageSOH function but 
				   TNCS can still delive the message using the pfnReceiveMessage. 
				   'sohType' is extracted from the message. */

				outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_ReceiveMessage (type: %#x, length: %d)\n", 
					sohType, sohMessage->sohRELength);

				if( IsMessageTypeSupported( sohType, g_pImvMessageTypes, g_nImvMessageTypesCount ) )
				{
					rc = imvFuncs.pfnReceiveMessage( IMV_ID, cid, sohMessage->sohReportEntry, 
						sohMessage->sohRELength, sohType );
					outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_ReceiveMessage result: %d\n", rc );
				}
				else
				{
					outfmt( OUT_LEVEL_NORMAL, "> Message type not registered; message not delivered!\n" );
				}
			}
			else 
			{
				outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_ReceiveMessage and "
						"TNC_IMV_ReceiveMessageSOH NOT SUPPORTED!\n" );
			}
		} 
		else if (messageCategory == MESSAGE_CATEGORY_LONG ) 
		{
			/* Now that we know the message type, retrieve the message */
			QueueGetMessageLong(i, &longTypeMessage);

			/* This is the preferred way of delivery */
			if (imvFuncs.pfnReceiveMessageLong) 
			{
				outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_ReceiveMessageLong (vendorID: %#x, subtype: %#x, length: %d)\n", 
					longTypeMessage->messageVendorID, longTypeMessage->messageSubtype, longTypeMessage->messageLength );

				if( (longTypeMessage->messageFlags & TNC_MESSAGE_FLAGS_EXCLUSIVE) == TNC_MESSAGE_FLAGS_EXCLUSIVE )
				{
					if(longTypeMessage->imvID == IMV_ID)
					{
						rc = imvFuncs.pfnReceiveMessageLong( IMV_ID, cid, longTypeMessage->messageFlags, 
							longTypeMessage->message, longTypeMessage->messageLength, 
							longTypeMessage->messageVendorID, longTypeMessage->messageSubtype, 
							longTypeMessage->imcID, longTypeMessage->imvID);

						outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_ReceiveMessageLong (Exclusive Delivery) result: %d\n", rc );
					}
					else
					{
						/* Ignore it. This can happen if,
						   longTypeMessage->imvID matches with any other IMV OR
						   longTypeMessage->imvID == TNC_IMVID_ANY */
						outfmt( OUT_LEVEL_NORMAL, "> Message marked for exclusive delivery to another IMV; not delivered!\n" );
					}
				}
				else if( IsMessageLongTypeSupported( longTypeMessage->messageSubtype, longTypeMessage->messageVendorID, 
												g_pImvMessageLongSubtypes, g_pImvVendorIDs,
												g_nImvMessageLongSubtypesCount) )
				{
					rc = imvFuncs.pfnReceiveMessageLong( IMV_ID, cid, longTypeMessage->messageFlags, 
						longTypeMessage->message, longTypeMessage->messageLength, 
						longTypeMessage->messageVendorID, longTypeMessage->messageSubtype, 
						longTypeMessage->imvID, longTypeMessage->imvID);
					outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_ReceiveMessageLong result: %d\n", rc );
				}
				else
				{
					outfmt( OUT_LEVEL_NORMAL, "> Message type not registered; message not delivered!\n" );
				}
			} 
			else if (imvFuncs.pfnReceiveMessage)
			{
				/* IMV doesn't implement TNC_IMV_ReceiveMessageLong function but
				   TNCS can still delive the message using imvFuncs.pfnReceiveMessage.*/
				outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_ReceiveMessage (vendorID: %#x, subtype: %#x, length: %d)\n", 
					longTypeMessage->messageVendorID, longTypeMessage->messageSubtype, longTypeMessage->messageLength );

				/* Create a single message type from subtype and vendorID */
				longMessageType = (longTypeMessage->messageVendorID << 8 | longTypeMessage->messageSubtype);

				if( IsMessageTypeSupported( longMessageType, g_pImvMessageTypes, g_nImvMessageTypesCount ) )
				{
					rc = imvFuncs.pfnReceiveMessage( IMV_ID, cid, longTypeMessage->message, 
						longTypeMessage->messageLength, longMessageType );
					outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_ReceiveMessage result: %d\n", rc );
				}
				else
				{
					outfmt( OUT_LEVEL_NORMAL, "> Message type not registered; message not delivered!\n" );
				}
			}
			else
			{
				outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_ReceiveMessage and "
					"TNC_IMV_ReceiveMessageLong NOT SUPPORTED!\n" );
			}
		}
	}

    return 0;
}

unsigned NotifyImvConnectionState( TNC_ConnectionID cid, TNC_ConnectionState state )
{
    TNC_Result rc = TNC_RESULT_SUCCESS;
    extern char *g_pszConnStates[];

    if( NULL != imvFuncs.pfnNotifyConnectionChange )
    {
        outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_NotifyConnectionChange (IMV: %d, CID: %d, state: `%s')\n", 
            IMV_ID, cid, g_pszConnStates[ state ] );

        rc = imvFuncs.pfnNotifyConnectionChange( IMV_ID, cid, state );
        outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_NotifyConnectionChange result: %d\n", rc );
    }

    return rc;
}

unsigned ImvBatchEnding( TNC_ConnectionID cid )
{
    TNC_Result rc = TNC_RESULT_SUCCESS;

    if( NULL != imvFuncs.pfnBatchEnding )
    {
        outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_BatchEnding (IMV: %d, CID: %d)\n", IMV_ID, cid );
        rc = imvFuncs.pfnBatchEnding( IMV_ID, cid );
        outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_BatchEnding result: %d\n", rc );
    }

    return rc;
}

unsigned ImvGetRecommendation( TNC_ConnectionID cid, unsigned *result )
{
    TNC_Result rc;
    static unsigned nRecommendation2ConnState[] = 
    {
        TNC_CONNECTION_STATE_ACCESS_ALLOWED, TNC_CONNECTION_STATE_ACCESS_NONE, 
        TNC_CONNECTION_STATE_ACCESS_ISOLATED, TNC_CONNECTION_STATE_ACCESS_NONE
    };

    if( ! g_bRecommendationProvided )
    {
        outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_SolicitRecommendation (IMV: %d, CID: %d)\n", IMV_ID, cid );
        rc = imvFuncs.pfnSolicitRecommendation( IMV_ID, cid );
        outfmt( OUT_LEVEL_NORMAL, "> TNC_IMV_SolicitRecommendation result %d\n", rc );

        if( TNC_RESULT_SUCCESS != rc )
            return -1;
    }

    if( NULL != result )
        *result = g_nEvaluation;

    return nRecommendation2ConnState[ g_nRecommendation ];
}

TNC_Result TNC_TNCS_SendMessage(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_BufferReference message,
/*in*/  TNC_UInt32 messageLength,
/*in*/  TNC_MessageType messageType) 
{
	MESSAGE_BASIC  basicMessage;

    outfmt( OUT_LEVEL_NORMAL, "< TNC_TNCS_SendMessage: IMV %d, CID %d, msg length %d, msg type %#x\n", 
        imvID, connectionID, messageLength, messageType );

    outfmt( OUT_LEVEL_VERBOSE, "< IMV message data:\n" );
    outmessage( OUT_LEVEL_VERBOSE, message, messageLength );

	basicMessage.message = message;
	basicMessage.messageLength = messageLength;
	basicMessage.messageType = messageType;

    QueueAddMessage( &basicMessage );

    return TNC_RESULT_OTHER;
}

TNC_Result TNC_TNCS_SendMessageSOH(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_BufferReference sohReportEntry,
/*in*/  TNC_UInt32 sohRELength)
{
	MESSAGE_SOH  sohMessage;

    outfmt( OUT_LEVEL_NORMAL, "< TNC_TNCS_SendMessageSOH: IMV %d, CID %d, msg length %d\n", 
        imvID, connectionID, sohRELength );

    outfmt( OUT_LEVEL_VERBOSE, "< IMV message data:\n" );
    outmessage( OUT_LEVEL_VERBOSE, sohReportEntry, sohRELength );

	sohMessage.sohReportEntry = sohReportEntry;
	sohMessage.sohRELength = sohRELength;

    QueueAddMessageSOH( &sohMessage );

    return TNC_RESULT_SUCCESS;
}

TNC_Result TNC_TNCS_SendMessageLong(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_UInt32 messageFlags,
/*in*/  TNC_BufferReference message,
/*in*/  TNC_UInt32 messageLength,
/*in*/  TNC_VendorID messageVendorID,
/*in*/  TNC_MessageSubtype messageSubtype,
/*in*/  TNC_UInt32 destinationIMCID)
{
	MESSAGE_LONG  longTypeMessage;

    outfmt( OUT_LEVEL_NORMAL, "< TNC_TNCS_SendMessageLong: IMV %d, CID %d, "
		"msg length %d, vendor ID %#x msg subtype %#x, msg flags %#x, destIMCID %#x\n", 
        imvID, connectionID, messageLength, messageVendorID, 
		messageSubtype, messageFlags, destinationIMCID );

    outfmt( OUT_LEVEL_VERBOSE, "< IMC message data:\n" );
    outmessage( OUT_LEVEL_VERBOSE, message, messageLength );

	longTypeMessage.imcID = destinationIMCID;
	longTypeMessage.imvID = imvID;
	longTypeMessage.message = message;
	longTypeMessage.messageFlags = messageFlags;
	longTypeMessage.messageLength = messageLength;
	longTypeMessage.messageSubtype = messageSubtype;
	longTypeMessage.messageVendorID = messageVendorID;

    QueueAddMessageLong( &longTypeMessage );

    return TNC_RESULT_SUCCESS;
}

TNC_Result TNC_TNCS_ProvideRecommendation(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_IMV_Action_Recommendation recommendation,
/*in*/  TNC_IMV_Evaluation_Result compliance) 
{
    static char *rs[] = 
    {
        "Allow", "DENY", "Isolate", "No recommendation" 
    };

    static char *cs[] = 
    {
        "Compliant", "minor noncompliance", "MAJOR noncompliance", "Error", "Don't know"
    };

    outfmt( OUT_LEVEL_NORMAL, "< TNC_TNCS_ProvideRecommendation: IMV %d, CID %d, '%s', '%s'\n",
        imvID, connectionID, rs[ recommendation ], cs[ compliance ] );

    g_nRecommendation = recommendation;
    g_nEvaluation = compliance;
    g_bRecommendationProvided = 1;
    return TNC_RESULT_SUCCESS;
}

TNC_Result TNC_TNCS_ReportMessageTypes(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_MessageTypeList supportedTypes,
/*in*/  TNC_UInt32 typeCount) 
{
    unsigned i;

    if( typeCount > g_nImvMessageTypesCount )
        g_pImvMessageTypes = (TNC_MessageTypeList) realloc( g_pImvMessageTypes, sizeof( *supportedTypes ) * typeCount );

    outfmt( OUT_LEVEL_NORMAL, "< TNC_TNCS_ReportMessageTypes (IMV %d)", imvID );
    if( typeCount > 0 )
    {
        memcpy( g_pImvMessageTypes, supportedTypes, sizeof( *supportedTypes ) * typeCount );
        g_nImvMessageTypesCount = typeCount;

        for( i=0; i < typeCount; ++i )
            outfmt( OUT_LEVEL_NORMAL, "%c%#x%c", 
                0 == i ? '\n' : ' ', 
                g_pImvMessageTypes[ i ], 
                i == typeCount - 1 ? '\n' : ',' );
    }
    else
	{
        outfmt( OUT_LEVEL_NORMAL, "\nEmpty message list. No messages will be delivered to this IMV!\n" );
	}

    return TNC_RESULT_SUCCESS;
}

TNC_Result TNC_TNCS_ReportMessageTypesLong(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_VendorIDList supportedVendorIDs,
/*in*/  TNC_MessageSubtypeList supportedSubtypes,
/*in*/  TNC_UInt32 typeCount)
{
    unsigned i;

    if( typeCount > g_nImvMessageLongSubtypesCount )
	{
		g_pImvMessageLongSubtypes = (TNC_MessageSubtypeList) realloc( g_pImvMessageLongSubtypes, sizeof( *supportedSubtypes ) * typeCount );
		g_pImvVendorIDs = (TNC_VendorIDList) realloc( g_pImvVendorIDs, sizeof(TNC_VendorID) * typeCount );
	}

    outfmt( OUT_LEVEL_NORMAL, "< TNC_TNCS_ReportMessageTypesLong (IMV %d)", imvID );
    if( typeCount > 0 )
    {
        memcpy( g_pImvMessageLongSubtypes, supportedSubtypes, sizeof( *supportedSubtypes ) * typeCount );
		memcpy( g_pImvVendorIDs, supportedVendorIDs, sizeof( *supportedVendorIDs ) * typeCount );
        g_nImvMessageLongSubtypesCount = typeCount;

        for( i=0; i < typeCount; ++i )
			outfmt( OUT_LEVEL_NORMAL, "%c(vendor ID %#x, message subtype %#x)%c", 
                0 == i ? '\n' : ' ', 
				g_pImvVendorIDs[i],
                g_pImvMessageLongSubtypes[i], 
                i == typeCount - 1 ? '\n' : ',' );
    }
    else
	{
        outfmt( OUT_LEVEL_NORMAL, "\nEmpty message list. No messages will be delivered to this IMV!\n" );
	}

    return TNC_RESULT_SUCCESS;
}

TNC_Result TNC_TNCS_RequestHandshakeRetry(
/*in*/  TNC_IMVID imvID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_RetryReason reason)
{
    return TNC_RESULT_SUCCESS;
}

static TNC_Result CopyBuffer( const char *src, TNC_UInt32 dstLen, TNC_BufferReference dst, TNC_UInt32 *strLen )
{
    *strLen = 1 + strlen( src );

    if( dstLen < *strLen )
        return TNC_RESULT_OTHER;

    strcpy( (char*) dst, src );
    return TNC_RESULT_SUCCESS;
}


TNC_Result TNC_TNCS_BindFunction(
/*in*/  TNC_IMVID imvID,
/*in*/  char *functionName,
/*out*/ void **pOutfunctionPointer) 
{
    *pOutfunctionPointer = NULL;
    if (!strcmp(functionName, "TNC_TNCS_BindFunction"))
        *pOutfunctionPointer = &TNC_TNCS_BindFunction;

    else if (!strcmp(functionName, "TNC_TNCS_SendMessage"))
        *pOutfunctionPointer = &TNC_TNCS_SendMessage;

	else if (!strcmp(functionName, "TNC_TNCS_SendMessageSOH"))
        *pOutfunctionPointer = &TNC_TNCS_SendMessageSOH;

	else if (!strcmp(functionName, "TNC_TNCS_SendMessageLong"))
        *pOutfunctionPointer = &TNC_TNCS_SendMessageLong;

    else if (!strcmp(functionName, "TNC_TNCS_ProvideRecommendation"))
        *pOutfunctionPointer = &TNC_TNCS_ProvideRecommendation;

    else if (!strcmp(functionName, "TNC_TNCS_ReportMessageTypes"))
        *pOutfunctionPointer = &TNC_TNCS_ReportMessageTypes;

    else if (!strcmp(functionName, "TNC_TNCS_ReportMessageTypesLong"))
        *pOutfunctionPointer = &TNC_TNCS_ReportMessageTypesLong;

    else if (!strcmp(functionName, "TNC_TNCS_RequestHandshakeRetry"))
        *pOutfunctionPointer = &TNC_TNCS_RequestHandshakeRetry;

    return TNC_RESULT_SUCCESS;
}

/*
 * IMCIMVTNCC.c
 *
 * Platform-independent portions of IMCIMVTester TNCC Code
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

#include "IMCIMVTNCC.h"
#include "IMCIMVTester.h"
#include "msgqueue.h"
#include "output.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* TNCC will assign this IMC ID after loading the IMC */
#define IMC_ID  0

/* List of functions implemented by IMC */
static IMCFuncs imcFuncs;

/* List of message types supported by IMC in TNC_TNCC_ReportMessageTypes */
static TNC_MessageTypeList g_pImcMessageTypes = NULL;
static TNC_UInt32 g_nImcMessageTypesCount = 0;

/* List of message types supported by IMC in TNC_TNCC_ReportMessageTypesLong */
static TNC_MessageSubtypeList g_pImcMessageLongSubtypes = NULL;
static TNC_VendorIDList g_pImcVendorIDs = NULL;
static TNC_UInt32 g_nImcMessageLongSubtypesCount = 0;

/* These functions are defined in platform specific files */
int LoadImcDLL(const char *dllPath, IMCFuncs *funcTable);
void UnloadImcDLL(void);

/* These extract sub-information from TNC_MessageType */
#define EXTRACT_VENDOR(x) (x >> 8)
#define EXTRACT_SUBTYPE(x) (x & 0xff)

char *g_pszConnStates[] = 
{
    "Create", "Handshake", "Access Allowed", "Access Isolated", "Access DENIED", "Delete"
};

int LoadIMC(const char *dllPath)
{
    int err;

    outfmt( OUT_LEVEL_NORMAL, "Loading IMC DLL: \"%s\"...", dllPath );
    err = LoadImcDLL( dllPath, &imcFuncs );
    if (err) 
    {
        outfmt( OUT_LEVEL_NORMAL, " Error %d.\n", err );
        return err;
    }

    outfmt( OUT_LEVEL_NORMAL, " Ok\n" );
    return TNC_RESULT_SUCCESS;
}

int InitializeIMC(void)
{
    TNC_Result result;
    TNC_Version actualVersion;

    outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_Initialize\n" );
    result = (imcFuncs.pfnInitialize)(IMC_ID, TNC_IFIMC_VERSION_1, TNC_IFIMC_VERSION_1, &actualVersion);
    outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_Initialize result: %d.\n", result);
    if (result != TNC_RESULT_SUCCESS) 
        return result;

    if (imcFuncs.pfnProvideBind != NULL) 
    {
        outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_ProvideBindFunction\n" );
        result = (imcFuncs.pfnProvideBind)(0, &TNC_TNCC_BindFunction);
        outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_ProvideBindFunction result: %d.\n", result);
        if (result != TNC_RESULT_SUCCESS) 
            return result;
    }

    outfmt( OUT_LEVEL_NORMAL, "IMC initialized successfully\n\n" );
    return 0;
}

int TerminateIMC(void)
{
    TNC_Result result;

    if( NULL != imcFuncs.pfnTerminate )
    {
        outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_Terminate (IMC %d)\n", IMC_ID );
        result = imcFuncs.pfnTerminate( IMC_ID );
        outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_Terminate result: %d\n", result );
    }

    free( g_pImcMessageTypes );
    g_pImcMessageTypes = NULL;
    g_nImcMessageTypesCount = 0;

	free( g_pImcMessageLongSubtypes );
	free( g_pImcVendorIDs );
	g_pImcMessageLongSubtypes = NULL;
	g_pImcVendorIDs = NULL;
	g_nImcMessageLongSubtypesCount = 0;

    UnloadImcDLL();
    return 0;
}

int IsMessageTypeSupported( TNC_MessageType type, TNC_MessageTypeList list, TNC_UInt32 count )
{
    TNC_UInt32 i;
    TNC_MessageType myType;
    TNC_VendorID myVendor, vendor = EXTRACT_VENDOR( type );
    TNC_MessageSubtype mySubtype, subtype = EXTRACT_SUBTYPE( type );

    if( NULL == list )
        return 0;

    for( i=0; i < count; ++i )
    {
        myType = list[ i ];
        mySubtype = EXTRACT_SUBTYPE( myType );
        myVendor = EXTRACT_VENDOR( myType );
        if( myType == type 
            || (TNC_VENDORID_ANY == myVendor && mySubtype == subtype)
            || (TNC_SUBTYPE_ANY == mySubtype && myVendor == vendor)
            || (TNC_SUBTYPE_ANY == mySubtype && TNC_VENDORID_ANY == myVendor) )
            return 1;
    }

    return 0;
}

int IsMessageLongTypeSupported( TNC_MessageSubtype subtype, 
							    TNC_VendorID vendorID,
								TNC_MessageSubtypeList listOfSubTypes,
							    TNC_VendorIDList listOfVendorIDs, 
								TNC_UInt32 count )
{
    TNC_UInt32 i;
    TNC_VendorID myVendor;
    TNC_MessageSubtype mySubtype;

	if( count == 0 )
        return 0;

    for( i=0; i < count; ++i )
    {
        mySubtype = listOfSubTypes[i];
		myVendor  = listOfVendorIDs[i];

        if( (subtype == mySubtype && vendorID == myVendor)
			|| (TNC_VENDORID_ANY == myVendor && mySubtype == subtype)
            || (TNC_SUBTYPE_ANY == mySubtype && myVendor == vendorID)
            || (TNC_SUBTYPE_ANY == mySubtype && TNC_VENDORID_ANY == myVendor) )
            return 1;
    }

    return 0;
}

unsigned DeliverImcMessages( TNC_ConnectionID cid )
{
	/* TNCC may receive messages belonging to different categories. Either of
	   these pointers will refer to the current message depending on its category */
	unsigned messageCategory = MESSAGE_CATEGORY_UNKNOWN;
	MESSAGE_BASIC  * basicMessage = NULL;
	MESSAGE_SOH    * sohMessage = NULL;
	MESSAGE_LONG   * longTypeMessage = NULL;
	TNC_MessageType  sohMessageType = 0;
	TNC_MessageType  longMessageType = 0;
    TNC_Result rc;
    unsigned i;

	/* Deliver each message to the IMC */
	for (i=0; i < QueueGetMessageCount(); ++i)
	{
		/* Depending on the message category, it needs to be delivered differently */
		messageCategory = QueueGetMessageCategory(i);

		if (messageCategory == MESSAGE_CATEGORY_BASIC) 
		{
			/* Now that we know the message type, retrieve the message */
			QueueGetMessage(i, &basicMessage);

			outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_ReceiveMessage (type: %#x, length: %d)\n", 
				basicMessage->messageType, basicMessage->messageLength );

			if( imcFuncs.pfnReceiveMessage )
			{
				if( IsMessageTypeSupported(basicMessage->messageType, g_pImcMessageTypes, g_nImcMessageTypesCount) )
				{
					rc = imcFuncs.pfnReceiveMessage( IMC_ID, cid, basicMessage->message, 
						basicMessage->messageLength, basicMessage->messageType );

					outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_ReceiveMessage result: %d\n", rc );
				}
				else
				{
					outfmt( OUT_LEVEL_NORMAL, "> Message type not registered; message not delivered!\n" );
				}
			}
			else
			{
				outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_ReceiveMessage TYPE NOT SUPPORTED!\n" );
			}
		}
		else if (messageCategory == MESSAGE_CATEGORY_SOH)
		{
			/* Now that we know the message type, retrieve the message */
			QueueGetMessageSOH(i, &sohMessage);

			/* This is the preferred way of delivery */
			if (imcFuncs.pfnReceiveMessageSOH)
			{
				/* The received buffer is complete SOHReportEntry. TNCC will parse
				   it into individual SOHRReportEntry buffers and deliver each buffer
				   to the IMC if IMC is supposed to receive it. Since logic for 
				   parsing SOH message is somewhat involved, it is not implemented.

				   Deliver the parsed SOHRReportEntries using imcFuncs.pfnReceiveMessageSOH.
				*/
				outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_ReceiveMessageSOH (length: %d)\n", 
					sohMessage->sohRELength);
				outfmt( OUT_LEVEL_NORMAL, "> Dispatching SOH messages to IMC **NOT IMPLEMENTED**\n");
			} 
			else if (imcFuncs.pfnReceiveMessage)
			{
				/* IMC didn't implement TNC_IMC_ReceiveMessageSOH function but 
				   TNCC can still delive the message using the pfnReceiveMessage. 
				   'sohMessageType' is extracted from the message. */

				outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_ReceiveMessage (type: %#x, length: %d)\n", 
					sohMessageType, sohMessage->sohRELength);

				if( IsMessageTypeSupported( sohMessageType, g_pImcMessageTypes, g_nImcMessageTypesCount ) )
				{
					rc = imcFuncs.pfnReceiveMessage( IMC_ID, cid, sohMessage->sohReportEntry, 
						sohMessage->sohRELength, sohMessageType );
					outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_ReceiveMessage result: %d\n", rc );
				}
				else
				{
					outfmt( OUT_LEVEL_NORMAL, "> Message type not registered; message not delivered!\n" );
				}
			}
			else 
			{
				outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_ReceiveMessage and "
						"TNC_IMC_ReceiveMessageSOH NOT SUPPORTED!\n" );
			}
		} 
		else if (messageCategory == MESSAGE_CATEGORY_LONG ) 
		{
			/* Now that we know the message type, retrieve the message */
			QueueGetMessageLong(i, &longTypeMessage);

			/* This is the preferred way of delivery */
			if (imcFuncs.pfnReceiveMessageLong) 
			{
				outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_ReceiveMessageLong (vendorID: %#x, subtype: %#x, length: %d)\n", 
					longTypeMessage->messageVendorID, longTypeMessage->messageSubtype, longTypeMessage->messageLength );

				if( (longTypeMessage->messageFlags & TNC_MESSAGE_FLAGS_EXCLUSIVE) == TNC_MESSAGE_FLAGS_EXCLUSIVE )
				{
					if(longTypeMessage->imcID == IMC_ID)
					{
						rc = imcFuncs.pfnReceiveMessageLong( IMC_ID, cid, longTypeMessage->messageFlags, 
							longTypeMessage->message, longTypeMessage->messageLength, 
							longTypeMessage->messageVendorID, longTypeMessage->messageSubtype, 
							longTypeMessage->imvID, longTypeMessage->imcID);

						outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_ReceiveMessageLong (Exclusive Delivery) result: %d\n", rc );
					}
					else
					{
						/* Ignore it. This can happen if,
						   longTypeMessage->imcID matches with any other IMC OR
						   longTypeMessage->imcID == TNC_IMCID_ANY */
						outfmt( OUT_LEVEL_NORMAL, "> Message marked for exclusive delivery to another IMC; not delivered!\n" );
					}
				}
				else if( IsMessageLongTypeSupported( longTypeMessage->messageSubtype, longTypeMessage->messageVendorID, 
												g_pImcMessageLongSubtypes, g_pImcVendorIDs,
												g_nImcMessageLongSubtypesCount) )
				{
					rc = imcFuncs.pfnReceiveMessageLong( IMC_ID, cid, longTypeMessage->messageFlags, 
						longTypeMessage->message, longTypeMessage->messageLength, 
						longTypeMessage->messageVendorID, longTypeMessage->messageSubtype, 
						longTypeMessage->imvID, longTypeMessage->imcID);
					outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_ReceiveMessageLong result: %d\n", rc );
				}
				else
				{
					outfmt( OUT_LEVEL_NORMAL, "> Message type not registered; message not delivered!\n" );
				}
			} 
			else if (imcFuncs.pfnReceiveMessage) 
			{
				/* IMC doesn't implement TNC_IMC_ReceiveMessageLong function but 
				   TNCC can still delive the message using imcFuncs.pfnReceiveMessage.*/
				outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_ReceiveMessage (vendorID: %#x, subtype: %#x, length: %d)\n", 
					longTypeMessage->messageVendorID, longTypeMessage->messageSubtype, longTypeMessage->messageLength );

				/* Create a single message type from subtype and vendorID */
				longMessageType = (longTypeMessage->messageVendorID << 8 | longTypeMessage->messageSubtype);

				if( IsMessageTypeSupported( longMessageType, g_pImcMessageTypes, g_nImcMessageTypesCount ) )
				{
					rc = imcFuncs.pfnReceiveMessage( IMC_ID, cid, longTypeMessage->message, 
						longTypeMessage->messageLength, longMessageType );
					outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_ReceiveMessage result: %d\n", rc );
				}
				else
				{
					outfmt( OUT_LEVEL_NORMAL, "> Message type not registered; message not delivered!\n" );
				}
			}
			else
			{
				outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_ReceiveMessage and "
					"TNC_IMC_ReceiveMessageLong NOT SUPPORTED!\n" );
			}
		}
	}

    return 0;
}

unsigned NotifyImcConnectionState( TNC_ConnectionID cid, TNC_ConnectionState state )
{
    TNC_Result rc = TNC_RESULT_SUCCESS;

    if( NULL != imcFuncs.pfnNotifyConnChg )
    {
        outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_NotifyConnectionChange (IMC: %d, CID: %d, state: `%s')\n", 
            IMC_ID, cid, g_pszConnStates[ state ] );

        rc = imcFuncs.pfnNotifyConnChg( IMC_ID, cid, state );
        outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_NotifyConnectionChange result: %d\n", rc );
    }

    return rc;
}

unsigned ImcBeginHandshake( TNC_ConnectionID cid )
{
    TNC_Result rc;

    outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_BeginHandshake (IMC: %d, CID: %d)\n", IMC_ID, cid );
    rc = imcFuncs.pfnBeginHandshake( IMC_ID, cid );
    outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_BeginHandshake result: %d\n", rc );

    return rc;
}

unsigned ImcBatchEnding( TNC_ConnectionID cid )
{
    TNC_Result rc = TNC_RESULT_SUCCESS;

    if( NULL != imcFuncs.pfnBatchEnding )
    {
        outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_BatchEnding (IMC: %d, CID: %d)\n", IMC_ID, cid );
        rc = imcFuncs.pfnBatchEnding( IMC_ID, cid );
        outfmt( OUT_LEVEL_NORMAL, "> TNC_IMC_BatchEnding result: %d\n", rc );
    }

    return rc;
}


/* TNCC IMC API Functions - Called By IMC's*/

TNC_Result TNC_TNCC_ReportMessageTypes(
/*in*/  TNC_IMCID imcID,
/*in*/  TNC_MessageTypeList supportedTypes,
/*in*/  TNC_UInt32 typeCount)
{
    unsigned i;

    if( typeCount > g_nImcMessageTypesCount )
		g_pImcMessageTypes = (TNC_MessageTypeList) realloc( g_pImcMessageTypes, sizeof( *supportedTypes ) * typeCount );

    outfmt( OUT_LEVEL_NORMAL, "< TNC_TNCC_ReportMessageTypes (IMC %d)", imcID );
    if( typeCount > 0 )
    {
        memcpy( g_pImcMessageTypes, supportedTypes, sizeof( *supportedTypes ) * typeCount );
        g_nImcMessageTypesCount = typeCount;

        for( i=0; i < typeCount; ++i )
            outfmt( OUT_LEVEL_NORMAL, "%c%#x%c", 
                0 == i ? '\n' : ' ', 
                g_pImcMessageTypes[ i ], 
                i == typeCount - 1 ? '\n' : ',' );
    }
    else
	{
        outfmt( OUT_LEVEL_NORMAL, "\nEmpty message list. No messages will be delivered to this IMC!\n" );
	}

    return TNC_RESULT_SUCCESS;
}

TNC_Result TNC_TNCC_ReportMessageTypesLong(
/*in*/  TNC_IMCID imcID,
/*in*/  TNC_VendorIDList supportedVendorIDs,
/*in*/  TNC_MessageSubtypeList supportedSubtypes,
/*in*/  TNC_UInt32 typeCount)
{
    unsigned i;

    if( typeCount > g_nImcMessageLongSubtypesCount )
	{
		g_pImcMessageLongSubtypes = (TNC_MessageSubtypeList) realloc( g_pImcMessageLongSubtypes, sizeof( *supportedSubtypes ) * typeCount );
		g_pImcVendorIDs = (TNC_VendorIDList) realloc( g_pImcVendorIDs, sizeof(TNC_VendorID) * typeCount );
	}

    outfmt( OUT_LEVEL_NORMAL, "< TNC_TNCC_ReportMessageTypesLong (IMC %d)", imcID );
    if( typeCount > 0 )
    {
        memcpy( g_pImcMessageLongSubtypes, supportedSubtypes, sizeof( *supportedSubtypes ) * typeCount );
		memcpy( g_pImcVendorIDs, supportedVendorIDs, sizeof( *supportedVendorIDs ) * typeCount );
        g_nImcMessageLongSubtypesCount = typeCount;

        for( i=0; i < typeCount; ++i )
			outfmt( OUT_LEVEL_NORMAL, "%c(vendor ID %#x, message subtype %#x)%c", 
                0 == i ? '\n' : ' ', 
				g_pImcVendorIDs[i],
                g_pImcMessageLongSubtypes[i],
                i == typeCount - 1 ? '\n' : ',' );
    }
    else
	{
        outfmt( OUT_LEVEL_NORMAL, "\nEmpty message list. No messages will be delivered to this IMC!\n" );
	}

    return TNC_RESULT_SUCCESS;
}

TNC_Result TNC_TNCC_SendMessage(
/*in*/  TNC_IMCID imcID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_BufferReference message,
/*in*/  TNC_UInt32 messageLength,
/*in*/  TNC_MessageType messageType) 
{
	MESSAGE_BASIC  basicMessage;

    outfmt( OUT_LEVEL_NORMAL, "< TNC_TNCC_SendMessage: IMC %d, CID %d, msg length %d, msg type %#x\n", 
        imcID, connectionID, messageLength, messageType );

    outfmt( OUT_LEVEL_VERBOSE, "< IMC message data:\n" );
    outmessage( OUT_LEVEL_VERBOSE, message, messageLength );

	basicMessage.message = message;
	basicMessage.messageLength = messageLength;
	basicMessage.messageType = messageType;

    QueueAddMessage( &basicMessage );

    return TNC_RESULT_SUCCESS;
}

TNC_Result TNC_TNCC_SendMessageSOH(
/*in*/  TNC_IMCID imcID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_BufferReference sohReportEntry,
/*in*/  TNC_UInt32 sohRELength)
{
	MESSAGE_SOH  sohMessage;

    outfmt( OUT_LEVEL_NORMAL, "< TNC_TNCC_SendMessageSOH: IMC %d, CID %d, msg length %d\n", 
        imcID, connectionID, sohRELength );

    outfmt( OUT_LEVEL_VERBOSE, "< IMC message data:\n" );
    outmessage( OUT_LEVEL_VERBOSE, sohReportEntry, sohRELength );

	sohMessage.sohReportEntry = sohReportEntry;
	sohMessage.sohRELength = sohRELength;

    QueueAddMessageSOH( &sohMessage );

    return TNC_RESULT_SUCCESS;
}

TNC_Result TNC_TNCC_SendMessageLong(
/*in*/  TNC_IMCID imcID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_UInt32 messageFlags,
/*in*/  TNC_BufferReference message,
/*in*/  TNC_UInt32 messageLength,
/*in*/  TNC_VendorID messageVendorID,
/*in*/  TNC_MessageSubtype messageSubtype,
/*in*/  TNC_UInt32 destinationIMVID)
{
	MESSAGE_LONG  longTypeMessage;

    outfmt( OUT_LEVEL_NORMAL, "< TNC_TNCC_SendMessageLong: IMC %d, CID %d, "
		"msg length %d, vendor ID %#x msg subtype %#x, msg flags %#x, destIMVID %#x\n", 
        imcID, connectionID, messageLength, messageVendorID,
		messageSubtype, messageFlags, destinationIMVID );

    outfmt( OUT_LEVEL_VERBOSE, "< IMC message data:\n" );
    outmessage( OUT_LEVEL_VERBOSE, message, messageLength );

	longTypeMessage.imcID = imcID;
	longTypeMessage.imvID = destinationIMVID;
	longTypeMessage.message = message;
	longTypeMessage.messageFlags = messageFlags;
	longTypeMessage.messageLength = messageLength;
	longTypeMessage.messageSubtype = messageSubtype;
	longTypeMessage.messageVendorID = messageVendorID;

    QueueAddMessageLong( &longTypeMessage );

    return TNC_RESULT_SUCCESS;
}

TNC_Result TNC_TNCC_RequestHandshakeRetry(
/*in*/  TNC_IMCID imcID,
/*in*/  TNC_ConnectionID connectionID,
/*in*/  TNC_RetryReason reason)
{
    return TNC_RESULT_SUCCESS;
}


TNC_Result TNC_TNCC_BindFunction(
/*in*/  TNC_IMCID imcID,
/*in*/  char *functionName,
/*out*/ void **pOutfunctionPointer) 
{
    *pOutfunctionPointer = NULL;
    if (!strcmp(functionName, "TNC_TNCC_BindFunction"))
        *pOutfunctionPointer = &TNC_TNCC_BindFunction;

    else if (!strcmp(functionName, "TNC_TNCC_ReportMessageTypes"))
        *pOutfunctionPointer = &TNC_TNCC_ReportMessageTypes;

    else if (!strcmp(functionName, "TNC_TNCC_ReportMessageTypesLong"))
        *pOutfunctionPointer = &TNC_TNCC_ReportMessageTypesLong;

    else if (!strcmp(functionName, "TNC_TNCC_RequestHandshakeRetry"))
        *pOutfunctionPointer = &TNC_TNCC_RequestHandshakeRetry;

    else if (!strcmp(functionName, "TNC_TNCC_SendMessage"))
        *pOutfunctionPointer = &TNC_TNCC_SendMessage;

    else if (!strcmp(functionName, "TNC_TNCC_SendMessageSOH"))
        *pOutfunctionPointer = &TNC_TNCC_SendMessageSOH;

    else if (!strcmp(functionName, "TNC_TNCC_SendMessageLong"))
        *pOutfunctionPointer = &TNC_TNCC_SendMessageLong;

    return TNC_RESULT_SUCCESS;
}

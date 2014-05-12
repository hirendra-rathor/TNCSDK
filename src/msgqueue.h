/*
 * msgqueue.h
 *
 * Header File for TNC SDK Message Queue Code
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

#include "tncifimc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TNC_TNCC_SendMessage and TNC_TNCS_SendMessage family of APIs accept different
   arguments and send the received data to the other side to be passed on to the
   IMC or IMV. For the sake of convenience, these arguments are collectively 
   referrred to as category and packed in a structure for ease of understanding.

   A better structure packing could have been achieved and that would certainly
   be the goal with the production code. However this code is supposed to 
   illustrate rather than write the mose efficient code, hence the structures
   have no overlap.

   This header defines these structures and the APIs to manipulate storing/retrieving
   these structures from the message queue.
*/

#define MESSAGE_CATEGORY_UNKNOWN 0
#define MESSAGE_CATEGORY_BASIC 1
#define MESSAGE_CATEGORY_SOH 2
#define MESSAGE_CATEGORY_LONG 3

unsigned IsQueueEmpty(void);

unsigned QueueGetMessageCount(void);

unsigned QueueGetMessageCategory(unsigned index);

unsigned QueueClearMessages(void);

unsigned QueueSaveState(void);

/* Used by TNC_TNCC_SendMessage, TNC_IMC_ReceiveMessage, 
		   TNC_TNCS_SendMessage, TNC_IMV_ReceiveMessage*/
typedef struct MESSAGE_BASIC_tag
{
	TNC_BufferReference message;
	TNC_UInt32 messageLength;
	TNC_MessageType	messageType;
} MESSAGE_BASIC;

unsigned QueueAddMessage(MESSAGE_BASIC   *basicMessage);

unsigned QueueGetMessage(unsigned index, MESSAGE_BASIC  **basicMessage);

/* Used by TNC_TNCC_SendMessageSOH, TNC_IMC_ReceiveMessageSOH,
		   TNC_TNCS_SendMessageSOH, TNC_IMV_ReceiveMessageSOH */
typedef struct MESSAGE_SOH_tag
{
	TNC_BufferReference sohReportEntry;
	TNC_UInt32 sohRELength;
} MESSAGE_SOH;

unsigned QueueAddMessageSOH(MESSAGE_SOH  *sohMessage);

unsigned QueueGetMessageSOH(unsigned index, MESSAGE_SOH **sohMessage);

/* Used by TNC_TNCC_SendMessageLong, TNC_IMC_ReceiveMessageLong,
		   TNC_TNCS_SendMessageLong, TNC_IMV_ReceiveMessageLong */
typedef struct MESSAGE_LONG_tag
{
	TNC_UInt32 messageFlags;
	TNC_BufferReference message;
	TNC_UInt32 messageLength;
	TNC_VendorID messageVendorID;
	TNC_MessageSubtype messageSubtype;
	TNC_UInt32 imcID;
	TNC_UInt32 imvID;
} MESSAGE_LONG;

unsigned QueueAddMessageLong(MESSAGE_LONG  *longTypeMessage);

unsigned QueueGetMessageLong(unsigned index, MESSAGE_LONG **longTypeMessage);

#ifdef __cplusplus
}
#endif

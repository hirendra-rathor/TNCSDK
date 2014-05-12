/*
 * msgqueue.c
 *
 * TNC SDK Message Queue Code
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

#include "msgqueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>

/* Generic structure in the message queue; can refer to either of suitable 
   MESSAGE_* structure */
typedef struct MESSAGE_NODE_tag
{
    struct MESSAGE_NODE_tag *next;
	TNC_UInt32	messageCategory;
	union
	{
		MESSAGE_BASIC	basicMessage;
		MESSAGE_SOH		sohMessage;
		MESSAGE_LONG	longTypeMessage;
	};

} MESSAGE_NODE;

/* Pointers to head and tail of the list. Once the client (TNCC/TNCS) is done 
 * inserting the messages into the queue, these messages are ready to be delivered
 * to the other side of the network. At that time, the g_Msg* nodes are copied over
 * to g_Copy* list.
 */
static MESSAGE_NODE *g_MsgListHead = NULL, *g_MsgListTail = NULL;
static MESSAGE_NODE *g_CopyListHead = NULL, *g_CopyListTail = NULL;

MESSAGE_NODE* QueueCreateNode(unsigned int messageCategory)
{
    MESSAGE_NODE *msg;
    msg = (MESSAGE_NODE *) malloc(sizeof(*msg));
    if( NULL != msg )
	{
		memset( msg, 0, sizeof( *msg ) );
		msg->messageCategory = messageCategory;
	}
	return msg;
}

void QueueInsertNode(MESSAGE_NODE* pNode)
{
    if( NULL != g_MsgListTail )
    {
        g_MsgListTail->next = pNode;
        g_MsgListTail = pNode;
    }
    else
        g_MsgListHead = g_MsgListTail = pNode;
}

unsigned QueueGetMessageCategory(unsigned index)
{
    MESSAGE_NODE *pNode;

    for( pNode = g_CopyListHead; NULL != pNode && 0 != index; pNode = pNode->next, --index );
    
    if( NULL == pNode )
		return MESSAGE_CATEGORY_UNKNOWN;

	return pNode->messageCategory;
}

unsigned IsQueueEmpty(void)
{
    return NULL == g_MsgListHead ? 1 : 0;
}

unsigned QueueGetMessageCount(void)
{
	unsigned count = 0;
	MESSAGE_NODE * pNode = NULL;
	for (pNode = g_CopyListHead; NULL != pNode; pNode = pNode->next)
		++count;
	return count;
}

unsigned QueueClearMessages(void)
{
    MESSAGE_NODE *pNode, *tmp;

    for( pNode = g_CopyListHead; NULL != pNode; pNode = tmp )
    {
        tmp = pNode->next;
		
		if ( pNode->messageCategory == MESSAGE_CATEGORY_BASIC )
			free( pNode->basicMessage.message );
		else if( pNode->messageCategory == MESSAGE_CATEGORY_SOH )
			free( pNode->sohMessage.sohReportEntry );
		else if( pNode->messageCategory == MESSAGE_CATEGORY_LONG )
			free( pNode->longTypeMessage.message );

		free( pNode );
    }

    g_CopyListHead = g_CopyListTail = NULL;
    return 0;
}

unsigned QueueSaveState(void)
{
    QueueClearMessages();

    g_CopyListHead = g_MsgListHead;
    g_CopyListTail = g_MsgListTail;

    g_MsgListHead = g_MsgListTail = NULL;
    return 0;
}

/* Functions to add regular messages to the queue */
unsigned QueueAddMessage(MESSAGE_BASIC * basicMessage)
{
    MESSAGE_NODE *node;

	/* Create queue node of the appropriate type */
	node = QueueCreateNode(MESSAGE_CATEGORY_BASIC);
    if( NULL == node )
        return ENOMEM;

	/* Copy all the contents of incoming message; safe to do since it's all
	   basic data types */
	memcpy(&(node->basicMessage), basicMessage, sizeof(*basicMessage));

	/* Buffer needs to be deep-copied */
    if (0 != basicMessage->messageLength) {
		node->basicMessage.message = (TNC_BufferReference) malloc( basicMessage->messageLength );
		if (NULL == node->basicMessage.message) {
            free( node );
            return ENOMEM;
        }
		memcpy( node->basicMessage.message, basicMessage->message, basicMessage->messageLength );
    }

	QueueInsertNode( node );
    return 0;
}

unsigned QueueGetMessage(unsigned index, MESSAGE_BASIC ** basicMessage)
{
    MESSAGE_NODE *node;

    for( node = g_CopyListHead; NULL != node && 0 != index; node = node->next, --index );
    
    if( NULL == node )
        return 0;

	*basicMessage = &(node->basicMessage);
    return 1;
}

/* Functions to add SOH messages to the queue */
unsigned QueueAddMessageSOH(MESSAGE_SOH * sohMessage)
{
    MESSAGE_NODE *node;

	/* Create queue node of the appropriate type */
	node = QueueCreateNode(MESSAGE_CATEGORY_SOH);
    if( NULL == node )
        return ENOMEM;

	/* Copy all the contents of incoming message; safe to do since it's all
	   basic data types */
	memcpy(&(node->sohMessage), sohMessage, sizeof(*sohMessage));

	/* Buffer needs to be deep-copied */
	if (0 != sohMessage->sohRELength) {
		node->sohMessage.sohReportEntry = (TNC_BufferReference) malloc( sohMessage->sohRELength );
		if (NULL == node->sohMessage.sohReportEntry) {
            free( node );
            return ENOMEM;
        }
		memcpy( node->sohMessage.sohReportEntry, sohMessage->sohReportEntry, sohMessage->sohRELength );
    }

	QueueInsertNode( node );
    return 0;
}

unsigned QueueGetMessageSOH(unsigned index, MESSAGE_SOH ** sohMessage)
{
    MESSAGE_NODE *node;

    for( node = g_CopyListHead; NULL != node && 0 != index; node = node->next, --index );
    
    if( NULL == node )
        return 0;

	*sohMessage = &(node->sohMessage);
    return 1;
}

/* Functions to add Long messages to the queue */
unsigned QueueAddMessageLong(MESSAGE_LONG * longTypeMessage)
{
    MESSAGE_NODE *node;

	/* Create queue node of the appropriate type */
	node = QueueCreateNode(MESSAGE_CATEGORY_LONG);
    if( NULL == node )
        return ENOMEM;

	/* Copy all the contents of incoming message; safe to do since it's all
	   basic data types */
	memcpy(&(node->longTypeMessage), longTypeMessage, sizeof(*longTypeMessage));

	/* Buffer needs to be deep-copied */
    if (0 != longTypeMessage->messageLength) {
		node->longTypeMessage.message = (TNC_BufferReference) malloc( longTypeMessage->messageLength );
		if (NULL == node->longTypeMessage.message) {
            free( node );
            return ENOMEM;
        }
		memcpy( node->longTypeMessage.message, longTypeMessage->message, longTypeMessage->messageLength );
    }

	QueueInsertNode( node );
    return 0;
}

unsigned QueueGetMessageLong(unsigned index, MESSAGE_LONG ** longTypeMessage)
{
    MESSAGE_NODE *node;

    for( node = g_CopyListHead; NULL != node && 0 != index; node = node->next, --index );
    
    if( NULL == node )
        return 0;

	*longTypeMessage = &(node->longTypeMessage);
    return 1;
}

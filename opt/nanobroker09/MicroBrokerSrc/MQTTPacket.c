/* (C) Copyright IBM Corp. 2007, 2008
Author: Ian Craggs
*/

#include "MQTTPacket.h"
#include "Log.h"
#include "Clients.h"
#include "Messages.h"

#include <stdlib.h>
#include <string.h>

#include "Heap.h"

static char* packet_names[] = 
{
	"RESERVED", "CONNECT", "CONNACK", "PUBLISH", "PUBACK", "PUBREC", "PUBREL",
	"PUBCOMP", "SUBSCRIBE", "SUBACK", "UNSUBSCRIBE", "UNSUBACK",
	"PINGREQ", "PINGRESP", "DISCONNECT"
};


char* MQTTPacket_name(int ptype)
{
	return (ptype >= 0 && ptype <= DISCONNECT) ? packet_names[ptype] : "UNKNOWN";
}

typedef void* (*pf)(unsigned char, char*, int);
#if defined(NO_BRIDGE)
static pf new_packets[] =
{
	NULL,
	MQTTPacket_connect,
	NULL,  // CONNACK
	MQTTPacket_publish,
	MQTTPacket_ack, // PUBACK
	MQTTPacket_ack, // PUBREC
	MQTTPacket_ack, // PUBREL
	MQTTPacket_ack, // PUBCOMP
	MQTTPacket_subscribe,
	NULL, // SUBACK
	MQTTPacket_unsubscribe,
	MQTTPacket_ack, // UNSUBACK
	MQTTPacket_header_only, // PINGREQ
	MQTTPacket_header_only, // PINGRESP
	MQTTPacket_header_only  // DISCONNECT
};
#else
static pf new_packets[] =
{
	NULL,
	MQTTPacket_connect,
	MQTTPacket_connack, 
	MQTTPacket_publish,
	MQTTPacket_ack, // PUBACK
	MQTTPacket_ack, // PUBREC
	MQTTPacket_ack, // PUBREL
	MQTTPacket_ack, // PUBCOMP
	MQTTPacket_subscribe,
	MQTTPacket_suback, 
	MQTTPacket_unsubscribe,
	MQTTPacket_ack, // UNSUBACK
	MQTTPacket_header_only, // PINGREQ
	MQTTPacket_header_only, // PINGRESP
	MQTTPacket_header_only  // DISCONNECT
};
#endif


void* MQTTPacket_Factory(int socket, int* error)
{
	char* data = NULL;
	static Header header;
	int remaining_length, ptype;
	void* pack = NULL;
	int actual_len = 0;

	*error = SOCKET_ERROR;  // indicate whether an error occurred, or not

	// read the packet data from the socket
	if ((*error = TCPSocket_getch(socket, &(header.byte))) != TCPSOCKET_COMPLETE)   // first byte is the header byte
		return NULL; /* packet not read, *error indicates whether SOCKET_ERROR occurred */

	// now read the remaining length, so we know how much more to read
	if ((*error = MQTTPacket_decode(socket, &remaining_length)) != TCPSOCKET_COMPLETE)
		return NULL; /* packet not read, *error indicates whether SOCKET_ERROR occurred */

	//if (remaining_length > 0)
		// now read the rest, the variable header and payload
		if ((data = TCPSocket_getdata(socket, remaining_length, &actual_len)) == NULL)
			return NULL; /* socket error */

	if (actual_len != remaining_length)
		*error = TCPSOCKET_INTERRUPTED;
	else
	{
		ptype = header.type;
		if (ptype < CONNECT || ptype > DISCONNECT || new_packets[ptype] == NULL)
			#if !defined(NO_DEBUG)
				Log(LOG_DEBUG, Messages_get(107), ptype);
			#else
				;
			#endif
		else
		{
			if ((pack = (*new_packets[ptype])(header.byte, data, remaining_length)) == NULL)
				*error = BAD_MQTT_PACKET;
		}
	}
	return pack;
}


int MQTTPacket_send(int socket, Header header, char* buffer, int buflen)
{
	int rc;
	char *buf;

	buf = malloc(10);
	buf[0] = header.byte;
	rc = 1 + MQTTPacket_encode(&buf[1], buflen);
	rc = TCPSocket_putdatas(socket, buf, rc, 1, &buffer, &buflen);
	if (rc != TCPSOCKET_INTERRUPTED)
	  free(buf);

	return rc;
}


int MQTTPacket_sends(int socket, Header header, int count, char** buffers, int* buflens)
{
	int i, rc, total = 0;
	char *buf;

	buf = malloc(10);
	buf[0] = header.byte;
	for (i = 0; i < count; i++)
		total += buflens[i];
	rc = 1 + MQTTPacket_encode(&buf[1], total);
	rc = TCPSocket_putdatas(socket, buf, rc, count, buffers, buflens);
	if (rc != TCPSOCKET_INTERRUPTED)
	  free(buf);
	return rc;
}


/* Encode the message length according to the MQTT algorithm */
int MQTTPacket_encode(char* buf, int length)
{
	int rc = 0;
	do
	{
		char d = length % 128;
		length /= 128;
		/* if there are more digits to encode, set the top bit of this digit */
		if (length > 0)
			d |= 0x80;
		buf[rc++] = d;
	} while (length > 0);
	return rc;
} // end encode


// decode the message length according to the MQTT algorithm
int MQTTPacket_decode(int socket, int* value)
{
	int rc = 1;  // 1 is OK, -1 error, 0 is no char read
	char c;
	int multiplier = 1;
	*value = 0;
	do
	{
		if ((rc = TCPSocket_getch(socket, &c)) != 1)
			return rc;
		*value += (c & 127) * multiplier;
		multiplier *= 128;
	} while ((c & 128) != 0);
	return rc;
} // end decode


int readInt(char** pptr)
{
	char* ptr = *pptr;
	int len = 256*((unsigned char)(*ptr)) + (unsigned char)(*(ptr+1));
	*pptr += 2;
	return len;
}


char* readUTFlen(char** pptr, char* enddata, int* len)
{
	char* string = NULL;

	if (enddata - (*pptr) > 1) // enough length to read the integer?
	{
		*len = readInt(pptr);
		if (&(*pptr)[*len] <= enddata)
		{
			string = malloc(*len+1);
			memcpy(string, *pptr, *len);
			string[*len] = '\0';
			*pptr += *len;
		}
	}
	return string;
}


char* readUTF(char** pptr, char* enddata)
{
	int len;
	return readUTFlen(pptr, enddata, &len);
}


char readChar(char** pptr)
{
	char c = **pptr;
	(*pptr)++;
	return c;
}


void writeChar(char** pptr, char c)
{
	**pptr = c;
	(*pptr)++;
}


void writeInt(char** pptr, int anInt)
{
	**pptr = (char)(anInt / 256);
	(*pptr)++;
	**pptr = (char)(anInt % 256);
	(*pptr)++;
}


void writeUTF(char** pptr, char* string)
{
	int len = strlen(string);
	writeInt(pptr, len);
	memcpy(*pptr, string, len);
	*pptr += len;
}


int MQTTPacket_checkVersion(Connect* pack)
{
	return (strcmp(pack->Protocol, "MQIsdp") == 0 && 
		(pack->version == 3 || pack->version == NANO_PROTOCOL_VERSION)) ||
				(strcmp(pack->Protocol, "MQIpdp") == 0 && pack->version == 2);
}


void* MQTTPacket_connect(unsigned char aHeader, char* data, int datalen)
{
	Connect* pack = malloc(sizeof(Connect));
	char* curdata = data;
	char* enddata = &data[datalen];
	memset(pack, '\0', sizeof(Connect));
	pack->header.byte = aHeader;
	if ((pack->Protocol = readUTF(&curdata, enddata)) == NULL || // should be "MQIsdp"
		enddata - curdata < 0) // can we read protocol version char?
	{
		MQTTPacket_freeConnect(pack);
		return NULL;
	}
	pack->version = (int)readChar(&curdata); // Protocol version
	if (MQTTPacket_checkVersion(pack))
	{
		pack->flags.all = readChar(&curdata);
		pack->keepAliveTimer = readInt(&curdata);
		pack->clientID = readUTF(&curdata, enddata);
		if (pack->flags.will)
		{
			if ((pack->willTopic = readUTF(&curdata, enddata)) == NULL ||
				  (pack->willMsg = readUTF(&curdata, enddata)) == NULL)
			{
				MQTTPacket_freeConnect(pack);
				return NULL;
			}
			#if !defined(NO_DEBUG)
				Log(LOG_DEBUG, Messages_get(108), pack->willTopic, pack->willMsg, pack->flags.willRetain);
			#endif
		}
	}
	else
	{
		pack->clientID = NULL;
		pack->flags.will = 0;
	}
	return (void*)pack;
} // end constructor


void MQTTPacket_freeConnect(Connect* pack)
{
	if (pack->Protocol != NULL)
		free(pack->Protocol);
	if (pack->clientID != NULL)
		free(pack->clientID);
	if (pack->flags.will)
	{
		if (pack->willTopic != NULL)
			free(pack->willTopic);
		if (pack->willMsg != NULL)
			free(pack->willMsg);
	}
	free(pack);
}


void* MQTTPacket_header_only(unsigned char aHeader, char* data, int datalen)
{
	static unsigned char header = 0;
	header = aHeader;
	return &header;
}


int MQTTPacket_send_pingresp(int socket)
{
	Header header;
	header.byte = 0;
	header.type = PINGRESP;
	return MQTTPacket_send(socket, header, NULL, 0);
}


int MQTTPacket_send_disconnect(int socket)
{
	Header header;
	header.byte = 0;
	header.type = DISCONNECT;
	return MQTTPacket_send(socket, header, NULL, 0);
}


void* MQTTPacket_subscribe(unsigned char aHeader, char* data, int datalen)
{
	Subscribe* pack = malloc(sizeof(Subscribe));
	char* curdata = data;
	char* enddata = &data[datalen];

	pack->header.byte = aHeader;
	pack->msgId = readInt(&curdata);
	pack->topics = ListInitialize();
	pack->qoss = ListInitialize();
	pack->noTopics = 0;
	while (curdata - data < datalen)
	{
		int* newint;
		int len;
		char* str = readUTFlen(&curdata, enddata, &len);
		if (str == NULL || enddata - curdata < 1)
		{
			MQTTPacket_freeSubscribe(pack, 1);
			return NULL;
		}
		ListAppend(pack->topics, str, len);
		newint = malloc(sizeof(int));
		*newint = (int)readChar(&curdata);
		ListAppend(pack->qoss, newint, sizeof(int));
		(pack->noTopics)++;
	}
	return pack;
}


void MQTTPacket_freeSubscribe(Subscribe* pack, int all)
{
	if (pack->topics != NULL && all)
		ListFree(pack->topics);
	else
		ListFreeNoContent(pack->topics);
	if (pack->qoss != NULL)
		ListFree(pack->qoss);
	free(pack);
}


int MQTTPacket_send_suback(int msgid, int noOfTopics, int* qoss, int socket)
{
	Header header;
	char *data, *ptr;
	int i, datalen, rc;

	header.byte = 0;
	header.type = SUBACK;
	datalen = 2 + noOfTopics;
	ptr = data = malloc(datalen);
	writeInt(&ptr, msgid);
	for (i = 0; i < noOfTopics; i++)
		writeChar(&ptr, (char)(qoss[i]));
	rc = MQTTPacket_send(socket, header, data, datalen);	
	free(data);
	return rc;
} 


void MQTTPacket_freeSuback(Suback* pack)
{
	if (pack->qoss != NULL)
		ListFree(pack->qoss);
	free(pack);
}


void* MQTTPacket_unsubscribe(unsigned char aHeader, char* data, int datalen)
{
	Unsubscribe* pack = malloc(sizeof(Unsubscribe));
	char* curdata = data;
	char* enddata = &data[datalen];

	pack->header.byte = aHeader;
	pack->msgId = readInt(&curdata);
	pack->noTopics = 0;
	pack->topics = NULL;
	pack->topics = ListInitialize();
	while (curdata - data < datalen)
	{
		int len;
		char* str = readUTFlen(&curdata, enddata, &len);
		if (str == NULL)
		{
			MQTTPacket_freeUnsubscribe(pack);
			return NULL;
		}
		ListAppend(pack->topics, str, len);
		(pack->noTopics)++;
	}
	return pack;
}


void MQTTPacket_freeUnsubscribe(Unsubscribe* pack)
{
	if (pack->topics != NULL)
		ListFree(pack->topics);
	free(pack);
}


void* MQTTPacket_publish(unsigned char aHeader, char* data, int datalen)
{
	Publish* pack = malloc(sizeof(Publish));
	char* curdata = data;
	char* enddata = &data[datalen];

	pack->header.byte = aHeader;
	if ((pack->topic = readUTF(&curdata, enddata)) == NULL) // Topic name on which to publish
	{
		free(pack);
		return NULL;
	}
	if (pack->header.qos > 0)  // Msgid only exists for QoS 1 or 2
		pack->msgId = readInt(&curdata);
	else
		pack->msgId = 0;
	pack->payload = curdata;
	pack->payloadlen = datalen-(curdata-data);
	return pack;
} // end constructor


void MQTTPacket_freePublish(Publish* pack)
{
	if (pack->topic != NULL)
		free(pack->topic);
	free(pack);
}


int MQTTPacket_send_ack(int type, int msgid, int dup, int socket)
{
	Header header;
	int rc;
	char *buf = malloc(2);
	char *ptr = buf;

	header.byte = 0;
	header.type = type;
	header.dup = dup;
	writeInt(&ptr, msgid);
	if ((rc = MQTTPacket_send(socket, header, buf, 2)) != TCPSOCKET_INTERRUPTED)
		free(buf);
	return rc;
}


int MQTTPacket_send_connack(int aRc, int socket)
{
/*
	char buf[2];
	Header header;

	header.byte = 0;
	header.type = CONNACK;
	buf[0] = (char)0;
	buf[1] = (char)aRc;
	return MQTTPacket_send(socket, header, buf, 2);*/
	return MQTTPacket_send_ack(CONNACK, aRc, 0, socket);
}


int MQTTPacket_send_unsuback(int msgid, int socket)
{
	return MQTTPacket_send_ack(UNSUBACK, msgid, 0, socket);
} 


int MQTTPacket_send_puback(int msgid, int socket)
{
	return MQTTPacket_send_ack(PUBACK, msgid, 0, socket);
}


int MQTTPacket_send_pubrec(int msgid, int socket)
{
	return MQTTPacket_send_ack(PUBREC, msgid, 0, socket);
}


int MQTTPacket_send_pubrel(int msgid, int dup, int socket)
{
	return MQTTPacket_send_ack(PUBREL, msgid, dup, socket);
}


int MQTTPacket_send_pubcomp(int msgid, int socket)
{
	return MQTTPacket_send_ack(PUBCOMP, msgid, 0, socket);
}


void* MQTTPacket_ack(unsigned char aHeader, char* data, int datalen)
{
	Ack* pack = malloc(sizeof(Ack));
	char* curdata = data;

	pack->header.byte = aHeader;
	pack->msgId = readInt(&curdata);
	return pack;
}


int MQTTPacket_send_publish(Publish* pack, int dup, int qos, int retained, int socket)
{
	Header header;
	char *buf, *topiclen;
	int rc = -1;

	topiclen = malloc(2);
	buf = malloc(2);

	header.type = PUBLISH;
	header.dup = dup;
	header.qos = qos;
	header.retain = retained;
	if (qos > 0)
	{
		char *ptr = buf;
		char* bufs[4] = {topiclen, pack->topic, buf, pack->payload};
		int lens[4] = {2, strlen(pack->topic), 2, pack->payloadlen};
		writeInt(&ptr, pack->msgId);
		ptr = topiclen;
		writeInt(&ptr, lens[1]);
		rc = MQTTPacket_sends(socket, header, 4, bufs, lens);
	}
	else
	{
		char* ptr = topiclen;
		char* bufs[3] = {topiclen, pack->topic, pack->payload};
		int lens[3] = {2, strlen(pack->topic), pack->payloadlen};
		writeInt(&ptr, lens[1]);
		rc = MQTTPacket_sends(socket, header, 3, bufs, lens);
	}	
	if (rc != TCPSOCKET_INTERRUPTED)
	{
		free(topiclen);
		free(buf);
	}
	return rc;
}


void MQTTPacket_free_packet(MQTTPacket* pack)
{
	if (pack->header.type == PUBLISH)
		MQTTPacket_freePublish((Publish*)pack);
	else if (pack->header.type == SUBSCRIBE)
		MQTTPacket_freeSubscribe((Subscribe*)pack, 1);
	else if (pack->header.type == UNSUBSCRIBE)
		MQTTPacket_freeUnsubscribe((Unsubscribe*)pack);
	else
		free(pack);
}

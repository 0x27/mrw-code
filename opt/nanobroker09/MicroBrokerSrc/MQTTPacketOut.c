#if !defined(NO_BRIDGE)

#include "MQTTPacketOut.h"

#include <string.h>

#include "Heap.h"

int MQTTPacket_send_connect(Clients* client)
{
	char *buf, *ptr;
	Connect packet;
	int rc, len;

	packet.header.byte = 0;
	packet.header.type = CONNECT;
	packet.header.qos = 1;
	
	len = 12 + strlen(client->clientID)+2;
	if (client->will)
		len += strlen(client->will->topic)+2 + strlen(client->will->msg)+2;

	ptr = buf = malloc(len);
	writeUTF(&ptr, "MQIsdp");
	if (client->noLocal)
		writeChar(&ptr, (char)-125);
	else
		writeChar(&ptr, (char)3);
	packet.flags.will = (client->will) ? 1 : 0;
	packet.flags.cleanstart = client->cleansession;
	packet.flags.willQoS = client->will->qos;
	packet.flags.willRetain = client->will->retained;
	writeChar(&ptr, packet.flags.all);
	writeInt(&ptr, client->keepAliveInterval);
	writeUTF(&ptr, client->clientID);
	if (client->will)
	{
		writeUTF(&ptr, client->will->topic);
		writeUTF(&ptr, client->will->msg);
	}
	rc = MQTTPacket_send(client->socket, packet.header, buf, len);
	free(buf);
	return rc;
}


void* MQTTPacket_connack(unsigned char aHeader, char* data, int datalen)
{
	Connack* pack = malloc(sizeof(Connack));
	char* curdata = data;

	pack->header.byte = aHeader;
	pack->rc = readChar(&curdata);
	return pack;
}


int MQTTPacket_send_pingreq(int socket)
{
	Header header;
	header.byte = 0;
	header.type = PINGREQ;
	return MQTTPacket_send(socket, header, NULL, 0);
}

int MQTTPacket_send_subscribe(List* topics, List* qoss, int msgid, int dup, int socket)
{
	Header header;
	char *data, *ptr;
	int rc = -1;
	ListElement *elem = NULL, *qosElem = NULL;
	int datalen;

	header.type = SUBSCRIBE;
	header.dup = dup;
	header.qos = 1;
	header.retain = 0;

	datalen = 2 + topics->count * 3; // utf length + char qos == 3
	while (ListNextElement(topics, &elem))
		datalen += strlen((char*)(elem->content));
	ptr = data = malloc(datalen);

	writeInt(&ptr, msgid);
	elem = NULL;
	while (ListNextElement(topics, &elem))
	{
		ListNextElement(qoss, &qosElem);
		writeUTF(&ptr, (char*)(elem->content));
		writeChar(&ptr, *(int*)(qosElem->content));
	}
	rc = MQTTPacket_send(socket, header, data, datalen);

	free(data);
	return rc;
}


void* MQTTPacket_suback(unsigned char aHeader, char* data, int datalen)
{
	Suback* pack = malloc(sizeof(Suback));
	char* curdata = data;
	pack->header.byte = aHeader;
	pack->msgId = readInt(&curdata);
	pack->qoss = ListInitialize();
	while (curdata - data < datalen)
	{
		int* newint;
		newint = malloc(sizeof(int));
		*newint = (int)readChar(&curdata);
		ListAppend(pack->qoss, newint, sizeof(int));
	}
	return pack;
}

#endif

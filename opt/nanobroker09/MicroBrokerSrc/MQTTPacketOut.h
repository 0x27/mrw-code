/* (C) Copyright IBM Corp. 2007, 2008
	 Author: Ian Craggs
*/

#if !defined(MQTTPACKETOUT_H)
#define MQTTPACKETOUT_H

#include "MQTTPacket.h"

int MQTTPacket_send_connect(Clients* client);
void* MQTTPacket_connack(unsigned char aHeader, char* data, int datalen);

int MQTTPacket_send_pingreq(int socket);

int MQTTPacket_send_subscribe(List* topics, List* qoss, int msgid, int dup, int socket);
void* MQTTPacket_suback(unsigned char aHeader, char* data, int datalen);

#endif

/* (C) Copyright IBM Corp. 2007, 2008
	 Author: Ian Craggs
*/

#if !defined(MQTTPACKET_H)
#define MQTTPACKET_H

#include "TCPSocket.h"
#include "LinkedList.h"
#include "Clients.h"

typedef unsigned int bool;

#define NANO_PROTOCOL_VERSION 131  // protocol version used for noLocal subscriptions
#define BAD_MQTT_PACKET -2

enum msgTypes
{
	CONNECT = 1, CONNACK, PUBLISH, PUBACK, PUBREC, PUBREL,
	PUBCOMP, SUBSCRIBE, SUBACK, UNSUBSCRIBE, UNSUBACK,
	PINGREQ, PINGRESP, DISCONNECT
};

typedef union
{
	unsigned char byte;
#if defined(REVERSED)
	struct
	{
		unsigned int type : 4;
		bool dup : 1;
		unsigned int qos : 2;
		bool retain : 1;
	};
#else	
	struct
	{
		bool retain : 1;
		unsigned int qos : 2;
		bool dup : 1;
		unsigned int type : 4;
	};
#endif
} Header;

typedef struct 
{
	Header header;
	union
	{
		unsigned char all;
#if defined(REVERSED)
		struct
		{
			int : 2; // unused
			bool willRetain : 1;
			unsigned int willQoS : 2;
			bool will : 1;
			bool cleanstart : 1;
			int : 1; // unused
		};
#else
		struct
		{
			int : 1; // unused
			bool cleanstart : 1;
			bool will : 1;
			unsigned int willQoS : 2;
			bool willRetain : 1;
			int : 2; // unused
		};
#endif
	} flags;

	char *Protocol, *clientID,
        *willTopic, *willMsg;

	int keepAliveTimer;
	unsigned char version;
} Connect;

typedef struct 
{
	Header header;
	char rc; /* connack return code */
} Connack;

typedef struct 
{
	Header header;
} MQTTPacket;

typedef struct 
{
	Header header;
	int msgId;
	List* topics;
	List* qoss;
	int noTopics;
} Subscribe;

typedef struct 
{
	Header header;
	int msgId;
	List* qoss;
} Suback;

typedef struct 
{
	Header header;
	int msgId;
	List* topics;
	int noTopics;
} Unsubscribe;

typedef struct
{
	Header header;
	char* topic;
	int msgId;
	char* payload;
	int payloadlen;
} Publish;


typedef struct
{
	Header header;
	int msgId;
} Ack;

typedef Ack Puback;
typedef Ack Pubrec;
typedef Ack Pubrel;
typedef Ack Pubcomp;
typedef Ack Unsuback;

int MQTTPacket_encode(char* buf, int length);
int MQTTPacket_decode(int socket, int* value);
int readInt(char** pptr);
char* readUTF(char** pptr, char* enddata);
char readChar(char** pptr);
void writeChar(char** pptr, char c);
void writeInt(char** pptr, int anInt);
void writeUTF(char** pptr, char* string);

char* MQTTPacket_name(int ptype);

void* MQTTPacket_Factory(int socket, int* error);
int MQTTPacket_send(int socket, Header header, char* buffer, int buflen);
int MQTTPacket_sends(int socket, Header header, int count, char** buffers, int* buflens);

int MQTTPacket_checkVersion(Connect* pack);
void* MQTTPacket_connect(unsigned char aHeader, char* data, int datalen);
void MQTTPacket_freeConnect(Connect* pack);
int MQTTPacket_send_connack(int aRc, int socket);

void* MQTTPacket_header_only(unsigned char aHeader, char* data, int datalen);
int MQTTPacket_send_pingresp(int socket);
int MQTTPacket_send_disconnect(int socket);

void* MQTTPacket_subscribe(unsigned char aHeader, char* data, int datalen);
void MQTTPacket_freeSubscribe(Subscribe* pack, int all);
int MQTTPacket_send_suback(int aRc, int noOfTopics, int* qoss, int socket);
void MQTTPacket_freeSuback(Suback* pack);

void* MQTTPacket_unsubscribe(unsigned char aHeader, char* data, int datalen);
void MQTTPacket_freeUnsubscribe(Unsubscribe* pack);
int MQTTPacket_send_unsuback(int aRc, int socket);

void* MQTTPacket_publish(unsigned char aHeader, char* data, int datalen);
void MQTTPacket_freePublish(Publish* pack);
int MQTTPacket_send_publish(Publish* pack, int dup, int qos, int retained, int socket);
int MQTTPacket_send_puback(int msgid, int socket);
void* MQTTPacket_ack(unsigned char aHeader, char* data, int datalen);

int MQTTPacket_send_pubrec(int msgid, int socket);
int MQTTPacket_send_pubrel(int msgid, int dup, int socket);
int MQTTPacket_send_pubcomp(int msgid, int socket);

void MQTTPacket_free_packet(MQTTPacket* pack);

#if !defined(NO_BRIDGE)
	#include "MQTTPacketOut.h"
#endif

#endif // MQTTPACKET_H

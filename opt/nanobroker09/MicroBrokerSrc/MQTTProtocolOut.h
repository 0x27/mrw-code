/* (C) Copyright IBM Corp. 2007, 2008
         Author: Ian Craggs
*/

#if !defined(MQTTPROTOCOLOUT_H)
#define MQTTPROTOCOLOUT_H

#include "NanoBroker.h"
#include "LinkedList.h"
#include "SubsEngine.h"
#include "MQTTPacket.h"
#include "Clients.h"
#include "Log.h"

#define DEFAULT_PORT 1883

void MQTTProtocol_reconnect(char* ip_address, Clients* client);
Clients* MQTTProtocol_connect(char* ip_address, char* clientID, int cleansession, int try_nano);
int MQTTProtocol_handlePingresps(void* pack, int sock);
int MQTTProtocol_subscribe(Clients* client, List* topics, List* qoss);
int MQTTProtocol_handleSubacks(void* pack, int sock);
int MQTTProtocol_handleUnsubacks(void* pack, int sock);

#endif

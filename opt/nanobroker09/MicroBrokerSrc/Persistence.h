/* (C) Copyright IBM Corp. 2007, 2008
  Author: Ian Craggs
*/

#if !defined(PERSISTENCE_H)
#define PERSISTENCE_H

#include "NanoBroker.h"
#include <stdio.h>

int Persistence_read_config(char* filename, BrokerStates* s, int config_set);
void Persistence_free_config(BrokerStates* bs);

FILE* Persistence_open_retained(char mode);
void Persistence_write_retained(char* payload, int payloadlen, int qos, char* topicName);
RetainedPublications* Persistence_read_retained();
void Persistence_close_retained();

#endif // PERSISTENCE_H

/* (C) Copyright IBM Corp. 2007, 2008
  Author: Ian Craggs
*/

#if !defined(NANOBROKER_H)
#define NANOBROKER_H

#include "SubsEngine.h"
#include "Bridge.h"

#define true 1
#define false 0

#define assert(x) if (!x) printf("Assertion error %s\n", # x);

typedef struct
{
	int trace_level;
	int connection_messages;
	SubscriptionEngines* se;
	List* clients;
	volatile int toStop;
	int port;
	char* bind_address;
	int max_inflight_messages; /* per client, outbound */
	int max_queued_messages; /* per client, outbound */
	int retry_interval;
	char* persistence_location;
	int retained_persistence;
	List* clientid_prefixes;
	Bridges bridge;
} BrokerStates;

#endif

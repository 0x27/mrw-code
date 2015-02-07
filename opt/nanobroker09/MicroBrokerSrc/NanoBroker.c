/*
	(C) Copyright IBM Corp. 2007, 2008
	Author: Ian Craggs
*/

#include <signal.h>
#include <stdlib.h>

#include "NanoBroker.h"
#include "Log.h"
#include "MQTTProtocol.h"
#include "TCPSocket.h"
#include "SubsEngine.h"
#include "Persistence.h"
#include "Heap.h"
#include "Messages.h"

int Broker_startup();
void Broker_shutdown(int);


static BrokerStates BrokerState = {LOG_INFO, 1, NULL, NULL, 0, 1883, NULL, 10, 100, 20, NULL, 0, NULL, { NULL }};
char* config = "nanobroker.cfg";
int config_set = 0;


void finish(int sig)
{
	Log(LOG_INFO, "Nanobroker stopping\n");
	BrokerState.toStop = 1;
}


int main(int argc, char* argv[])
{
	int rc = 0;
	char version[] = 
		"0.90"; // __VERSION__

	Log(LOG_INFO, "Nanobroker - %s (C) Copyright IBM Corp. 2007, 2008\n",
#if defined(NO_BRIDGE)
	"bridgeless"
#else
	"with bridge"
#endif
	);
	Log(LOG_INFO, "Version %s, %s\n", version,
		"Thu Apr 17 13:56:33 2008"); // __TIMESTAMP__
	Log(LOG_INFO, "Author: Ian Craggs (icraggs@uk.ibm.com)\n");

	if (argc > 1)
	{
		config = argv[1];
		Log(LOG_INFO, "Configuration file name is %s\n", config);
		config_set = 1;
	}
	
	rc = Broker_startup();
	SubscriptionEngines_setRetained(BrokerState.se, "$SYS/broker/version", 0, version, strlen(version));
	if (rc == 0)
		while (!BrokerState.toStop)
		{
			MQTTProtocol_timeslice();
			#if !defined(NO_BRIDGE)
				Bridge_timeslice(&BrokerState.bridge);
			#endif
		}
	Broker_shutdown(rc);

	Log(LOG_INFO, "Nanobroker stopped\n");

	return rc;
}


int Broker_startup()
{
	int rc;

	signal(SIGINT, finish);
	signal(SIGTERM, finish);
	Heap_initialize();
	if ((rc = Messages_initialize()) == 0)
	{
		Persistence_read_config(config, &BrokerState, config_set);
		BrokerState.clients = ListInitialize();
		BrokerState.se = SubscriptionEngines_initialize();
		rc = MQTTProtocol_initialize(&BrokerState);
#if !defined(NO_BRIDGE)
		Bridge_initialize(&(BrokerState.bridge), BrokerState.se);
#endif
		Log_setPublish(true);
		Log_setlevel(BrokerState.trace_level);
	}
	return rc;
}


void Broker_shutdown(rc)
{
	if (rc != -99)
	{
		Log_setPublish(false);
#if !defined(NO_BRIDGE)
		Bridge_terminate(&(BrokerState.bridge));
#endif
		MQTTProtocol_terminate();
		SubscriptionEngines_terminate(BrokerState.se);
		ListFree(BrokerState.clients);
		Persistence_free_config(&BrokerState);
		Messages_terminate();
	}
	Heap_terminate();
}

/*
 
 stdout subscriber
 
 compulsory parameters:
 
  --topic topic to subscribe to
 
 defaulted parameters:
 
	--host localhost
	--port 1883
	--qos 2
	--delimiter \n
	--clientid stdout_subscriber
	
	--userid none
	--password none
 
*/

#include "MQTTClient.h"
#include "MQTTClientPersistence.h"
#include "Thread.h"

#include <stdio.h>
#include <signal.h>


#if defined(WIN32)
#include <Windows.h>
#include <memory.h>
#define sleep Sleep
#else
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#endif


volatile int toStop = 0;
volatile int completed_threads = 0;
int failures = 0;
char url[100];
char* topic = NULL;



void usage()
{
	printf("MQTT multiple stdout subscriber\n");
	printf("Usage: stdoutsub topicname <options>, where options are:\n");
	printf("  --host <hostname> (default is localhost)\n");
	printf("  --port <port> (default is 1883)\n");
	printf("  --qos <qos> (default is 2)\n");
	printf("  --delimiter <delim> (default is \n)");
	printf("  --clientid <clientid> (default is hostname+timestamp)\n");
	printf("  --username none\n");
	printf("  --password none\n");
  printf("  --showtopics <on or off> (default is on if the topic has a wildcard, else off)\n");
	exit(-1);
}

static mutex_type cmutex;
static mutex_type dmutex;
static mutex_type mmutex;

static mutex_type main_mutex;


void connectionLost(void* context, char* cause)
{
	Thread_lock_mutex(cmutex);
	printf("thread %d callback -> connectionLost\n", *(int*)context);
	Thread_unlock_mutex(cmutex);
}

void deliveryComplete(void* context, MQTTClient_deliveryToken token)
{
	Thread_lock_mutex(dmutex);
	printf("thread %d callback -> publish complete for token %d\n", *(int*)context, token);
	Thread_unlock_mutex(dmutex);
}

int messageArrived(void* context, char* topicName, int topicLen, MQTTClient_message* m)
{
	Thread_lock_mutex(mmutex);
	printf("thread %d callback -> msgId = %5d for message '%.*s' received on topic '%s'\n", *(int*)context, m->msgid, m->payloadlen, (char*)(m->payload), topicName);
	free(topicName);
	MQTTClient_freeMessage(&m);
	Thread_unlock_mutex(mmutex);
	return 0;
}


void cfinish(int sig)
{
	signal(SIGINT, NULL);
	toStop = 1;
}


struct
{
	char* clientid;
	char delimiter;
	int qos;
	char* username;
	char* password;
	char* host;
	char* port;
  int showtopics;
	int threads;
} opts =
{
	"stdout-subscriber", '\n', 2, NULL, NULL, "localhost", "1883", 0, 2
};

void getopts(int argc, char** argv);
thread_return_type client_thread(void* parm);

int main(int argc, char** argv)
{
	int i = 0;
	
	if (argc < 2)
		usage();
		
	signal(SIGINT, cfinish);
	signal(SIGTERM, cfinish);
	
	cmutex = Thread_create_mutex();
	dmutex = Thread_create_mutex();
	mmutex = Thread_create_mutex();
	main_mutex = Thread_create_mutex();
	
	topic = argv[1];
	printf("topic is %s\n", topic);

  if (strchr(topic, '#') || strchr(topic, '+'))
		opts.showtopics = 1;

	getopts(argc, argv);	
	sprintf(url, "%s:%s", opts.host, opts.port);
	
	for (i = 0; i < opts.threads; ++i) 
		Thread_start(client_thread, &url[i]);

	while (completed_threads != opts.threads)
		MQTTClient_sleep(100);
		
	printf("Exiting main thread ...\n");
	
	Thread_destroy_mutex(cmutex);
	Thread_destroy_mutex(dmutex);
	Thread_destroy_mutex(mmutex);
	Thread_destroy_mutex(main_mutex);

	if (failures == 0)
		printf("\nTEST SUCCEEDED\n");
	else
		printf("\nTEST FAILED\n");

	return failures;
}
	
	
thread_return_type client_thread(void* parm)
{
	MQTTClient client;
	MQTTClient_connectOptions conn_opts;
	char clientid[24];
	int rc = 0;
	int mycount = (char*)parm - &url[0];
	
	printf("Thead %d starting\n", mycount);

	sprintf(clientid, "%s%d", opts.clientid, mycount);
	rc = MQTTClient_create(&client, url, clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	printf("Thread %d, create rc %d\n", mycount, rc);
	
	MQTTClient_setCallbacks(client, &mycount, connectionLost, messageArrived, deliveryComplete);
	printf("Thread %d, setCallbacks rc %d\n", mycount, rc);

	memset(&conn_opts, '\0', sizeof(conn_opts));
	conn_opts.keepAliveInterval = 20;
	conn_opts.reliable = 1;
	conn_opts.cleansession = 1;
	conn_opts.username = opts.username;
	conn_opts.password = opts.password;
	
	printf("Thread %d, connecting...\n", mycount);
	rc = MQTTClient_connect(client, &conn_opts);
	printf("Thread %d, connect rc %d\n", mycount, rc);
	if (rc != MQTTCLIENT_SUCCESS)
		goto exit;
	
	rc = MQTTClient_subscribe(client, topic, opts.qos);
	printf("Thread %d, subscribe rc %d\n", mycount, rc);

	while (!toStop)
	{
		char buffer[30] = "random crap";
		int data_len = strlen(buffer) + 1;
				rc = MQTTClient_publish(client, topic, data_len, buffer, opts.qos, 0 /*opts.retained*/, NULL);
		printf("Thread %d, publish rc %d\n", mycount, rc);
		if (!toStop)
			MQTTClient_sleep(1000);
	}
	
	printf("Thread %d stopping\n", mycount);

exit:
	MQTTClient_disconnect(client, 0);

 	MQTTClient_destroy(&client);
	
	Thread_lock_mutex(main_mutex);
	++completed_threads;
	Thread_unlock_mutex(main_mutex);
	
	printf("Thread %d completed\n", mycount);

	return 0;
}

void getopts(int argc, char** argv)
{
	int count = 2;
	
	while (count < argc)
	{
		if (strcmp(argv[count], "--qos") == 0)
		{
			if (++count < argc)
			{
				if (strcmp(argv[count], "0") == 0)
					opts.qos = 0;
				else if (strcmp(argv[count], "1") == 0)
					opts.qos = 1;
				else if (strcmp(argv[count], "2") == 0)
					opts.qos = 2;
				else
					usage();
			}
			else
				usage();
		}
		else if (strcmp(argv[count], "--host") == 0)
		{
			if (++count < argc)
				opts.host = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--port") == 0)
		{
			if (++count < argc)
				opts.port = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--threads") == 0)
		{
			if (++count < argc)
				opts.threads = atoi(argv[count]);
			else
				usage();
		}
		else if (strcmp(argv[count], "--clientid") == 0)
		{
			if (++count < argc)
				opts.clientid = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--username") == 0)
		{
			if (++count < argc)
				opts.username = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--password") == 0)
		{
			if (++count < argc)
				opts.password = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--delimiter") == 0)
		{
			if (++count < argc)
				opts.delimiter = argv[count][0];
			else
				usage();
		}
		else if (strcmp(argv[count], "--showtopics") == 0)
		{
			if (++count < argc)
			{
				if (strcmp(argv[count], "on") == 0)
					opts.showtopics = 1;
				else if (strcmp(argv[count], "off") == 0)
					opts.showtopics = 0;
				else
					usage();
			}
			else
				usage();
		}
		count++;
	}
	
}

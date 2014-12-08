/* (C) Copyright IBM Corp. 2007, 2008
	 Author: Ian Craggs
*/

#include "Clients.h"

#include <string.h>
#include <stdio.h>


int clientIDCompare(void* a, void* b)
{
	Clients* client = (Clients*)a;
	//printf("comparing clientdIDs %s with %s\n", client->clientID, (char*)b);
	return strcmp(client->clientID, (char*)b) == 0;
}


int clientSocketCompare(void* a, void* b)
{
	Clients* client = (Clients*)a;
	//printf("comparing %d with %d\n", (char*)a, (char*)b);
	return client->socket == *(int*)b;
}


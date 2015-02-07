/* (C) Copyright IBM Corp. 2008
	 Author: Ian Craggs
*/

#include "Messages.h"
#include "Heap.h"
#include "Log.h"

#include <stdio.h>
#include <memory.h>
#include <string.h>

static char* message_list[MESSAGE_COUNT];

#define max_msg_len 120

int Messages_initialize()
{
  FILE* rfile = NULL;
	char buf[max_msg_len];
	int count = 0;
	int rc = -99;

	if ((rfile = fopen("Messages.txt", "r")) == NULL)
	  Log(LOG_ERROR, "Could not open message file Messages.txt\n");
	else
	{
		char* msg;
		memset(message_list, '\0', sizeof(message_list));
		while (fgets(buf, max_msg_len, rfile) != NULL && count < MESSAGE_COUNT)
		{
			msg = (char*)malloc(strlen(buf)+1);
			strcpy(msg, buf);
			//printf("adding %s", msg);
			message_list[count++] = msg;
		}
		fclose(rfile);
		rc = 0;
	}
	return rc;
}


char* Messages_get(int index)
{
  return message_list[index];
}


void Messages_terminate()
{
	int i;
	for (i = 0; i < MESSAGE_COUNT; ++i)
	{
		if (message_list[i])
			free(message_list[i]);
	}
}

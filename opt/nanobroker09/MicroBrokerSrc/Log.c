/* (C) Copyright IBM Corp. 2007, 2008
	 Author: Ian Craggs
*/

#include "Log.h"
#include "MQTTPacket.h"
#include "MQTTProtocol.h"
#include "Messages.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

static int minloglevel = LOG_INFO;
static int Log_publishFlag = 0;
static int Log_recurse_flag = 0;


void Log_setlevel(int minlevel)
{
	minloglevel = minlevel;
}


void Log_setPublish(int flag)
{
	Log_publishFlag = flag;
}


void Log_Publish(char* topic, char* string)
{
	Publish publish;
	publish.header.byte = 0;
	publish.header.retain = 1;
	publish.payload = string;
	publish.payloadlen = strlen(string)-1; /* omit newline at end */
	publish.topic = topic;
	MQTTProtocol_handlePublishes(&publish, 0);
}


void Log(int log_level, char* format, ...)
{
	if (log_level >= minloglevel && Log_recurse_flag == 0)
	{
		va_list args;
		time_t rawtime;
		struct tm *timeinfo;
		static char buffer[512];
		int off = 16;

		time(&rawtime);
		timeinfo = localtime(&rawtime);
		strftime(buffer, 80, "%Y%m%d %H%M%S ", timeinfo);

		if (log_level == LOG_ERROR)
		{
			strcpy(&buffer[off], "Error: ");
			off += 7;
		}
		else if (log_level == LOG_WARNING)
		{
			strcpy(&buffer[off], "Warning: ");
			off += 9;
		}

		va_start(args, format);
		vsprintf(&buffer[off], format, args);
		va_end(args);

		printf(buffer);
		fflush(stdout);
		if (log_level > LOG_DEBUG && Log_publishFlag)
		{
			Log_recurse_flag = 1;
			Log_Publish("$SYS/broker/log", buffer);
			Log_recurse_flag = 0;
		}
	}
}

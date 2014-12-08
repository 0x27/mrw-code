/* (C) Copyright IBM Corp. 2007
    Author: Ian Craggs               
*/

#if !defined(LOG_H)
#define LOG_H

#define LOG_DEBUG 10
#define LOG_PROTOCOL 20
#define LOG_INFO 30
#define LOG_WARNING 40
#define LOG_ERROR 50

void Log_setlevel(int);
void Log_setPublish(int flag);

void Log(int, char *, ...);

#endif

/* (C) Copyright IBM Corp. 2008
    Author: Ian Craggs               
*/

#if !defined(MESSAGES_H)
#define MESSAGES_H

#define MESSAGE_COUNT 137

int Messages_initialize();
void Messages_terminate();

char* Messages_get(int);

#endif

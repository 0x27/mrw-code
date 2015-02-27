#include <Arduino.h>

#ifndef ByeByeStandby_h
#define ByeByeStandby_h

// Handles a BBSB command, sending the correct bytes using the RFM12B OOK pulse mechanism 
void handleBBSBCommand(byte* command);

#endif


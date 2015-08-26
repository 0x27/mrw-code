#include <Arduino.h>

#ifndef ByeByeStandby_h
#define ByeByeStandby_h

/*
 * Handles a BBSB command, sending the correct bytes using the RFM12B OOK pulse mechanism 
 *
 * Can be used with different RF chips. To do this, the methods for setting the OOK transmitter HIGH (usually TX ON)
 * and LOW (usually TX off) for a particular chip are passed in.
 *
 * Param 1 = the method used to set the chip HIGH
 * Param 2 = the method used to set the chip LOW
 * Param 3 = if the methods for setting HIGH and LOW are over an SPI interface, extra delays are included
 *           in the OOK timings to take into account the fact that the SPI interface incurs a delay
 * Param 4 = a colon-terminated string (using a proprietary format) denoting the command to send, e.g. "BBSB 1 A 0:"
 *           to turn socket 1 in home group A off.
 */
void handleBBSBCommand(void (*methodToSetChipHigh)(), void (*methodToSetChipLow)(), bool includeSPIInterfaceDelays, byte* command);

#endif


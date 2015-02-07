/*  
 *  This is a library which allows you to control BBSB sockets by sending
 *  commands to the arduino over the serial connection. The sketch also loops through
 *  a few standard commands on & off which is also useful for testing.
 */

/*  
 *  The high-level encoding this sketch uses is a very basic BBSB 2-byte protocol. 
 *  It would be useful/interesting to see if this 2-byte protocol also works with
 *  KaKu, Dormia, and HomeEasy sockets, as well as more modern BBSB sockets. Once
 *  a few of them have been tried I'll add more comments here.
 *
 *  The actual bit values in the 2 bytes are as follows:
 *
 *  Byte 1:  [] [] [] [] [Socket on (1) or off (0)] [unknown] [unknown] []
 *
 *  Byte 2:  [] [] [Socket ID] [Socket ID] [] [HomeGroup ID] [HomeGroup ID] [HomeGroup ID]
 *
 *  e.g. to turn socket A4 on (under group 1)  = 00001110 00110000
 *  e.g. to turn socket B2 off (under group 1) = 00000110 00010001
 *  e.g. to turn socket H3 off (under group 1) = 00000110 00100111
 *
 *  The actual binary encoding over the air is exactly the same as the KlikOnKlikOff
 *  protocol as used in the kakuSend demo sketch. That is to say:
 *  
 *    - Each bit is preceded by a short (375ms) low signal followed by a long (1125ms) high signal
 *    - The actual bit is then sent as: - long high, short low (to indicate a 1)
 *                                      - short high, long low (to indicate a 0)
 *
 *  This should work with KaKu sockets since it's the same as the KaKu demo send sketch. Again it
 *  would be interesting to see if this pattern works with HomeEasy or Dormia sockets (if I ever
 *  come by some of those)
 */
 
/*  
 *  Serial commands for us to send out should be as a string in the form:
 *
 *  [protocol type string] [space] [socket ID int, starting at 1] [space] [home group ID letter] [space] [on/off int 1/0] e.g.
 *  
 *     "BBSB 4 B 1" = BBSB, socket 4, home group B, ON
 *  
 *  (Specifically the first socket int is 1, not 0, and the same for the home group ID)
 */ 

#include <Arduino.h>
#include <ByeByeStandby.h>
#include <JeeLib.h>
#include <util/parity.h>

// Turn transmitter on or off, but also apply asymmetric correction and account
// for 25 us SPI overhead to end up with the proper on-the-air pulse widths.
// With thanks to JGJ Veken for his help in getting these values right.
static void ookPulse(int on, int off) {
    rf12_onOff(1);
    delayMicroseconds(on + 150);
    rf12_onOff(0);
    delayMicroseconds(off - 200);
}

// Define the 4 byte codes we'll use at the beginning of Serial
// port commands to indicate what protocol is to be used
#define BBSB "BBSB"
#define KAKU "KAKU"
#define HOME "HOME"
#define DORM "DORM"

// Define the on/off timings used to send 0s and 1s
#define ON 1200
#define OFF 500
#define TOTAL 1700
 
static void OOKSend(unsigned long cmd) {
	// Send the command a few times just to ensure it's received correctly
	for (byte i = 0; i < 5; ++i) {
		for (byte bit = 0; bit < 12; ++bit) {
			ookPulse(OFF, ON);
			int nextBit = bitRead(cmd, bit);
			int on = nextBit ? ON : OFF;
			ookPulse(on, TOTAL - on);
		}

		// Finish the transmission with a short-long pair
		ookPulse(OFF, ON);
		delay(4);
	}
}

void handleBBSBCommand(byte* command) {
  // First 4 bytes indicate the command type (e.g. BBSB, KAKU, HOME, DORM)
  if (strncmp((char*)command, BBSB, 4) == 0) {
    // We're doing BBSB stuff
    Serial.println("INFO: Handling BBSB command");
    
    // We can now extract the socket, home group, and on/off command ints (we need to
    // convert from the ascii number e.g. '3' to the actual int 3, so we subtract the 
    // ascii code for '0', and we subtract the code for 'A' to get the int for the home group
    byte socketID = command[5] - '0';
    byte homeGroupID = command[7] - 'A';
    byte onOff = command[9] - '0';
    
    // Then, because we want the first socket and home group IDs to start at 1, not 0,
    // we subtract 1 from each of those values before they're encoded into the BBSB command
    // we send
    socketID--;
    
    // Do basic range checking in case we happened to read some garbage
    if (socketID <= 30 && homeGroupID <= 30 && socketID >= 0 && homeGroupID >= 0)
    {
      if (onOff == 0 || onOff == 1) {
        Serial.print("INFO: Socket ID: ");
        Serial.print(socketID);
        Serial.print(" HomeGroup: ");
        Serial.print(homeGroupID);
        Serial.print(" On/Off: ");
        Serial.println(onOff);
        // Construct the correct 2-byte BBSB command to send (as a 2-byte int) by ORing together the following values:
        // the home group ID; socket ID; on/off flag; together with (1024+512) (don't quite know what that's for yet)
        int theCommand = (homeGroupID & 7) | ((socketID & 7) << 4) | (onOff << 11) | (1024 + 512);
        //OOKSend(0b 00000110 00110000);
        OOKSend(theCommand);
      } else {
        Serial.println("INFO: The on/off value was out of range. Valid values are 0 or 1 (as chars, e.g. 48 or 49)");
      }
    } else {
      Serial.println("INFO: The socket or home group IDs were out of range. The command is invalid.");
    }
  } else {
    Serial.println("INFO: Unrecognised serial command received (and ignored)");
  }
}


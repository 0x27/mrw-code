/*  
 *  This is a sketch which allows you to control BBSB sockets by sending
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
 
#include <JeeLib.h>
#include <util/parity.h>
#include <ByeByeStandby.h>

// A buffer to store commands received over the serial port
char command[10];
 
void setup() {
    
    // Initialise the RFM12B chip to 433MHZ
    rf12_initialize(0, RF12_433MHZ);
    
    // Optionally set the specific frequency to operate on. This doesn't appear to be
    // necessary even though BBSB runs on 433.92mhz and the default is 434mhz
    rf12_control(0xA620); // A68A    433.92mhz (A620) or 434mhz (AC40)
    
    //rf12_control(0x9489); // VDI; FAST;200khz;GAIN -6db ;DRSSI 97dbm 
    
    // Set the data rate. Not really sure if you definitely have to set this for the sketch to work, 
    // but seems to be OK without.
    rf12_control(0xc647); // C691    c691 datarate 2395 kbps 0xc647 = 4.8kbps 
}

void loop() {
  // Send A4 ON
  strcpy(command, "BBSB 1 A 0:");
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  
  delay(2500);
  
  // Send A4 OFF
  strcpy(command, "BBSB 1 A 1:");
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  handleBBSBCommand((byte *)command);
  
  delay(2500);
  
  // Send A4 ON
  //strcpy(command, "BBSB 4 A 0:");
  //handleBBSBCommand((byte *)command);
  
  delay(2500);
  
  // Send A4 OFF
  //strcpy(command, "BBSB 4 A 1:");
  //handleBBSBCommand((byte *)command);
  
  delay(2500);
  
  // Send A4 ON
  //strcpy(command, "BBSB 2 A 0:");
  //handleBBSBCommand((byte *)command);
  
  delay(2500);
  
  // Send A4 OFF
  //strcpy(command, "BBSB 2 A 1:");
  //handleBBSBCommand((byte *)command);
  
  delay(2500);
}

/// @dir RF12demo
/// Configure some values in EEPROM for easy config of the RF12 later on.
// 2009-05-06 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

// this version adds flash memory support, 2009-11-19

#include <JeeLib.h>
#include <util/crc16.h>
#include <util/parity.h>
#include <ByeByeStandby.h>

#define SERIAL_BAUD 57600

#define LED_PIN     9   // activity LED, comment out to disable

#define COLLECT 0x20 // collect mode, i.e. pass incoming without sending acks

#define ENDOFSERIALCOMMAND :

static unsigned long now () {
    // FIXME 49-day overflow
    return millis() / 1000;
}
#define LED_PIN 13
static void activityLed (byte on) {
#ifdef LED_PIN
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, !on);
#endif
}

byte groupId;

static char cmd;
static byte value, stack[RF12_MAXDATA], top, sendLen, dest, quiet;
static byte testbuf[RF12_MAXDATA], testCounter;

// Variables used for handling incoming commands over Serial
int cursor = 0;
byte command[100] = {0};

void setup() {
    Serial.begin(SERIAL_BAUD);
    Serial.println("INFO: rf12_hub_receiver started");
    
    groupId = 0x35;
    
    // Initialise as node 1, 433mHz, group 53
    rf12_initialize(0x41, RF12_433MHZ, groupId);
}

void loop() {

    if (rf12_recvDone()) {
        byte n = rf12_len;
        if (rf12_crc == 0) {
            Serial.print("DATA: ");
            
            Serial.print("GROUP(");
            Serial.print(rf12_grp);
        
            Serial.print(") HEADER(");
            Serial.print((int) rf12_hdr);
        
            Serial.print(") BYTES(");
            Serial.print((int) rf12_len);
            Serial.print(") ");
        
            for (byte i = 0; i < n; ++i) {
               Serial.print((char) rf12_data[i]);
            }
            
            Serial.println();
            
            activityLed(1);
            
            if (RF12_WANTS_ACK) {
                Serial.println("INFO: Replying with ack");
                rf12_sendStart(RF12_ACK_REPLY, 0, 0);
            }
            
            activityLed(0);
        } else {
            Serial.print("RANDOMBYTES: ");
            
            // print at most 60 bytes if crc is wrong
            if (n > 200) n = 200;
            
            for (byte i = 0; i < n; ++i) {
              Serial.print((char) rf12_data[i]);
            }
            Serial.println();
        }
    } else if (Serial.available() > 0) {
      // This allows us to accept commands to do something in particular
      // Currently this is used to accept requests to send a ByeByeStandby command to a BBSB socket, 
      // e.g. "BBSB 3 H 1" means BBSB command, turn socket H 3 ON. Additionall 'handleXXXXCommand()'
      // methods will be added as we need to handle different types of commands
      cursor = 0;
      unsigned long startTime = millis();
      byte commandTimedOut = 0;
      
      // Handle the command
      while (cursor < sizeof(command)) {
        command[cursor++] = Serial.read();
        
        // If we see our end-of-command marker, break out of the loop
        if (command[cursor-1] == ':') break;
        
        // Also, if we've been waiting for the end of the command for 
        // greater than 30 seconds, break out of it anyway. First check
        // if we happen to have hit the 50 day rollover of the millis()
        // function...
        if (millis() < startTime) {
          // We happened to start reading bytes just before the millis() timer rolled over.
          // Easiest way to handle this is to reset the startTime, which means we'll wait
          // a bit more than 30 seconds but that's not a big deal
          startTime = millis();
        } else {
          // If we timed out waiting for the ':', we'll completely ignore the command
          if (millis() - startTime > 30000) {
            commandTimedOut = 1;
            break;
          }
        }
      }
      
      if (commandTimedOut) {
        Serial.print("INFO: Timed out receiving serial command (ignoring it) -> ");
        Serial.println((char *)command);
      } else {
        Serial.print("INFO: Serial command received -> ");
        Serial.println((char *)command);
      
        // The RFM12B chip needs re-initialising to node value 0 for On-Off keying
        rf12_initialize(0, RF12_433MHZ);
      
        // This will take the command we've received, check if it's a BBSB request, and handle it if it is
        handleBBSBCommand(command);
      
        // Re-initialise the RFM12B chip back to the values we need
        rf12_initialize(0x41, RF12_433MHZ, groupId);
      }
    }
    
    delay(100);
}

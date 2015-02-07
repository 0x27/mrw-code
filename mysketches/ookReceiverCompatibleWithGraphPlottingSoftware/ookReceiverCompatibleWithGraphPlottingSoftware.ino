/// @dir ookRelay2
/// Generalized decoder and relay for 868 MHz and 433 MHz OOK signals.
// 2010-04-11 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <JeeLib.h>
#include "decoders.h"

#define DEBUG 1     // set to 1 to also report results on the serial port
//#define DEBUG_LED // define as pin 1..19 to blink LED on each pin change
#define NODO 0      // use the Nodo shield hardware (only 433 MHz receiver)

// RF12 communication settings
#define NODEID 19
#define NETGRP 5

// I/O pin allocations, leave any of these undefined to omit the code for it
#if NODO
#define PIN_433 2     // D.2 = 433 MHz receiver
#define POWER_433 12  // must be 1 to supply power to the receiver
#define DEBUG_LED 13  // std Arduino led is also red status led
#else
#define USE_RF12 1    // only set if the RFM12B hardware is present
#define PIN_433 14    // AIO4 = 433 MHz receiver
#endif

const void byteToBinary(char* byteString, int x)
{
    byteString[0] = '\0';
    int z;
    for (z = 128; z > 0; z >>= 1) strcat(byteString, ((x & z) == z) ? "1" : "0");
}

#if PIN_433
//OregonDecoder orsc;
//CrestaDecoder cres;
KakuDecoder kaku;
KakuADecoder kakuA; //WvD
//XrfDecoder xrf;
//HezDecoder hez;
//FlamingoDecoder fmgo;
//SmokeDecoder smk;
//ByronbellDecoder byr;
//ElroDecoder elro;

DecoderInfo di_433[] = {
    //{ 5, "ORSC", &orsc },
    //{ 6, "CRES", &cres },
    { 7, "KAKU", &kaku },
    //{ 8, "XRF", &xrf },
    //{ 9, "HEZ", &hez },
    //{ 10, "ELRO", &elro },
    //{ 11, "FMGO", &fmgo },
    //{ 12, "SMK", &smk },
    //{ 13, "BYR", &byr },
    { 14, "KAKUA", &kakuA },
    { -1, 0, 0 }
};

byte sendPacket = 0;

// State to track pulse durations measured in the interrupt code
volatile word pulse_433;
word last_433; // never accessed outside ISR's

#if PIN_433 >= 14
#define VECT PCINT1_vect
#elif PIN_433 >= 8
#define VECT PCINT0_vect
#else
#define VECT PCINT2_vect
#endif

ISR(VECT) {
    word now = micros();
    pulse_433 = now - last_433;
    last_433 = now;
}
#endif

#define PULSEARRAYSIZE 200

// Outgoing data buffer for RF12
byte packetBuffer [RF12_MAXDATA], packetFill;
long totalTime = 0;
byte state = 0;

byte bits[BITARRAYSIZE];
long bitTimestamps[BITARRAYSIZE];
byte bitIndex = 0;

unsigned long pulses[PULSEARRAYSIZE];
int pulseIndex = 0;

// Timer to only relay packets up to 10x per second, even if more come in.
MilliTimer sendTimer;

static void setupPinChangeInterrupt () {
#if PIN_433
    pinMode(PIN_433, INPUT);
    digitalWrite(PIN_433, 1);   // pull-up

    // interrupt on pin change
#if PIN_433 >= 14
    bitSet(PCMSK1, PIN_433 - 14);
    bitSet(PCICR, PCIE1);
#elif PIN_433 >= 8
    bitSet(PCMSK0, PIN_433 - 8);
    bitSet(PCICR, PCIE0);
#else
    bitSet(PCMSK2, PIN_433);
    bitSet(PCICR, PCIE2);
#endif
#endif
}

// Append a new data item to the outgoing packet buffer (if there is room
static void addToBuffer (byte code, const char* name, const byte* buf, byte len) {
  
  byte bbsbMatch = 0;
  byte bbsbOn = 0;
  char byteString[9];

#if DEBUG
    Serial.print("OTHER ");
    Serial.print(name);
    for (byte i = 0; i < len; ++i) {
        Serial.print(' ');
        Serial.print((int) buf[i]);
    }
    Serial.print("     ");
    for (byte i = 0; i < len; ++i) {
        Serial.print(' ');
        byteToBinary(byteString, (int) buf[i]);
        Serial.print(byteString);
    }
    Serial.println();
    //Serial.print(' ');
    //Serial.print(millis() / 1000);
    //Serial.println();*/
#endif

    if (!sendPacket) {
       if (packetFill + len < sizeof packetBuffer) {
           packetBuffer[packetFill++] = code + (len << 4);
           memcpy(packetBuffer + packetFill, buf, len);
           packetFill += len;
          } else {
#if DEBUG
           Serial.print(" dropped: ");
           Serial.print(name);
           Serial.print(", ");
           Serial.print((int) len);
           Serial.println(" bytes");
#endif        
       }
    }
}

static void addDecodedData (DecoderInfo& di) {
    byte size;
    const byte* data = di.decoder->getData(size);
    addToBuffer(di.typecode, di.name, data, size);
    di.decoder->resetDecoder();
}

// Check for a new pulse and run the corresponding decoders for it
static void runPulseDecoders (DecoderInfo* pdi, volatile word& pulse) {
    // get next pulse with and reset it - need to protect against interrupts
    cli();
    word p = pulse;
    pulse = 0;
    sei();

    // if we had a pulse, go through each of the decoders
    if (p != 0) { 
      totalTime = totalTime + p;
      
      // Removing this makes things work - not sure why - need to put it
      // back in but not sure why it breaks thins...
      pulses[pulseIndex] = totalTime;
      pulseIndex++;
      //if (pulseIndex >= PULSEARRAYSIZE) pulseIndex = 0;
      
      // If we've captured a load of pulses but we haven't yet written them
      // out (because they only usually get dumped out when we finish decoding
      // a recognised sequence of bytes) we need to dump them now.  
      if (pulseIndex >= PULSEARRAYSIZE) {
        for (int index = 0; index < pulseIndex; index = index + 1) {
           if (index > 0) {
             state = (state + 1) % 2;
             Serial.print("RAW ");
             Serial.print(pulses[index-1]);
             Serial.print(" ");
             Serial.println(state);
             Serial.print("RAW ");
             Serial.print(pulses[index]);
             Serial.print(" ");
             Serial.println(state);
             Serial.print("MISC ");
             Serial.print(pulses[index] - pulses[index-1]);
             Serial.println();
           }
        }
        pulseIndex = 0;
      }
 
#if DEBUG_LED
        digitalWrite(DEBUG_LED, 1);
        
#endif
        while (pdi->typecode >= 0) {
            if (pdi->decoder->nextPulse(p, totalTime, bits, bitTimestamps, &bitIndex)) {
                // A complete set of pulses has come to an end
                // and been decoded successfully
                addDecodedData(*pdi);
                
                for (int index = 0; index < bitIndex; index++) {
                  if (bits[index]) {
                    Serial.print("BIT ");
                    Serial.print(bitTimestamps[index] - 51);
                    Serial.print(" ");
                    Serial.print(0);
                    Serial.println(" KAKU");
                    Serial.print("BIT ");
                    Serial.print(bitTimestamps[index] - 50);
                    Serial.print(" ");
                    Serial.print(bits[index]);
                    Serial.println(" KAKU");
                    Serial.print("BIT ");
                    Serial.print(bitTimestamps[index] + 50);
                    Serial.print(" ");
                    Serial.print(bits[index]);
                    Serial.println(" KAKU");
                    Serial.print("BIT ");
                    Serial.print(bitTimestamps[index] + 51);
                    Serial.print(" ");
                    Serial.print(0);
                    Serial.println(" KAKU");
                  }
                }
                bitIndex = 0;
                
                // If we've captured a known bit/byte sequence, now's a good time
                // to dump the raw pulses that were used to construct the sequence
                for (int index = 0; index < pulseIndex; index = index + 1) {
                  if (index > 0) {
                    state = (state + 1) % 2;
                    Serial.print("RAW ");
                    Serial.print(pulses[index-1]);
                    Serial.print(" ");
                    Serial.println(state);
                    Serial.print("RAW ");
                    Serial.print(pulses[index]);
                    Serial.print(" ");
                    Serial.println(state);
                    Serial.print("MISC ");
                    Serial.print(pulses[index] - pulses[index-1]);
                    Serial.println();
                  }
                }
                pulseIndex = 0;
            }
            ++pdi;
        }
#if DEBUG_LED
        digitalWrite(DEBUG_LED, 0);
#endif
    }
}

// see http://jeelabs.org/2011/01/27/ook-reception-with-rfm12b-2/
static void rf12_init_OOK () {
    rf12_initialize(0, RF12_868MHZ);

    rf12_control(0x8017); // 8027    868 Mhz;disabel tx register; disable RX
                          //         fifo buffer; xtal cap 12pf, same as xmitter
    rf12_control(0x82c0); // 82C0    enable receiver; enable basebandblock 
    rf12_control(0xA640); // A68A    433.92mhz (A620) or 434mhz (AC40)
    rf12_control(0xc647); // C691    c691 datarate 2395 kbps 0xc647 = 4.8kbps 
    rf12_control(0x948B); // 9489    VDI; FAST;200khz;GAIn -6db; DRSSI 97dbm   (948A for long range, 948B for short range < 1m)
    rf12_control(0xC220); // C220    datafiltercommand; ** not documented cmd 
    rf12_control(0xCA00); // CA00    FiFo and resetmode cmd; FIFO fill disabeld
    rf12_control(0xC473); // C473    AFC run only once; enable AFC; enable
                          //         frequency offset register; +3 -4
    rf12_control(0xCC67); // CC67    pll settings command
    rf12_control(0xB800); // TX register write command not used
    rf12_control(0xC800); // disable low dutycycle 
    rf12_control(0xC040); // 1.66MHz,2.2V not used see 82c0  
}

void setup () {
#if DEBUG_LED   
    pinMode(DEBUG_LED, 1);
    // brief LED flash on startup to make sure it works
    digitalWrite(DEBUG_LED, 1);
    delay(100);
    digitalWrite(DEBUG_LED, 0);
#endif

#if DEBUG
    Serial.begin(57600);
    Serial.println("\n[ookRelay2]");
#endif

#if USE_RF12
//#if PIN_868
    rf12_init_OOK();
//#else
//    rf12_initialize(NODEID, RF12_868MHZ, NETGRP);
//#endif
#endif

#if PIN_433
    setupPinChangeInterrupt();
#endif

#if POWER_433
    pinMode(POWER_433, 1);
    digitalWrite(POWER_433, 1);
#endif
}

void loop () {
#if PIN_433
    runPulseDecoders(di_433, pulse_433);  
#endif
}

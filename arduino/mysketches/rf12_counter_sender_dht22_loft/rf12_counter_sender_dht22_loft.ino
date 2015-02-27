#include <DHT.h>

#include <JeeLib.h>
#include <util/crc16.h>
#include <util/parity.h>
#include <avr/sleep.h>

dht DHT;

#define DHT22PIN 6
#define SERIAL_BAUD 57600
#define COLLECT 0x20 // collect mode, i.e. pass incoming without sending acks
#define HUB_NODE 1

volatile bool adcDone;

ISR(WDT_vect) { Sleepy::watchdogEvent(); }
ISR(ADC_vect) { adcDone = true; }

/*
 * Method used to read the current voltage left in the
 * battery so we can shut down when it goes too low
 */
static byte vccRead (byte count =4) {
  set_sleep_mode(SLEEP_MODE_ADC);
  ADMUX = bit(REFS0) | 14; // use VCC and internal bandgap
  bitSet(ADCSRA, ADIE);
  while (count-- > 0) {
    adcDone = false;
    while (!adcDone)
      sleep_mode();
  }
  bitClear(ADCSRA, ADIE);  
  // convert ADC readings to fit in one byte, i.e. 20 mV steps:
  //  1.0V = 0, 1.8V = 40, 3.3V = 115, 5.0V = 200, 6.0V = 250
  float temp = (55U * 1023U) / (ADC + 1) - 50;
  
  // Then convert to actual voltages, x 10 so that 3.3v = 33 which fits into a byte
  float temp2 = temp / 250 * 100;
  float voltage = ((5.0 / 100.0 * temp2) + 1.0) * 10.0;
  return (byte)voltage;
}

static char cmd;
static char stack[RF12_MAXDATA];
static byte value, top, sendLen, dest, quiet = 1;
static byte testbuf[RF12_MAXDATA], testCounter;
byte counter = 0;
byte oldCounter = 0;
byte batteryVoltage = 0;
byte shuttingDown = 0;
int sleepIterations = 10;
byte nodeNumber = 2;

void setup() {
  
    Serial.begin(SERIAL_BAUD);
    Serial.println("INFO: rf12_counter_sender started");
    
    delay(2000);
    
    // Initialise as node 2, 433mHz, group 53
    rf12_initialize((0x40) + nodeNumber, RF12_433MHZ, 0x35);
    rf12_sleep(RF12_SLEEP);
    
    // Set specific RF12 control commands
    rf12_control(0xC047);   // Set the low-voltage threshold to 2.9v
    rf12_control(0x82C5);   // Enable transmit, disable receive/pll-synthesize/crystal/wakeup-timer
    
    Sleepy::loseSomeTime(2000);
}

void loop() {
  
  Serial.println("INFO: Loop start");
  
  int chk = DHT.read22(DHT22PIN);
  float temp;
  float humidity;
  byte header = 0;
  MilliTimer timer;
  
  // The first loop() call, read the current battery voltage.
  // After that we only update the reading every n 
  if (batteryVoltage == 0) batteryVoltage = vccRead();

  switch (chk)
  {
    case DHTLIB_OK:
                Serial.print("INFO: ");
                humidity = (float)DHT.humidity;
                temp = (float)DHT.temperature;
                Serial.print("Humidity: ");
                Serial.print(humidity, 2);
                Serial.print("%   Dig Temp: ");
                Serial.print(temp, 2);
                Serial.println("C");
                
                Serial.println("INFO: Sending data...");
                
                if (batteryVoltage < 30) {
                  // If battery has dropped below 3v, shutdown (it's unsafe to run the lipo batteries flat)
                  sprintf(stack, "%s%d %d %d %d %d NODE_SHUTTING_DOWN_LOW_VOLTS", "Nd0", nodeNumber, counter, (int)humidity, (int)temp, (int)batteryVoltage);
                  shuttingDown = 1;
                } else {
                  sprintf(stack, "%s%d %d %d %d %d", "Nd0", nodeNumber, counter, (int)humidity, (int)temp, (int)batteryVoltage);
                }
                   
                Serial.print("INFO: Data buffer = ");
                Serial.println(stack);
                   
                memcpy(testbuf, stack, strlen(stack));
                   
                Serial.println("INFO: Ready to send");
                   
                // Reinstate this delay for serial output to work correctly
                //delay(20);
                   
                header = RF12_HDR_ACK;                       // Specify that we want an ACK
                header |= RF12_HDR_DST | HUB_NODE;                // Set the destination node
                   
                oldCounter = counter;
                   
                while (true) {
                   while (!rf12_canSend()) 
                      rf12_recvDone();
                      
                   rf12_sendStart(header, testbuf, strlen(stack));     // Send the data  
                   rf12_sendWait(2);
                   
                   timer.set(30);
                   
                   while (!timer.poll()) {
                      if (rf12_recvDone() && rf12_crc == 0) {
                         // got a good ACK packet
                         Serial.println("INFO: Ack received");
                         
                         // If the previous battery reading was too low, we shut down after receiving the last ACK
                         if (shuttingDown) {
                           sleepIterations = 25000;
                         }
                         
                         counter++;
                         if (counter > 9) {
                           counter = 0;
                           
                           // Every nth reading, read the current remaining voltage in the battery
                           batteryVoltage = vccRead();
                         }
                         break;
                      }
                   }
                   
                   // Reinstate this delay for Serial output to work correctly
                   //delay(20);
                   
                   if (oldCounter == counter) {
                     // We didn't receive an ACK - try again
                     rf12_sleep(RF12_SLEEP);
                     Sleepy::loseSomeTime(4000);
                     rf12_sleep(RF12_WAKEUP);
                   } else {
                     break;
                   }
                }
                   
                Serial.println("INFO: Ready to sleep");
                break;
    case DHTLIB_ERROR_CHECKSUM:
    case DHTLIB_ERROR_TIMEOUT:
    default:
         Serial.println("DHT error");
         delay(1000);
         break;
  }
  
  // Reinstate this delay for serial to work correctly
  //delay(20);
  
  rf12_sleep(RF12_SLEEP);
  for (int i=0; i<sleepIterations; i++) {
     Sleepy::loseSomeTime(60000);
  }
  rf12_sleep(RF12_WAKEUP);
  
  Serial.println("INFO: Woken after sleep");
}

  
#include <CapacitiveSensor.h>

#include <JeeLib.h>
#include <util/crc16.h>
#include <util/parity.h>
#include <avr/sleep.h>

#define SERIAL_BAUD 57600
#define COLLECT 0x20 // collect mode, i.e. pass incoming without sending acks
#define HUB_NODE 1
#define NODE_NUMBER 4
#define SIZE_OF_AVERAGE_WINDOW 30
#define CAPACITANCE_SEND_PIN 4
#define CAPACITANCE_SENSE_PIN 8
#define SETTLE_DOWN_AFTER_N_PINGS 10

volatile bool adcDone;

ISR(WDT_vect) { Sleepy::watchdogEvent(); }
ISR(ADC_vect) { adcDone = true; }

CapacitiveSensor capSensor = CapacitiveSensor(CAPACITANCE_SEND_PIN,CAPACITANCE_SENSE_PIN);

int i = 0;
int dataSet[SIZE_OF_AVERAGE_WINDOW];

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
static byte testbuf[RF12_MAXDATA], testCounter;
byte counter = 0;
byte oldCounter = 0;
byte batteryVoltage = 0;
byte shuttingDown = 0;
int j = 0;
long capAverage = 0;
byte header = RF12_HDR_ACK | (RF12_HDR_DST | HUB_NODE);
byte ackReceived = 0;
int totalPingCount = 0;

int sleepIterations = 10;
int sleepTimeMillis = 500;
byte settledDown = 0;

void setup() {   
  
    //Serial.begin(SERIAL_BAUD);
    //Serial.println("INFO: rf12_counter_sender started");
    
    delay(2000);
    
    //header = RF12_HDR_ACK;                       // Specify that we want an ACK
    //header |= RF12_HDR_DST | HUB_NODE;           // Set the destination node
    
    // Initialise as node 4, 433mHz, group 53
    rf12_initialize((0x40) + NODE_NUMBER, RF12_433MHZ, 0x35);
    rf12_sleep(RF12_SLEEP);
    
    // Set specific RF12 control commands
    rf12_control(0xC047);   // Set the low-voltage threshold to 2.9v
    rf12_control(0x82C5);   // Enable transmit, disable receive/pll-synthesize/crystal/wakeup-timer
    
    Sleepy::loseSomeTime(2000);
}

void loop() {
  
  //Serial.println("INFO: Loop start");
  
  /*******      THIS WAS CHANGED LAST TIME I UPDATED IT, TO USE THE NEWER       ******** 
   ******* LIBRARY - COMMENTING IN CASE IT DOESN'T WORK STRAIGHT AWAY NEXT TIME *********/
  //MilliTimer timer;
  
  // The first loop() call, read the current battery voltage.
  // After that we only update the reading every n 
  if (batteryVoltage == 0) batteryVoltage = vccRead();
  
  i = 0;
  // Take 30 readings, each of which comprises 30 samples
  while (i < SIZE_OF_AVERAGE_WINDOW) {
     // Start timing the RC capacitance circuit
     long start = millis();
  
     // Tell the capSense library to use 30 samples
     long capReading =  capSensor.capacitiveSensor(30);
  
     // Add this value to next index in the data set. We'll calculate the
     // average of this data set once it's full  
     dataSet[i] = (int)capReading;
     i++;
     
     // Wait 10 milliseconds, then take another 30-sample reading
    delay(10);
  }
  
  // We've filled up the data set - calculate the average and then send it  
      
  // Calculate the average capacitance value
  for (j = 0; j < SIZE_OF_AVERAGE_WINDOW; j++) {
    capAverage = capAverage + dataSet[j];
  }
  capAverage = capAverage / SIZE_OF_AVERAGE_WINDOW;
    
  //Serial.print("INFO: ");
  //Serial.print("Average Capacitance: ");
  //Serial.println((int)capAverage);
                
  //Serial.println("INFO: Sending data...");
              
  if (batteryVoltage < 30) {
    // If battery has dropped below 3v, shutdown (it's unsafe to run the lipo batteries flat)
    sprintf(stack, "Nd0%d %d %d NODE_SHUTTING_DOWN_LOW_VOLTS", NODE_NUMBER, counter, (int)batteryVoltage);
    shuttingDown = 1;
  } else {
    sprintf(stack, "Nd0%d %d %d", NODE_NUMBER, (int)capAverage, (int)batteryVoltage);
  }
                   
  //Serial.print("INFO: Data buffer = ");
  //Serial.println(stack);
                   
  memcpy(testbuf, stack, strlen(stack));
                   
  //Serial.println("INFO: Ready to send");
  
  //delay(20);
                   
  oldCounter = counter;
                
  //Serial.println("INFO: Waiting to be able to send");
  //delay(20);
    
  while (!rf12_canSend()) rf12_recvDone();
                   
  //Serial.println("INFO: Starting to send");  
  rf12_sendStart(header, testbuf, strlen(stack), 1);     // Send the data  
  rf12_sendWait(2);
               
  // Give ourselves 30 milliseconds to receive an ACK    
  //timer.set(30);
  //ackReceived = 0;
                   
  //while (!timer.poll()) {
  //  if (rf12_recvDone() && rf12_crc == 0) {
      //ackReceived = 1;
      // got a good ACK packet
      //delay(20);
      //Serial.println("INFO: ACK");
      //delay(20);
  //  }
  //}
  
  //if (!ackReceived) {
  //  delay(20);
  //  Serial.println("INFO: NOACK");
  //  delay(20);
  //}
    
  counter++;
  if (counter > 9) {
    counter = 0;
                           
    // Every nth reading, read the current remaining voltage in the battery
    batteryVoltage = vccRead();
  }
                   
  //Serial.println("INFO: Ready to sleep");

  // Reinstate this delay for serial to work correctly
  //delay(20);
  
  rf12_sleep(RF12_SLEEP);
  for (int i=0; i<sleepIterations; i++) {
     Sleepy::loseSomeTime(sleepTimeMillis);
  }
  rf12_sleep(RF12_WAKEUP);
  
  totalPingCount++;
  
  // For debugging purposes, when the arduino boots it pings more 
  // frequently back to the hub for the first few minutes, then
  // settles down. 
  if (!settledDown && totalPingCount > SETTLE_DOWN_AFTER_N_PINGS) {
    sleepIterations = 30;
    sleepTimeMillis = 60000;
    settledDown = 1;
  }
  
  //Serial.println("INFO: Woken after sleep");
}

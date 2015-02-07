// Code that takes readings from a reflectivity sensor every 250ms
// to measure when the gas meter dial rotates one turn

// Dummy watchdog interrupt routine
ISR(WDT_vect) {
}

#include "avr/sleep.h"

#define ledPin 13
#define gasPhotoSensePin 18

// We loop every quarter of a second, but only want to flash the 
// LED every 4 seconds so we use a 16x multiplier for the LED loop
int ledLoopCount = 16;
int ledValue = HIGH;
int i = 0;

void setup() {
  analogReference(DEFAULT);
  
  // The raw 250ms measurements are sent directly over serial
  // so we leave the computer to decide how to tell if the
  // dial has turned. This means the arduino doesn't have to
  // be hardcoded with the relative high & low light levels.
  // If the sensor gets knocked or moved the software on the
  // viglen can be modified without having to upload new
  // firmware to the arduino. 
  Serial.begin(9600);
  
  pinMode(ledPin, OUTPUT);
  pinMode(gasPhotoSensePin, INPUT);
  
  // Setup the system to use watchdog interrups in the loop code...
  MCUSR &= ~(1<<WDRF);
  cli();
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  
  // Set the sleep time to4 for 1/4 of a second (set to 5 for 1/2 a second, 7 for 2 seconds) 
  WDTCSR = 4;
  WDTCSR |= bit(WDIE);
  sei();
}

void loop() {  
  
  //The way to sleep for 250ms seconds in power down mode,
  // with the ADC disabled while we're in sleep mode...
  delay(30);
  uint8_t prrSave = PRR, adcsraSave = ADCSRA;
  ADCSRA &= ~bit(ADEN);
  PRR &= ~bit(PRADC);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_mode();
  PRR = prrSave;
  ADCSRA = adcsraSave;
  
  // Read the value of the sensor pin
  analogRead(gasPhotoSensePin);
  delay(20);
  
  // Debounce the reading by taking it again
  int photoResistance = analogRead(gasPhotoSensePin);
  
  // Write the value out over serial
  Serial.print("Gas sensor reading: ");
  Serial.println(photoResistance);
  
  // Flash the LED every few seconds so we know
  // at a glance if the arduino is running OK
  if (i == (ledLoopCount-1)) {
    digitalWrite(ledPin, ledValue);
    i = 0;
    ledValue = !ledValue;
  } else {
    digitalWrite(ledPin, !ledValue);
  }
  
  i++;
}

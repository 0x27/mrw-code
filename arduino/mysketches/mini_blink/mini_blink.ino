/*
   Sketch designed to sleep in power-down mode for several minutes
   before performing a small amount of work (e.g. capturing and
   sending some data) and then going back to sleep. It is intended
   to be as power-efficient as possible.
 */
 
// This variable is made volatile because it is changed inside
// an interrupt function

volatile int sleep_count = 0; // Keep track of how many sleep
                              // cycles have been completed.
                              
const int interval = 5;       // Number of sleep cycles to loop through
                              // before waking up to actually do some work.
                              // (Total sleep time = interval * 8)

// Watchdog interrupt routine
ISR(WDT_vect) {
  sleep_count++;
}

#include "avr/sleep.h"
 
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;

void performStartupLEDFlash() {
  digitalWrite(led, HIGH);
  delay(200);
  digitalWrite(led, LOW);
  delay(700);
  digitalWrite(led, HIGH);
  delay(200);
  digitalWrite(led, LOW);
  delay(700);
  digitalWrite(led, HIGH);
  delay(200);
  digitalWrite(led, LOW);
  delay(700);
  digitalWrite(led, HIGH);
  delay(200);
  digitalWrite(led, LOW);
  delay(700);
  digitalWrite(led, HIGH);
  delay(200);
  digitalWrite(led, LOW);
  delay(700);
  digitalWrite(led, HIGH);
  delay(850);
  digitalWrite(led, LOW);
}

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT); 

  // Flash the LED on pin 13 to indicate clearly that the arduino
  // has booted up correctly. This helps now that we've removed
  // the power-on LED.
  performStartupLEDFlash();
  
  // Setup the system to use watchdog interrups in the loop code...
  MCUSR &= ~(1<<WDRF);
  cli();
  // Enabled watchdog interrupts
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  WDTCSR = 33;
  WDTCSR |= bit(WDIE);
  sei();
}

// the loop routine runs over and over again forever:
void loop() {
  if (sleep_count == interval) {
    sleep_count = 0;
    
    digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  
    delay(500);               // wait for 2 seconds (the old way)
  
    digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  
    // Sleep for 60 seconds in power down mode,
    // with the ADC disabled
    //delay(30);
    //uint8_t prrSave = PRR, adcsraSave = ADCSRA, acsrSave = ACSR, diSave = DIDR0;
    //ADCSRA &= ~bit(ADEN); // Disable the ADC
    //PRR &= B11111111; // Disable *everything*     ~bit(PRADC); // Disable the ADC only
    //ACSR = B10000000;     // Disable the analog comparator
    //DIDR0 = DIDR0 | B00111111;   // Disable digital input on analog pins
    //set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    //sleep_mode();
    //PRR = prrSave;
    //ADCSRA = adcsraSave;
    //ACSR = acsrSave;
    //DIDR0 = diSave;
  
    // delay(60000);               // wait for 60 seconds (the old way)
  } else {
    // Sleep for 60 seconds in power down mode,
    // with the ADC disabled
    delay(30);
    uint8_t prrSave = PRR, adcsraSave = ADCSRA, acsrSave = ACSR, diSave = DIDR0;
    ADCSRA &= ~bit(ADEN); // Disable the ADC
    PRR &= B11111111; // Disable *everything*     ~bit(PRADC); // Disable the ADC only
    ACSR = B10000000;     // Disable the analog comparator
    DIDR0 = DIDR0 | B00111111;   // Disable digital input on analog pins
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_mode();
    PRR = prrSave;
    ADCSRA = adcsraSave;
    ACSR = acsrSave;
    DIDR0 = diSave;
  }
}

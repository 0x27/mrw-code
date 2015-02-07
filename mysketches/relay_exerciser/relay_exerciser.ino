/*
 * Script to control a relay.
 *
 * Mostly used as a demo - copy the code snippets into another program.
 *
 * Uses digital pins 6 and 7 to alternate between high and low every half a second,
 * causing the relays to switch every half a second.
 */

// Define the Arduino pins used to control the relay
// NOTE: On an Arduino nano, analogue pins 6 and 7 can't be used for digital write
#define RELAY1 6
#define RELAY2 7

void setup() {
  
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
}

// Loop forever settings the relays high and low alternately
void loop() {
  digitalWrite(RELAY1, HIGH);
  delay(500);
  digitalWrite(RELAY2, HIGH);
  delay(500);
  digitalWrite(RELAY1, LOW);
  delay(500);
  digitalWrite(RELAY2, LOW);
  delay(500);
}

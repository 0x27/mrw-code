/*
  Melody
 
 Plays a melody 
 
 circuit:
 * 8-ohm speaker on digital pin 8
 
 created 21 Jan 2010
 modified 30 Aug 2011
 by Tom Igoe 

This example code is in the public domain.
 
 http://arduino.cc/en/Tutorial/Tone
 
 */
 #include "pitches.h"

// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_E4,NOTE_G4, NOTE_C5, NOTE_G3,0, NOTE_B3, NOTE_C4};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 4, 4, 4,4,4,4,4 };
  
int startupLDRReading;
int ldrThreshold = 0;

int tempI = 0;
  
int soundOn = 0;

void setup() {
  pinMode(15, INPUT);
  pinMode(8, OUTPUT);
  pinMode(13, OUTPUT);
  
  Serial.begin(9600);
  
  pinMode(8, INPUT);
  delay(20);
   // read the input on analog pin 0:
  analogRead(15);
  delay(20);
  startupLDRReading = analogRead(15);
  // print out the value you read:
  Serial.println(startupLDRReading);
  ldrThreshold = startupLDRReading * 1.5;
  
  if (ldrThreshold > 900) {
    ldrThreshold = 900;
  } else if (ldrThreshold < 100) {
    ldrThreshold = 100;
  }
  
  ldrThreshold = startupLDRReading + 100;
  
  delay(20);
  pinMode(8, OUTPUT);
  delay(20);
  
  pinMode(8, INPUT);
  delay(20);
  pinMode(8, OUTPUT);
  delay(20);
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 4; thisNote++) {

    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000/6;
    tone(8, melody[thisNote],noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(8);
  }
}

void loop() {
  pinMode(8, INPUT);
  delay(10);
   // read the input on analog pin 0:
  int sensorValue = analogRead(15);
  // print out the value you read:
  //Serial.println(sensorValue);
  delay(10);
  pinMode(8, OUTPUT);
  delay(10);
  
  if (!soundOn && sensorValue > ldrThreshold) {
    Serial.println("BEEP");
    digitalWrite(13, HIGH);
    int thisNote = 0;
    int noteDuration = 1000/noteDurations[thisNote];
    tone(8, melody[thisNote],noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    //noTone(8);
    soundOn = 1;
    //noTone(8);
  } else if (sensorValue < ldrThreshold) {
    noTone(8);
    digitalWrite(13, LOW);
    Serial.println("OFF");
    soundOn = 0;
  }
  
  delay(20);        // delay in between reads for stability
}

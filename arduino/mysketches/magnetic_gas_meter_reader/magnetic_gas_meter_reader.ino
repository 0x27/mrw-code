/*
 * Code which reads the value of a hall sensor on pin 14 and
 * writes "LOOP" down the serial port if a magnetic is detected.
 */
 
#define LED_LOOP_DELAY_MILLIS 200
 
int hallPin=14;
int ledPin = 13;

int statePin = LOW;
int previousReading = statePin;

void setup()
{
  pinMode(hallPin, INPUT);
  
  pinMode(ledPin, OUTPUT);
  
  digitalWrite(ledPin, LOW);
  
  Serial.begin(9600);
}

void loop()
{
  statePin=digitalRead(hallPin);
  
  // We're only interested in a state change
  if (statePin != previousReading) {
    
    previousReading = statePin;
    
    if (statePin==LOW) {  // We've detected a magnetic field
    
      Serial.println("Loop"); 
      
      digitalWrite(ledPin, HIGH);
      delay(LED_LOOP_DELAY_MILLIS);
      digitalWrite(ledPin, LOW);
      delay(LED_LOOP_DELAY_MILLIS);
      digitalWrite(ledPin, HIGH);
      delay(LED_LOOP_DELAY_MILLIS);
      digitalWrite(ledPin, LOW);
      delay(LED_LOOP_DELAY_MILLIS);
      digitalWrite(ledPin, HIGH);
      delay(LED_LOOP_DELAY_MILLIS);
      digitalWrite(ledPin, LOW);
      delay(LED_LOOP_DELAY_MILLIS);
      digitalWrite(ledPin, HIGH);
      delay(LED_LOOP_DELAY_MILLIS);
      digitalWrite(ledPin, LOW);
      delay(LED_LOOP_DELAY_MILLIS);
      digitalWrite(ledPin, HIGH);
      delay(LED_LOOP_DELAY_MILLIS);
      digitalWrite(ledPin, LOW);
      delay(LED_LOOP_DELAY_MILLIS);
      digitalWrite(ledPin, HIGH);
      delay(LED_LOOP_DELAY_MILLIS);
      digitalWrite(ledPin, LOW);
      delay(LED_LOOP_DELAY_MILLIS);
      digitalWrite(ledPin, HIGH);
      delay(LED_LOOP_DELAY_MILLIS);
      digitalWrite(ledPin, LOW);
      delay(LED_LOOP_DELAY_MILLIS);
      digitalWrite(ledPin, HIGH);
      delay(LED_LOOP_DELAY_MILLIS);
      digitalWrite(ledPin, LOW);
      delay(LED_LOOP_DELAY_MILLIS);
      digitalWrite(ledPin, HIGH);
      delay(LED_LOOP_DELAY_MILLIS);
      digitalWrite(ledPin, LOW);
      delay(LED_LOOP_DELAY_MILLIS);
      
      // Sleep several seconds
      delay(8000);
    }
  }
  
  delay(200);
}

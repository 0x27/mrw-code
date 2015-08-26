// Code that takes readings from a hall (magnetic field) sensor
// every second. If the field drops below a pre-configured threshold
// a 1 is sent down the serial port. Another 1 isn't sent until
// the reading has gone back above the threshold.

#define THRESHOLD_MAGNETIC_FIELD_LOW  520
#define THRESHOLD_MAGNETIC_FIELD_HIGH  550

int sensorPin = A4;    // select the input pin
int ledPin = 13;       // select the pin for the LED
int sensorValue = 0;   // variable to store the value coming from the sensor
int currentlySeeMagnet = 0;
int magnetOrientation = 0;
 
void setup () {
  pinMode(ledPin, OUTPUT);
  pinMode(sensorPin, INPUT);
  Serial.begin(9600);
  Serial.println("Gas monitor starting. No data will flow until a gas 'tick' is seen.");
  delay(30);
}
 
void loop () {
  // Read the hall sensor
  sensorValue = analogRead (sensorPin);
  
  // Debounce by taking another reading
  delay(20);
  sensorValue = analogRead (sensorPin);
  
  if (sensorValue == 0 || sensorValue == 1023) {
    // There's almost certainly something wrong with the wiring
    // Send an error message
    Serial.println("HALL SENSOR RETURNING 0 or 1023. WIRING PROBABLY BROKEN");
    delay(60000);
  } else {
    if (!currentlySeeMagnet && (sensorValue < THRESHOLD_MAGNETIC_FIELD_LOW || sensorValue > THRESHOLD_MAGNETIC_FIELD_HIGH)) {
      
      // This bit of code means it doesn't matter which way round the magnet is
      if (sensorValue < THRESHOLD_MAGNETIC_FIELD_LOW) {
        magnetOrientation = 0;
      } else {
        magnetOrientation = 1;
      }
      
      currentlySeeMagnet = 1; 
      digitalWrite (ledPin, HIGH);
      delay (400);
      digitalWrite (ledPin, LOW);
      delay (400);
      Serial.println(1);
    
      // Now wait a few seconds to allow the wheel to turn a bit
      delay(4000);
    } else {
      if (!magnetOrientation && sensorValue > THRESHOLD_MAGNETIC_FIELD_LOW) {
        // The wheel has turned enough that we're back above the threshold
        currentlySeeMagnet = 0;
      } else if (magnetOrientation && sensorValue < THRESHOLD_MAGNETIC_FIELD_HIGH) {
        // The wheel has turned enough that we're back above the threshold
        currentlySeeMagnet = 0;
      }
    }
  }
}

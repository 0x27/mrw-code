#define temperaturePin 0
#define lightSensePin 5
#define ledLightPin 9

int ledPin = 13;
int currentLightLevel = 0;
int currentLEDLevel = 0;
int statusLEDCount = 0;
boolean statusLEDHigh = false;

void setup() {
  analogReference(DEFAULT);
  Serial.begin(9600);
  
  pinMode(ledPin, OUTPUT);
  pinMode(ledLightPin, OUTPUT);
}

void loop() {  
  setStatusLED();
  
  // Ignore the return value for the time being - let it just be dumped to debug
  readTemperature();
  
  delay(50);
  
  currentLightLevel = readLightLevel();
  
  //Serial.println(currentLightLevel);
  
  currentLightLevel = constrain(currentLightLevel, 400, 800);
  currentLEDLevel = map(currentLightLevel, 400, 800, 255, 0);
  
  Serial.println(currentLEDLevel);
  
  analogWrite(ledLightPin, currentLEDLevel);
  
  delay(50);
}

// Set the status LED to flash every 2 seconds or so
void setStatusLED() {
  if (statusLEDHigh == false && statusLEDCount == 15) {
    digitalWrite(ledPin, HIGH);
    statusLEDHigh = true;
  } else if (statusLEDHigh == true && statusLEDCount == 0) {
    digitalWrite(ledPin, LOW);
    statusLEDHigh = false;
  } else if (statusLEDHigh == false) {
    statusLEDCount++;
  } else if (statusLEDHigh == true) {
    statusLEDCount--;
  }
}

int readLightLevel() {
  // Take the reading once and ignore the value
  analogRead(lightSensePin);
  
  // Then take it a second time to let the ADC settle
  delay(10);
  int reading = analogRead(lightSensePin);
  
  Serial.print("Light: ");
  Serial.println(reading);
  
  return reading;
}

float readTemperature() {
  // Take the reading once and ignore the value
  analogRead(temperaturePin);
  
  // Then take it a second time to let the ADC settle
  delay(10);
  int reading = analogRead(temperaturePin);
 
  float tempC = (reading * 5.0 * 100) / 1024;
  Serial.print("Temp: ");
  Serial.print(tempC);
  Serial.println(" deg C");
  
  return tempC;
}

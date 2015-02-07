//Celsius to Fahrenheit conversion
double Fahrenheit(double celsius)
{
        return 1.8 * celsius + 32;
}

//Celsius to Kelvin conversion
double Kelvin(double celsius)
{
        return celsius + 273.15;
}

// dewPoint function NOAA
// reference: http://wahiduddin.net/calc/density_algorithms.htm
double dewPoint(double celsius, double humidity)
{
        double A0= 373.15/(273.15 + celsius);
        double SUM = -7.90298 * (A0-1);
        SUM += 5.02808 * log10(A0);
        SUM += -1.3816e-7 * (pow(10, (11.344*(1-1/A0)))-1) ;
        SUM += 8.1328e-3 * (pow(10,(-3.49149*(A0-1)))-1) ;
        SUM += log10(1013.246);
        double VP = pow(10, SUM-3) * humidity;
        double T = log(VP/0.61078);   // temp var
        return (241.88 * T) / (17.558-T);
}

// delta max = 0.6544 wrt dewPoint()
// 5x faster than dewPoint()
// reference: http://en.wikipedia.org/wiki/Dew_point
double dewPointFast(double celsius, double humidity)
{
        double a = 17.271;
        double b = 237.7;
        double temp = (a * celsius) / (b + celsius) + log(humidity/100);
        double Td = (b * temp) / (a - temp);
        return Td;
}

// Function used to reduce the clock frequency when setting
// the system into sleep mode. Only used if we're not in
// "power down" sleep mod
//void setPrescaler(uint8_t mode) {
//  cli();
//  CLKPR = bit(CLKPCE);
//  CLKPR = mode;
//  sei();
//}

// Dummy watchdog interrupt routine
ISR(WDT_vect) {
}


#include <dht11.h>
#include <LiquidCrystal.h>
#include "avr/sleep.h"

// initialize the library with the numbers of the interface pins
dht11 DHT11;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


#define DHT11PIN 16


#define temperaturePin 0
#define lightSensePin 7
#define ledLightPin 9

int ledPin = 13;
int currentLightLevel = 0;
int currentLEDLevel = 0;
int statusLEDCount = 0;
boolean statusLEDHigh = false;

void setup() {
  analogReference(DEFAULT);
  //Serial.begin(9600);
  
  pinMode(ledPin, OUTPUT);
  pinMode(ledLightPin, OUTPUT);
  
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("   Reading     ");
  lcd.setCursor(0, 1);
  lcd.print("   sensors...");
  
  delay(1000);
  
  // Setup the system to use watchdog interrups in the loop code...
  MCUSR &= ~(1<<WDRF);
  cli();
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  WDTCSR = 7;
  WDTCSR |= bit(WDIE);
  sei();
}

void loop() {  
  // Removed to save power - add back in for the LED to
  // switch one & off every 2 seconds
  //setStatusLED();
  
  // Ignore the return value for the time being - let it just be dumped to debug
  //readTemperature();
  
  // The way to sleep for 2 seconds in idle mode...
  //delay(30);
  //setPrescaler(6);
  //long limit = millis() + 2000/64;
  //while ((long) millis() - limit < 0) {
  //  set_sleep_mode(SLEEP_MODE_IDLE);
  //  sleep_mode();
  //}
  //setPrescaler(0);
  
  //The way to sleep for 2 seconds in power down mode,
  // with the ADC disabled while we're in sleep mode...
  delay(30);
  uint8_t prrSave = PRR, adcsraSave = ADCSRA;
  ADCSRA &= ~bit(ADEN);
  PRR &= ~bit(PRADC);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_mode();
  PRR = prrSave;
  ADCSRA = adcsraSave;

  //LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
  lcd.begin(16, 2);
  
  //currentLightLevel = readLightLevel();
  
  //Serial.println(currentLightLevel);
  
  //currentLightLevel = constrain(currentLightLevel, 400, 800);
  //currentLEDLevel = map(currentLightLevel, 400, 800, 255, 0);
  
  //Serial.print("   LED: ");
  //Serial.print(currentLEDLevel);
  
  //analogWrite(ledLightPin, currentLEDLevel);
  
  //delay(1000);
  
  int chk = DHT11.read(DHT11PIN);

  switch (chk)
  {
    case DHTLIB_OK:
                //Serial.print("   Humidity: ");
                //Serial.print((float)DHT11.humidity, 2);
                //Serial.print("%   Dig Temp: ");
                //Serial.print((float)DHT11.temperature, 2);
                //Serial.println("C");
                lcd.setCursor(0,0);
                lcd.print("Humidity: ");
                lcd.setCursor(10, 0);
                lcd.print((float)DHT11.humidity, 2);
                lcd.setCursor(0,1);
                lcd.print("Temp:     ");
                lcd.setCursor(10,1);
                lcd.print((float)DHT11.temperature, 2);
                break;
    case DHTLIB_ERROR_CHECKSUM:
                //Serial.println("Checksum error");
                lcd.setCursor(0, 0);
                lcd.print("Error");
                lcd.setCursor(5, 0);
                lcd.print("       ");
                break;
    case DHTLIB_ERROR_TIMEOUT:
                //Serial.println("Time out error");
                lcd.setCursor(0, 0);
                lcd.print("Error");
                lcd.setCursor(5, 0);
                lcd.print("       ");
                break;
    default:
                //Serial.println("Unknown error");
                lcd.setCursor(0, 0);
                lcd.print("Error");
                lcd.setCursor(5, 0);
                lcd.print("       ");
                break;
  }
}

// Set the status LED to flash every 2 seconds or so
void setStatusLED() {
  if (statusLEDHigh == false && statusLEDCount == 1) {
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
  
  //Serial.print("   Light: ");
  //Serial.print(reading);
  
  return reading;
}

float readTemperature() {
  // Take the reading once and ignore the value
  analogRead(temperaturePin);
  
  // Then take it a second time to let the ADC settle
  delay(10);
  int reading = analogRead(temperaturePin);
 
  float tempC = (reading * 5.0 * 100) / 1024;
  //Serial.print("Temp: ");
  //Serial.print(tempC);
  //Serial.print(" C");
  
  return tempC;
}

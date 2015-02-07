#define aref_voltage 3.3

int ledPin = 13;
//int ledPin2 = 8;
//int lightSensorPin = 0;
int tempSensorPin = A1;

float currentTemp = 0;

void setup() {
  analogReference(EXTERNAL);
  
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  //pinMode(ledPin2, OUTPUT);
  //pinMode(lightSensorPin, INPUT);
  pinMode(tempSensorPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly: 
  digitalWrite(ledPin, HIGH);
  //digitalWrite(ledPin2, LOW);
  delay(2000);
  digitalWrite(ledPin, LOW);
  //digitalWrite(ledPin2, HIGH);
  delay(2000);
  
  int inputValue = analogRead(tempSensorPin);
  Serial.print("Input value: ");
  Serial.println(inputValue);
  
  float voltage = inputValue * aref_voltage;
  voltage /= 1024.0;
  currentTemp = (voltage - 0.5) / 0.01;
  
  Serial.print("Voltage: ");
  Serial.println(voltage);
  
  //Serial.print("Light: ");
  //Serial.println(analogRead(lightSensorPin));
  Serial.print("Temp:  ");
  Serial.print(currentTemp);
  Serial.println(" deg C");
  Serial.println(" ");
  Serial.println(" ");
}

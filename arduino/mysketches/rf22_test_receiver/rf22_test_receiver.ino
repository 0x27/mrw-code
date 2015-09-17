// rf22_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing client
// with the RH_RF22 class. RH_RF22 class does not provide for addressing or
// reliability, so you should only use RH_RF22 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf22_server
// Tested on Duemilanove, Uno with Sparkfun RFM22 wireless shield
// Tested on Flymaple with sparkfun RFM22 wireless shield
// Tested on ChiKit Uno32 with sparkfun RFM22 wireless shield

#include <SPI.h>
#include <RH_RF22.h>
#include <RHSPIDriver.h>

// Singleton instance of the radio driver
RH_RF22 rf22;

byte rawByte = 0;
byte rawByteSPI = 0;

void printDeviceStatus() {
  uint8_t status = rf22.spiRead(RH_RF22_REG_02_DEVICE_STATUS);
  Serial.println("---- Device Status ----");
  if ((status & 1) == 0 && (status & 2) == 0) {
    Serial.println("Power: Idle");
  } else if ((status & 1) && (status & 2) == 0) {
    Serial.println("Power: RX");
  } else if ((status & 1) == 0 && (status & 2)) {
    Serial.println("Power: TX");
  }
  
  if (status & 32) {
    Serial.println("RX buffer: Empty");
  } else {
    Serial.println("RX buffer: Not empty");
  }
}

void printTXPower(uint8_t txPowerRegister) {
  Serial.println("------ TX Power ------");
  //Serial.println(txPowerRegister & 7);
  Serial.print(((txPowerRegister & 7) * 3) - 1);
  Serial.println("db");
}

void printModulationControl(uint8_t modulationControl) {
  Serial.println("--- Modulation (2) ---");
  
  if (modulationControl & 3) Serial.println("Mod: GFSK");
  else if (modulationControl & 2) Serial.println("Mod: FSK");
  else if (modulationControl & 1) Serial.println("Mod: OOK");
  else Serial.println("Mod: None");
}

void printGPIOConfig(uint8_t gpioConfig) {
  Serial.println("-----   GPIO Config -----");
  
  Serial.print("Pull up resistor: ");
  Serial.println((gpioConfig & 32 ? " False" : " True"));
  
  Serial.print("Function: ");
  Serial.println(gpioConfig & 31);
}

int i = 0;
RH_RF22::ModemConfigChoice currentMode = RH_RF22::OOK_Rb1_2Bw75;

void setup() 
{
  Serial.begin(9600);
  if (!rf22.init()) {
    Serial.println("Failed to start RFM22 chip");
  } else {
    Serial.println("Started RFM22 chip successfully");
  }
  
  pinMode(3, INPUT);
  
  if (!rf22.setFrequency(433)) {
    Serial.println("Failed to set frequency");
  } else {
    Serial.println("Set frequency to 433");
  }
  rf22.setPromiscuous(1);
  // Defaults after init are 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36
  Serial.print("Temperature: ");
  uint8_t temp = rf22.temperatureRead();
  delay(40);
  temp = rf22.temperatureRead();
  delay(40);
  temp = rf22.temperatureRead();
  delay(40);
  temp = rf22.temperatureRead();
  delay(40);
  temp = rf22.temperatureRead();
  delay(40);
  temp = rf22.temperatureRead();
  delay(40);
  temp = rf22.temperatureRead();
  delay(40);
  Serial.println(temp);
  Serial.println("Waiting for bytes...");
  
  rf22.setModemConfig(RH_RF22::OOK_Rb1_2Bw75);
  rf22.setModeRx();
  
  printDeviceStatus();
  
  Serial.print("Operating mode (1) before:");
  uint8_t opMode1 = rf22.spiRead(RH_RF22_REG_07_OPERATING_MODE1);
  Serial.println(opMode1);
  rf22.spiWrite(RH_RF22_REG_07_OPERATING_MODE1, opMode1 | 64);   // Enable low battery detect
  Serial.print("Low battery threshold: ");
  Serial.println(rf22.spiRead(RH_RF22_REG_1A_LOW_BATTERY_DETECTOR_THRESHOLD));
  rf22.spiWrite(RH_RF22_REG_1A_LOW_BATTERY_DETECTOR_THRESHOLD, 20);   // Enable low battery detect
  delay(2);
  Serial.print("Low battery threshold: ");
  Serial.println(rf22.spiRead(RH_RF22_REG_1A_LOW_BATTERY_DETECTOR_THRESHOLD));
  Serial.print("Operating mode (1) after:");
  opMode1 = rf22.spiRead(RH_RF22_REG_07_OPERATING_MODE1);
  Serial.println(opMode1);
  delay(10);
  Serial.print("Battery level: ");
  Serial.println(rf22.spiRead(RH_RF22_REG_1B_BATTERY_VOLTAGE_LEVEL));   // Read the chip voltage
  delay(10);
  printTXPower(rf22.spiRead(RH_RF22_REG_6D_TX_POWER));
  rf22.setTxPower(RH_RF22_TXPOW_20DBM);
  printTXPower(rf22.spiRead(RH_RF22_REG_6D_TX_POWER));
  
  // Reduce the amount of data that should look like a pre-amble, to zero for now, so we see more data
  byte preAmble = rf22.spiRead(RH_RF22_REG_35_PREAMBLE_DETECTION_CONTROL1);
  rf22.spiWrite(RH_RF22_REG_35_PREAMBLE_DETECTION_CONTROL1, preAmble & 7);   // Clear out the last 5 bits i.e. max sensitivity
  //rf22.spiWrite(RH_RF22_REG_35_PREAMBLE_DETECTION_CONTROL1, (preAmble & 7) | 56);   // Set all pre-amble config bitts, i.e. min sensitivity
  
  // Set the modulation control parameters
  byte modulationControl2 = rf22.spiRead(RH_RF22_REG_71_MODULATION_CONTROL2);
  printModulationControl(modulationControl2);
  rf22.spiWrite(RH_RF22_REG_71_MODULATION_CONTROL2, modulationControl2 & 252); // Clear out the first 2 bits (i.e. set modulation to NO MODULATION)
  modulationControl2 = rf22.spiRead(RH_RF22_REG_71_MODULATION_CONTROL2);
  printModulationControl(modulationControl2);
  
  // Specify what we're using GPIO pin 2 for
  byte gpio2Setting = rf22.spiRead(RH_RF22_REG_0D_GPIO_CONFIGURATION2);
  printGPIOConfig(gpio2Setting);
  rf22.spiWrite(RH_RF22_REG_0D_GPIO_CONFIGURATION2, gpio2Setting | 20);   // Set it to make RX raw data available
  //rf22.spiWrite(RH_RF22_REG_0D_GPIO_CONFIGURATION2, gpio2Setting | 3);   // Set it to be used for Direct Digital Input
  gpio2Setting = rf22.spiRead(RH_RF22_REG_0D_GPIO_CONFIGURATION2);
  printGPIOConfig(gpio2Setting);
  
}

void loop()
{  
  uint8_t data[100];
  uint8_t length = sizeof(data);
  
  while (true) {
  
  bool received = false;
  
  // Send a message to rf22_server
  if (i == 0) {
  received = rf22.recv(data, &length);
  float temp = ((float)rf22.temperatureRead() / 10);
  Serial.print("Temp: ");
  Serial.println(temp);
  }
  i++;
  
  if (i == 100) {
    i = 0;
    Serial.println();
    break;
  }
  
  rawByteSPI = rf22.spiRead(RH_RF22_REG_0E_IO_PORT_CONFIGURATION) & 4;
  rawByte = digitalRead(3);
  
  Serial.print(rawByteSPI);
  Serial.print(" (");
  Serial.print(rawByte);
  Serial.print(") ");
  
  if (received) {
    Serial.print("Received some bytes: ");
    Serial.println(length);
  } else {
    //Serial.println("Received no bytes");
  }
  
  
  //rf22.waitPacketSent();
  // Now wait for a reply
  //uint8_t buf[RH_RF22_MAX_MESSAGE_LEN];
  //uint8_t len = sizeof(buf);

  /*if (rf22.waitAvailableTimeout(500))
  { 
    // Should be a reply message for us now   
    if (rf22.recv(buf, &len))
    {
      Serial.print("got reply: ");
      Serial.println((char*)buf);
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is rf22_server running?");
  }*/
  delay(100);
  }
  
  if (currentMode == RH_RF22::OOK_Rb1_2Bw75) {
    currentMode = RH_RF22::OOK_Rb2_4Bw335;
  } else if (currentMode == RH_RF22::OOK_Rb2_4Bw335) {
    currentMode = RH_RF22::OOK_Rb4_8Bw335;
  } else if (currentMode == RH_RF22::OOK_Rb4_8Bw335) {
    currentMode = RH_RF22::OOK_Rb9_6Bw335;
  } else if (currentMode == RH_RF22::OOK_Rb9_6Bw335) {
    currentMode = RH_RF22::OOK_Rb19_2Bw335;
  } else if (currentMode == RH_RF22::OOK_Rb19_2Bw335) {
    currentMode = RH_RF22::OOK_Rb38_4Bw335;
  } else if (currentMode == RH_RF22::OOK_Rb38_4Bw335) {
    currentMode = RH_RF22::OOK_Rb40Bw335;
  } else if (currentMode == RH_RF22::OOK_Rb40Bw335) {
    currentMode = RH_RF22::OOK_Rb1_2Bw75;
  }
  
  rf22.setModemConfig(currentMode);
  rf22.setModeRx();
  Serial.print("Waiting for data in mode: ");
  Serial.println(currentMode);
  printDeviceStatus();
}


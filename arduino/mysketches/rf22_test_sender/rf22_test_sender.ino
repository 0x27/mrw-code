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
#include <ByeByeStandby.h>

// Define the 4 byte codes we'll use at the beginning of Serial
// port commands to indicate what protocol is to be used
#define BBSB "BBSB"
#define KAKU "KAKU"
#define HOME "HOME"
#define DORM "DORM"

// Define the on/off timings used to send 0s and 1s
#define ON 1200
#define OFF 500
#define TOTAL 1700

void writeHigh() {
  digitalWrite(3, HIGH);
}

void writeLow() {
  digitalWrite(3, LOW);
}

int state = 0;

// Singleton instance of the radio driver
RH_RF22 rf22;

byte rawByte = 0;
byte rawByteSPI = 0;
byte inited = 0;

void spiWriteWithACK(uint8_t theRegister, uint8_t value) {
  // Try to write the value, then wait a few milliseconds, and read it back
  // If the value hasn't been set correctly, write it again
  rf22.spiWrite(theRegister, value);
  delay(10);
  while (rf22.spiRead(theRegister) != value) {
    delay(1000);
    rf22.spiWrite(theRegister, value);
  }
}

void printDeviceStatus() {
  delay(50);
  uint8_t status = rf22.spiRead(RH_RF22_REG_02_DEVICE_STATUS);
  Serial.println("---- Device Status ----");
  
  Serial.print("Temp: ");
  uint8_t temp = rf22.temperatureRead();
  Serial.print((float)temp / 10);
  Serial.println(" deg");
  delay(10);
  
  uint8_t chipReady = rf22.spiRead(RH_RF22_REG_04_INTERRUPT_STATUS2) & 2;
  
  if ((status & 1) == 0 && (status & 2) == 0) {
    Serial.println("Chip State: Idle");
    Serial.println((chipReady ? " (Ready)" : " (Not Ready)"));
  } else if ((status & 1) && (status & 2) == 0) {
    Serial.print("Chip State: RX");
    Serial.println((chipReady ? " (Ready)" : " (Not Ready)"));
  } else if ((status & 1) == 0 && (status & 2)) {
    Serial.print("Chip State: TX");
    Serial.println((chipReady ? " (Ready)" : " (Not Ready)"));
  }
  
  Serial.print("Frequency: ");
  byte frequency = rf22.spiRead(RH_RF22_REG_75_FREQUENCY_BAND_SELECT);
  delay(10);
  if (frequency & 32) {
    // Operating in the range 480-960MHz
    Serial.print(((frequency & 31) * 10) + 480);
    Serial.print(" - ");
    Serial.print(((frequency & 31) * 10) + 490);
    Serial.println("MHz");
  } else {
    // Operating in the range 240-480MHz
    Serial.print(((frequency & 31) * 10) + 240);
    Serial.print(" - ");
    Serial.print(((frequency & 31) * 10) + 250);
    Serial.println("MHz");
  }
  
  if (status & 32) {
    Serial.println("RX buffer: Empty");
  } else {
    Serial.println("RX buffer: Not empty");
  }
}

void printTXPower() {
  delay(50);
  uint8_t txPowerRegister = rf22.spiRead(RH_RF22_REG_6D_TX_POWER);
  
  Serial.println("------ TX Power ------");
  //Serial.println(txPowerRegister & 7);
  Serial.print("Power: ");
  Serial.print(((txPowerRegister & 7) * 3) - 1);
  Serial.println("db");
}

void printModulationControl() {
  delay(50);
  uint8_t modulationControl = rf22.spiRead(RH_RF22_REG_71_MODULATION_CONTROL2);
  Serial.println("---- Modulation ----");
  
  if ((modulationControl & 3) == 1) Serial.println("Mod: OOK");
  else if ((modulationControl & 3) == 2) Serial.println("Mod: FSK");
  else if ((modulationControl & 3) == 3) Serial.println("Mod: GFSK");
  else Serial.println("Mod: None");
}

void printGPIO2Config() {
  delay(50);
  byte gpioConfig = rf22.spiRead(RH_RF22_REG_0D_GPIO_CONFIGURATION2);
  
  Serial.println("-----   GPIO Config -----");
  
  Serial.print("Pull up resistor: ");
  Serial.println((gpioConfig & 32 ? " False" : " True"));
  
  Serial.print("Function: ");
  if ((gpioConfig & 31) == 3) Serial.println("Direct Digital RX Input");
  else if ((gpioConfig & 31) == 10) Serial.println("Direct Digital TX Output");
  else if ((gpioConfig & 31) == 16) Serial.println("TX Data Input");
  else if ((gpioConfig & 31) == 20) Serial.println("RX Data Output");
  else Serial.println(gpioConfig & 31);
}

int i = 0;

void setupChipForOOKTransmit() {  
  
    /**********************   Configure more specific settings   *****************************************/  
  
  
    /*****************************************/
    /* Set max transmission power            */
    /*****************************************/
    rf22.setTxPower(RH_RF22_TXPOW_14DBM);
    delay(10);
  
    /*****************************************/
    /* Set up the modulation we want         */
    /*****************************************/
    //byte modulationControl2 = rf22.spiRead(RH_RF22_REG_71_MODULATION_CONTROL2);
    //delay(10);
    spiWriteWithACK(RH_RF22_REG_71_MODULATION_CONTROL2, 1); // Clear out the first 2 bits, then set to 01 (i.e. set modulation to OOK)
    delay(10);
  
    /*****************************************/
    /* Configure what GPIO pin 2 is used for */
    /*****************************************/
    uint8_t gpio2Config = rf22.spiRead(RH_RF22_REG_0D_GPIO_CONFIGURATION2);
    delay(10);
    spiWriteWithACK(RH_RF22_REG_0D_GPIO_CONFIGURATION2, gpio2Config | 16);   // Set it to use TX data directly from GPIO2
    
    
    /*************************************************/
    /* Set the frequency of the chip to 433MHz       */
    /*************************************************/
    rf22.spiWrite(RH_RF22_REG_75_FREQUENCY_BAND_SELECT, 64 | 19); // 64 = Side band select (this defaults to ON, so we do), 16 = the amount above 240MHz we want
    

    /*************************************************/
    /* Set the radio in TX mode                      */
    /*************************************************/
    rf22.setModeTx();
  
    /**************************************************/
    /*  Dump the current settings for us to verify    */
    /**************************************************/
    delay(1000);
    printDeviceStatus();
    printModulationControl();
    printTXPower();
    printGPIO2Config();
}

void setup() 
{
  Serial.begin(9600);
  
  pinMode(3, OUTPUT);
  
  while (!rf22.init()) {
    Serial.println("Failed to start RFM22 chip");
    delay(2000);
  } 
  
  inited = 1;
  Serial.println("Started RFM22 chip successfully");
  
  delay(500);
  
  setupChipForOOKTransmit();
}

void loop()
{  
  while (inited) {
  
    setupChipForOOKTransmit();
    byte theCommand[] = {'B', 'B', 'S', 'B', ' ', '1', ' ', 'A', ' ', (state ? '0' : '1')};
    handleBBSBCommand(writeHigh, writeLow, false, theCommand);
    
    delay(1000);
    byte theCommand2[] = {'B', 'B', 'S', 'B', ' ', '2', ' ', 'A', ' ', (state ? '0' : '1')};
    //handleBBSBCommand(writeHigh, writeLow, false, theCommand2);
  
    delay(30000);
    state = !state;
  }
  
  delay(59999);
}


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
#include <Ethernet.h>
#include <PubSubClient.h>

#define prog_char  char PROGMEM

const char string00[] PROGMEM = "local/rfm/bbsb/commands";
const char string01[] PROGMEM = "local/rfm/bbsb/status";
const char string02[] PROGMEM = "RFM22b BBSB hub: Connected OK\0";
const char string03[] PROGMEM = "RFM22b BBSB hub: Subscribed OK\0";
const char string04[] PROGMEM = "Connecting to MQTT...";
const char string05[] PROGMEM = "Subscribed OK";
const char string06[] PROGMEM = "Failed to subscribe";
const char string07[] PROGMEM = "Failed to connect to MQTT broker";
const char string08[] PROGMEM = "RFM22 chip init OK";
const char string09[] PROGMEM = "RFM22 chip init failed";
const char string10[] PROGMEM = "MQTT message received";
const char string11[] PROGMEM = " - Topic: ";
const char string12[] PROGMEM = " - Payload: ";
const char string13[] PROGMEM = "MQTT/BBSB command handled correctly";
const char string14[] PROGMEM = "Handling BBSB command";
const char string15[] PROGMEM = "Publishing MQTT message";
const char string16[] PROGMEM = "---- Device Status ----";
const char string17[] PROGMEM = "------ TX Power ------";
const char string18[] PROGMEM = "----- Modulation -----";
const char string19[] PROGMEM = "----- GPIO Config -----";
const char string20[] PROGMEM = "Chip state: TX";
const char string21[] PROGMEM = "Chip state: RX";
const char string22[] PROGMEM = "Chip state: Idle";
const char string23[] PROGMEM = "Pull up resistor: ";
const char string24[] PROGMEM = "Function: ";
const char string25[] PROGMEM = "Direct Digital RX Input";
const char string26[] PROGMEM = "Direct Digital TX Output";
const char string27[] PROGMEM = "TX Data Input";
const char string28[] PROGMEM = "RX Data Output";
const char string29[] PROGMEM = "Ethernet DHCP failed";
const char string30[] PROGMEM = "My IP address: ";
const char string31[] PROGMEM = "Modulation: ";
const char * const strings[] PROGMEM = {string00, string01, string02, string03, string04, string05, string06,
                                        string07, string08, string09, string10, string11, string12, string13,
                                        string14, string15, string16, string17, string18, string19, string20,
                                        string21, string22, string23, string24, string25, string26, string27,
                                        string28, string29, string30, string31};
                                       
char tempBuffer[40];

//#define SERVER_IP_ADDRESS { 192, 168, 137, 1 }
#define SERVER_IP_ADDRESS { 192, 168, 0, 2 }

// The template Cliend ID. The blanks are filled with the first
// eight bytes of the uuid.
#define CLIENTID "bbsbBridge"

byte state = 0;
byte inited = 0;
byte i = 0;


// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCE, 0xAE, 0x04
};
#ifdef SERVER_IP_ADDRESS
byte mqttServer[] = SERVER_IP_ADDRESS;
#else
//IPAddress server;
byte server[4];
#endif

// Singleton instance of the radio driver
RH_RF22 rf22 = RH_RF22(9);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient theClient;

char clientId[] = CLIENTID;

// Subscription callback; unused
void cb(char* topic, byte* payload, unsigned int length) {
  
  char message[100];
  strncpy(message, (char*)payload, length);
  message[length] = '\0';
  
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[10])));    // "MQTT message received"
  Serial.println(tempBuffer);
  
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[11])));    // "Topic:"
  Serial.print(tempBuffer);
  Serial.println(topic);
  
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[12])));    // "Message:"
  Serial.print(tempBuffer);
  Serial.println(message);
  
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[0])));     // "local/rfm/bbsb/commands"
  
  if (strncmp(topic, tempBuffer, 23) == 0) {
    
    strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[14])));  // "Handling BBSB command"
    Serial.println(tempBuffer);
    
    byte payloadLength = strstr(message, ":") >= 0 ? (strstr(message, ":") - message + 1) : -1;  // Take everything up to the ':' in the payload
    
    if (payloadLength > 0 && payloadLength < 40) {
      memcpy(message, payload, payloadLength);
      message[payloadLength] = '\0';
      Serial.println(message);
    
      byte rc = handleBBSBCommand(writeHigh, writeLow, false, (byte*)message);
    
      if (rc == 0) {
        strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[13])));  // "MQTT/BBSB command handled correctly"
        Serial.println(tempBuffer);
      
        memcpy(tempBuffer, "Command '", 9);
        memcpy(tempBuffer + 9, payload, payloadLength);
        memcpy(tempBuffer + 9 + payloadLength, "' handled correctly\0", 20);
      
        Serial.println(tempBuffer);
        publish("local/rfm/bbsb/status", tempBuffer, payloadLength + 9 + 20, 0);
      }
    }
  } 
}

PubSubClient mqttClient(mqttServer, 1883, cb, theClient);

void publish(char* topic, char* value, int len, int retained) {
  
  char message[50];
  strncpy(message, value, len);
  message[len] = '\0';
  
  tempBuffer[0] = '\0';
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[15])));     // "Publishing MQTT message"
  Serial.println(tempBuffer);
  
  tempBuffer[0] = '\0';
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[11])));     // " - Topic:"
  Serial.print(tempBuffer);
  Serial.println(topic);
  
  tempBuffer[0] = '\0';
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[12])));     // " - Payload:"
  Serial.print(tempBuffer);
  Serial.println(message);
  
  mqttClient.publish(topic, (uint8_t*)message, len+1, retained);    // +1 for the '\0' character
}

void writeHigh() {
  digitalWrite(3, HIGH);
}

void writeLow() {
  digitalWrite(3, LOW);
}

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
  
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[16])));  
  Serial.println(tempBuffer);
  
  Serial.print("Temp: ");
  uint8_t temp = rf22.temperatureRead();
  Serial.print((float)temp / 10);
  Serial.println(" deg");
  delay(10);
  
  uint8_t chipReady = rf22.spiRead(RH_RF22_REG_04_INTERRUPT_STATUS2) & 2;
  
  if ((status & 1) == 0 && (status & 2) == 0) {
    strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[22])));  
    Serial.println(tempBuffer);
  } else if ((status & 1) && (status & 2) == 0) {
    strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[21])));  
    Serial.println(tempBuffer);
  } else if ((status & 1) == 0 && (status & 2)) {
    strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[20])));  
    Serial.println(tempBuffer);
  }
  Serial.println((chipReady ? " (Ready)" : " (Not Ready)"));
  
  Serial.print("Freq: ");
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
    Serial.println("RX buff: Empty");
  } else {
    Serial.println("RX buff: Not empty");
  }
}

void printTXPower() {
  delay(50);
  uint8_t txPowerRegister = rf22.spiRead(RH_RF22_REG_6D_TX_POWER);
  
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[17])));  
  Serial.println(tempBuffer);
  
  //Serial.println(txPowerRegister & 7);
  Serial.print("Pwr: ");
  Serial.print(((txPowerRegister & 7) * 3) - 1);
  Serial.println("db");
}

void printModulationControl() {
  delay(50);
  uint8_t modulationControl = rf22.spiRead(RH_RF22_REG_71_MODULATION_CONTROL2);
  
  tempBuffer[0] = '\0';
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[18])));
  Serial.println(tempBuffer);
  
  tempBuffer[0] = '\0';
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[31])));
  Serial.print(tempBuffer);
  if ((modulationControl & 3) == 1) Serial.println("OOK");
  else if ((modulationControl & 3) == 2) Serial.println("FSK");
  else if ((modulationControl & 3) == 3) Serial.println("GFSK");
  else Serial.println("None");
}

void printGPIO2Config() {
  delay(50);
  byte gpioConfig = rf22.spiRead(RH_RF22_REG_0D_GPIO_CONFIGURATION2);
  
  tempBuffer[0] = '\0';
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[19])));
  Serial.println(tempBuffer);
  
  tempBuffer[0] = '\0';
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[23])));
  Serial.print(tempBuffer);
  Serial.println((gpioConfig & 32 ? " False" : " True"));
  
  tempBuffer[0] = '\0';
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[24])));
  
  Serial.print(tempBuffer);
  if ((gpioConfig & 31) == 3) {
    tempBuffer[0] = '\0';
    strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[25])));
    Serial.println(tempBuffer);
  } else if ((gpioConfig & 31) == 10) {
    tempBuffer[0] = '\0';
    strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[26])));
    Serial.println(tempBuffer);
  } else if ((gpioConfig & 31) == 16) {
    tempBuffer[0] = '\0';
    strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[27])));
    Serial.println(tempBuffer);
  } else if ((gpioConfig & 31) == 20) {
    tempBuffer[0] = '\0';
    strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[28])));
    Serial.println(tempBuffer);
  }
  else Serial.println(gpioConfig & 31);
}

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
    tempBuffer[0] = '\0';
    strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[9])));
    Serial.println(tempBuffer);
    delay(2000);
  } 
  
  tempBuffer[0] = '\0';
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[8])));
  Serial.println(tempBuffer);
  
  delay(500);
  
  setupChipForOOKTransmit();
  
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    tempBuffer[0] = '\0';
    strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[29])));
    Serial.println(tempBuffer);    
  }
  
  // Print our IP address
  tempBuffer[0] = '\0';
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[30])));
  Serial.print(tempBuffer);
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
  
  Serial.println();
    
  while (!inited) {
    tempBuffer[0] = '\0';
    strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[4]))); 
    Serial.println(tempBuffer);
    delay(20);
    mqttClient.connect(clientId);
  
    if (!mqttClient.connected()) {
      tempBuffer[0] = '\0';
      strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[7])));
      Serial.println(tempBuffer);
      inited = 0;
      delay(5000);
    } else {
      inited = 1;
      
      tempBuffer[0] = '\0';
      strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[2])));  // "RFM22b BBSB hub: Connected OK"
      publish("local/rfm/bbsb/status", tempBuffer, strlen(tempBuffer), 0);
      
      tempBuffer[0] = '\0';      
      strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[3])));  // "RFM22b BBSB hub: Subscribed OK"
      publish("local/rfm/bbsb/status", tempBuffer, strlen(tempBuffer), 0);
      
      tempBuffer[0] = '\0';
      strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[0])));  // "local/rfm/bbsb/commands"
      bool rc = mqttClient.subscribe(tempBuffer);
      
      if (rc) { 
        tempBuffer[0] = '\0';
        strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[5])));  // "Subscribed OK"
        Serial.println(tempBuffer);
      } else {
        tempBuffer[0] = '\0';
        strcpy_P(tempBuffer, (char*)pgm_read_word(&(strings[6])));  // "Failed to subscribe"
        Serial.println(tempBuffer);
      }
    }
  }
  
  Serial.println();
}

void loop()
{  
  while (inited) {
    mqttClient.loop();
    delay(500);
  }
  
  delay(59999);
}


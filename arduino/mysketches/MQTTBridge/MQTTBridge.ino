/*
  CurrentCost MQTT Bridge

 Example sketch for the CurrentCost Bridge device that causes 
 readings to be published over MQTT.
 
 Requires:
  - Arduino 0022
  - PubSubClient v1.6+
      http://knolleary.net/arduino-client-for-mqtt/
  - Ethernet DHCP/DNS
      http://www.mcqn.com/files/Ethernet-DHCP-DNS.zip

 This must be compiled with the board set to:
   Arduino Pro or Pro Mini (3.3V, 8MHz) w/ ATmega328

 Nicholas O'Leary,  04-2011
 http://knolleary.net/?p=1024

 This code is in the public domain
*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

// Use either SERVER_HOST_NAME or SERVER_IP_ADDRESS - not both
#define _SERVER_HOST_NAME "realtime.ngi.ibm.com"
#define SERVER_IP_ADDRESS { 192, 168, 0, 2 }
#define MY_IP_ADDRESS { 192, 168, 0, 120 }
// The port to connect to
#define SERVER_PORT 1883

// The template Cliend ID. The blanks are filled with the first
// eight bytes of the uuid.
#define CLIENTID "ccMRWBridge"

// The template topic. The second topic-level is filled with the
// uuid of the bridge. The third/fourth levels are populated depending
// what is being published
#define TOPIC "local/CC/bridge/   "
// The char position at which the third topic level starts
#define TOPIC_ROOT_LENGTH 16

// Should the temperature be published
#define PUBLISH_TEMPERATURE
// Should birth/will messages be used
#define PUBLISH_CONNECTION_STATE

// Ethernet reset line attached to pin 7
#define ETHERNET_RESET 7 


// Struct to store a reading in
typedef struct {
  char sensor;
  char channel;
  char value[10];
} Reading;

// A single message my contain up to three channels
Reading readings[3];
byte readingCount = 0;

char powertopic[] = TOPIC;
char clientId[] = CLIENTID;


byte mac[] = {0x00, 0xF2, 0x3E, 0x4F, 0xCE, 0x02 };
byte ip[] = MY_IP_ADDRESS;
#ifdef SERVER_IP_ADDRESS
byte server[] = SERVER_IP_ADDRESS;
#else
//IPAddress server;
byte server[4];
#endif

#define STATE_INITIAL 0
#define STATE_IN_MSG 1
byte state = STATE_INITIAL;

char temperature[5];
int lastPublishedTemperature;
EthernetClient theClient;
SoftwareSerial currentCost(14, 300);

PubSubClient mqttClient(server, SERVER_PORT,cb, theClient);

// Subscription callback; unused
void cb(char* topic, byte* payload,unsigned int length) {
}

// Returns the next byte from the meter
// Blocks until there is something to return
char get_byte() {
  int a = -1; 
  while((a = currentCost.read()) == -1) {
  };
  
  return a;
}

// Handle a reading; causes it to be published
// to the approprite topic
void handleReading(char sensor, char channel, char* reading) {
  if (sensor != '\0' && channel != '\0') {
    char readingLength = 5;
    if (channel == '0') {
      readingLength = 10;
    }
    boolean validReading = true;
    for (int i=0;i<readingLength;i++) {
      validReading = (validReading && isDigit(reading[i]));
    }
    if (validReading) {
      powertopic[TOPIC_ROOT_LENGTH] = sensor;
      powertopic[TOPIC_ROOT_LENGTH+1] = '/';
      powertopic[TOPIC_ROOT_LENGTH+2] = channel;
      publish(powertopic,reading,readingLength,0);
    }
  }
}

void publish(char* topic, char* value, int len, int retained) {
  //Serial.write("\nPublishing to topic:");
  //Serial.write(topic);
  //Serial.write("\n");
  mqttClient.publish(topic,(uint8_t*)value,len,retained);
}

void setup() {

  delay(500);
  currentCost.begin(57600);
  //Serial.begin(57600);
  //Serial.write("Started");
}

int resetConnection() {
  //Serial.write("\nResetting LAN card\n");
  // Reset Ethernet shield in a controlled manner
  pinMode(ETHERNET_RESET,OUTPUT);
  digitalWrite(ETHERNET_RESET,LOW);
  delay(250);
  digitalWrite(ETHERNET_RESET,HIGH);
  pinMode(ETHERNET_RESET,INPUT);
  delay(500);
  //Serial.write("\nDone\n");
    
  // Get an IP address - will timeout after 1 minute
  
  //Serial.write("\nEthBegin\n");
  Ethernet.begin(mac, ip);
  mqttClient.connect(clientId);
  publish("local/CC/bridge/status","back",4,0);
  
  return 0;
}

void loop()
{
  // Check we're still connected
  if (!mqttClient.connected()) {
    //Serial.write("\nConnecting");
    resetConnection();
    
    // If still not connected, try a different IP address
    if (!mqttClient.connected()) {
      ip[3]++;
      if (ip[3] > 200) ip[3] = 100;
      resetConnection();
    }
  } else {
  
  readingCount = 0;
  
  //Serial.write("\nDone\n");
  state = STATE_INITIAL;
  while (state == STATE_INITIAL) {
    // Scan for </time
    while (get_byte() != '<') {}
    if (get_byte() == '/' && get_byte() == 't' && 
        get_byte() == 'i' && get_byte() == 'm' && 
        get_byte() == 'e') {
      state = STATE_IN_MSG;
    }
    mqttClient.loop();
  }
  get_byte(); // '>'
  get_byte(); // '<'
  if (get_byte() == 'h') {
    // skip history messages
    state = STATE_INITIAL;
  } 
  else {
    for (int i=0;i<4;i++) { // 'mpr>'
      get_byte();
    }
    for (int i=0;i<4;i++) {
      temperature[i]=get_byte();
    }
    temperature[4] = '\0';
    char sensor = 0;
    while (state == STATE_IN_MSG) {
      while (get_byte() != '<') {}
      char c = get_byte();
      if (c == 's' && get_byte() == 'e' && get_byte() == 'n' && 
          get_byte() == 's' && get_byte() == 'o' && get_byte() == 'r') {
        get_byte();
        sensor = get_byte();
      }
      else if (c == 'c' && get_byte() == 'h') {
        memset(&(readings[readingCount]),0,sizeof(Reading));
        readings[readingCount].sensor = sensor;
        readings[readingCount].channel = get_byte();
        for (int i=0;i<8;i++) { // '><watts>'
          get_byte();
        }
        for (int i=0;i<5;i++) {
          readings[readingCount].value[i] = get_byte();
        }
        readingCount++;
      }
      else if (c == 'i' && get_byte() == 'm' && get_byte() == 'p') {
        get_byte(); // '>'
        memset(&(readings[readingCount]),0,sizeof(Reading));
        readings[readingCount].sensor = sensor;
        readings[readingCount].channel = '0';
        for (int i=0;i<10;i++) {
          readings[readingCount].value[i] = get_byte();
        }
        readingCount++;
      }
      else if (c == '/' && get_byte() == 'm') {
        // End of a message
        
        for (int i=0;i<readingCount;i++) {
          handleReading(readings[i].sensor,readings[i].channel,readings[i].value);
        }
        readingCount = 0;
#ifdef PUBLISH_TEMPERATURE
        if (isDigit(temperature[0]) &&
            isDigit(temperature[1]) &&
            temperature[2]=='.' &&
            isDigit(temperature[3])) {
          int temperatureValue = atoi(temperature)*10+atoi(&(temperature[3]));
          if (abs(temperatureValue-lastPublishedTemperature) > 4) {
            powertopic[TOPIC_ROOT_LENGTH] = 't';
            powertopic[TOPIC_ROOT_LENGTH+1] = 'm';
            powertopic[TOPIC_ROOT_LENGTH+2] = 'p';
            publish(powertopic,temperature,4,1);
            lastPublishedTemperature = temperatureValue;
          }
        }
#endif
        state = STATE_INITIAL;
      }
    }
  } 
  }
}

/*
  Firmware for Arduino-based USB/LCD interface with Ethernet adapter
  
  @copyright Matthew Whitehead 2014 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>
//#include <PubSubClient.h>

// Comment out to make production build
#define DEV

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

prog_char versionNumber[] PROGMEM = "Version: 0.1.0";
PROGMEM const char *miscStrings[] = {versionNumber};

// Strings for displaying on the LCD - stored in flash memory to save space in SRAM
prog_char menuItem01[] PROGMEM = "        Menu"; 
prog_char menuItem02[] PROGMEM = "- View network info";
prog_char menuItem03[] PROGMEM = "- Ping server";
prog_char menuItem04[] PROGMEM = "- Show settings";
prog_char menuItem05[] PROGMEM = "- Restore defaults";
prog_char menuItem06[] PROGMEM = "- Version info";
PROGMEM const char *menuStrings[] =
{   
  menuItem01,
  menuItem02,
  menuItem03,
  menuItem04,
  menuItem05,
  menuItem06
};

// Strings for performing HTTP over TCP, plus related LCD text
prog_char httpString01[] PROGMEM = "GET / HTTP/1.1"; 
prog_char httpString02[] PROGMEM = "Host: www.google.com";
prog_char httpString03[] PROGMEM = "Connection: close";
prog_char httpString04[] PROGMEM = "www.google.com";
prog_char httpString05[] PROGMEM = "Connecting...";
prog_char httpString06[] PROGMEM = "Reading data...";
prog_char httpString07[] PROGMEM = "Ping Success!";
PROGMEM const char *httpStrings[] =
{   
  httpString01,
  httpString02,
  httpString03,
  httpString04,
  httpString05,
  httpString06,
  httpString07
};

// Static strings for sending to a connected browser
prog_char httpResponseString01[] PROGMEM = "HTTP/1.1 200 OK"; 
prog_char httpResponseString02[] PROGMEM = "Content-Type: text/html";
prog_char httpResponseString03[] PROGMEM = "Connection: close";
prog_char httpResponseString04[] PROGMEM = "<html><h3 style=\"font-family:arial\">Settings Saved</h3></html>";
prog_char httpResponseString05[] PROGMEM = "<!DOCTYPE HTML>";
prog_char httpResponseString06[] PROGMEM = "<html>";
prog_char httpResponseString07[] PROGMEM = "<title>Config Utility</title>";
prog_char httpResponseString08[] PROGMEM = "<body style=\"font-family:arial\">";
prog_char httpResponseString09[] PROGMEM = "<h3>Config Utility</h3><p></p>";
prog_char httpResponseString10[] PROGMEM = "<p> "; // Place to insert the "Version: 1.0.0" text
prog_char httpResponseString11[] PROGMEM = "</p><p></p>";
prog_char httpResponseString12[] PROGMEM = "<hr align=\"left\" width=\"300\"><p></p>";
prog_char httpResponseString13[] PROGMEM = "<form name=\"form1\" action=\"\" method=\"post\">";
prog_char httpResponseString14[] PROGMEM = "<label for=\"bltimer\">Backlight timer (seconds)</label>";
prog_char httpResponseString15[] PROGMEM = "<input type=\"text\" name=\"blttimer\" id=\"blttimer\" value=\"";
prog_char httpResponseString16[] PROGMEM = "\" maxlength=\"2\" size=\"8\"/><p></p>";
prog_char httpResponseString17[] PROGMEM = "<label for=\"data\">Some other data</label>";
prog_char httpResponseString18[] PROGMEM = "<input type=\"text\" name=\"data\" id=\"data\" maxlength=\"10\" size=\"10\"/><p></p>";
prog_char httpResponseString19[] PROGMEM = "<hr align=\"left\" width=\"300\"><p></p>";
prog_char httpResponseString20[] PROGMEM = "<input type=\"submit\" value=\"Save\"/>";
prog_char httpResponseString21[] PROGMEM = "</form>";
prog_char httpResponseString22[] PROGMEM = "</body>";
prog_char httpResponseString23[] PROGMEM = "</html>";
PROGMEM const char *httpResponseStrings[] =
{   
  httpResponseString01,httpResponseString02,httpResponseString03,
  httpResponseString04,httpResponseString05,httpResponseString06,
  httpResponseString07,httpResponseString08,httpResponseString09,
  httpResponseString10,httpResponseString11,httpResponseString12,
  httpResponseString13,httpResponseString14,httpResponseString15,
  httpResponseString16,httpResponseString17,httpResponseString18,
  httpResponseString19,httpResponseString20,httpResponseString21,
  httpResponseString22,httpResponseString23
};

// Error strings for sending to a connected browser
prog_char httpErrorString01[] PROGMEM = "HTTP/1.1 413 Request Entity too Large"; 
prog_char httpErrorString02[] PROGMEM = "Content-Type: text/html";
prog_char httpErrorString03[] PROGMEM = "Connection: close";
PROGMEM const char *httpErrorStrings[] =
{   
  httpErrorString01,httpErrorString02,httpErrorString03
};
  
byte numberOfMessages = 0;
byte messageCursor = 0;
char messages[6][63];
byte serialRXComplete = 1; // RX complete
byte serialRXCursor = 0;
byte backlightOn = 0;
long timeSinceLastLiveConnAttempt = 0;

/* Structure defining the function a menu item calls */
struct menuItem {
  byte menuStringNumber;
  void (*func) ();
};

/* Structure defining a menu you can navigate through */
struct menu {
  byte numberOfItems;
  byte selectionCursor;
  struct menuItem * menuItems;
  struct menu * parentMenu;
};

/* List of menus we use */
struct menu menus[1];

/* The current menu the user's on and the previous one */
struct menu *currentMenu;
byte insideMenuFunction = 0;

#define BACKLIGHTPIN 9
#define DEFAULTBACKLIGHTTIME 10
#define MAXBACKLIGHTTIME 30

byte buttonReadState = 0;
byte buttonState = 0;

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;
EthernetServer server(80);

long timeSinceBacklightOn = 0;

// Increment the cursor. The cursor goes from 0 to N, but the
// cursor has 1 added to it when accessing the menu text array
void incMenuCursor(struct menu *theMenu) {
  if (theMenu->selectionCursor < (theMenu->numberOfItems - 1)) {
    theMenu->selectionCursor++;
  }
}

void decMenuCursor(struct menu *theMenu) {
  if (theMenu->selectionCursor > 0) {
    theMenu->selectionCursor--;
  }
}

void showCurrentMenu() {
  // Temporary buffer for storing text read from flash memory. 
  // Max 20 chars cos that's how wide the LCD is
  char tempBuffer[20];
  
  lcd.clear();
  lcd.setCursor(0, 0);
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(menuStrings[currentMenu->menuItems[0].menuStringNumber])));
  lcd.print(tempBuffer); // The first item in a menu struct is always the heading
  
  byte menuDisplayCursor = currentMenu->selectionCursor < (currentMenu->numberOfItems - 3) ? currentMenu->selectionCursor : currentMenu->numberOfItems - 3; 
  
  // Show the lines after the menu title
  lcd.setCursor(0, 1);
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(menuStrings[currentMenu->menuItems[menuDisplayCursor + 1].menuStringNumber])));
  lcd.print(tempBuffer);
  lcd.setCursor(0, 2);
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(menuStrings[currentMenu->menuItems[menuDisplayCursor + 2].menuStringNumber])));
  lcd.print(tempBuffer);
  lcd.setCursor(0, 3);
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(menuStrings[currentMenu->menuItems[menuDisplayCursor + 3].menuStringNumber])));
  lcd.print(tempBuffer);
  
  // Show the cursor on the selected menu item
  int menuSelectionCursor = 3 - (currentMenu->numberOfItems - currentMenu->selectionCursor);
  int lineForCursor = menuSelectionCursor <= 0 ? 1 : menuSelectionCursor + 1;
  lcd.setCursor(1, lineForCursor);
  lcd.print(">");
}

void turnBacklightOn() {
  timeSinceBacklightOn = millis();
  analogWrite(BACKLIGHTPIN, 230);
  backlightOn = 1;
}

void turnBacklightOff() {
  analogWrite(BACKLIGHTPIN, 0);
  backlightOn = 0;
}

void commonButtonFunctions() {
  turnBacklightOn();
}

void printNetworkSettings() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected to network");
  lcd.setCursor(0, 1);
  lcd.print("IP: ");
  byte dots = 0;
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    lcd.print(Ethernet.localIP()[thisByte]);
    
    if (dots < 3) {
      lcd.print(".");
      dots++;
    }
  }
}

void pingServer() {
  
  // Buffer for reading strings from flash memory
  char tempBuffer[25];
    
  lcd.clear();
  lcd.setCursor(0, 0);
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(httpStrings[4])));   //   "Connecting..."
  lcd.print(tempBuffer);
  
  client.flush();
  
  if (client.connected()) {
    while (client.available()) {
      client.read();
    }
  }
  
  client.stop();
  
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(httpStrings[3])));
  int connected = client.connect(tempBuffer, 80); // www.google.com
  
  if (connected) {
    lcd.print("OK");
    strcpy_P(tempBuffer, (char*)pgm_read_word(&(httpStrings[0])));
    client.println(tempBuffer);  // GET / HTTP/1.1
    strcpy_P(tempBuffer, (char*)pgm_read_word(&(httpStrings[1])));
    client.println(tempBuffer);  //  Host: www.google.com
    strcpy_P(tempBuffer, (char*)pgm_read_word(&(httpStrings[2])));
    client.println(tempBuffer); //  Connection: close
    client.println();
  
    lcd.setCursor(0, 1);
    strcpy_P(tempBuffer, (char*)pgm_read_word(&(httpStrings[5])));  //   "Reading..."
    lcd.print(tempBuffer);
  
    int dataLength = 0;
    char dataLengthTextInKB[6];
    char units = 'B';
  
    while (dataLength < 1) {
      while (client.available()) {
        char c = client.read();
        dataLength++;
      }
    }
  
    if (dataLength > 999) {
      units = 'K';
      dataLength = dataLength / 1000;
    } else if (dataLength > 999999) {
      units = 'M';
      dataLength = dataLength / 1000000;
    }
  
    sprintf(dataLengthTextInKB, "%d%c", dataLength, units);
  
    lcd.print(dataLengthTextInKB);
    
    lcd.setCursor(0, 2);
    strcpy_P(tempBuffer, (char*)pgm_read_word(&(httpStrings[6])));  //   "Ping Success!"
    lcd.print(tempBuffer);
  } else {
    lcd.print("FAILED");
  }
  client.stop();
  
  timeSinceLastLiveConnAttempt = millis() - 60000;
}

/*
 * Show basic build information on screen
 */
void showVersionInfo() {
  char tempBuffer[21];
  tempBuffer[0] = '\0';
  strcpy_P(tempBuffer, (char*)pgm_read_word(&(miscStrings[0])));  // Copy version information out of flash strings
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(tempBuffer);
}

/*
 * Define the actions to take when OK is pressed
 */
void okButtonPressed() {
  commonButtonFunctions();
  
  // If we're in a menu, call the next function. Otherwise do nothing for the time being.
  if (currentMenu != 0) {
    insideMenuFunction = 1;
    (currentMenu->menuItems[currentMenu->selectionCursor + 1].func)();
  }
}

/*
 * Define the actions to take when CANCEL is pressed
 
 */
void cancelButtonPressed() {
  commonButtonFunctions();
  
  if (currentMenu == 0) {
    // We're not currently in a menu - so load the main menu
    currentMenu = &menus[0];
    currentMenu->selectionCursor = 0;
    showCurrentMenu();
  } else {
    // We're either in a menu screen, or we've run a menu function
    
    if (insideMenuFunction) {
      // Easy - just re-load the current menu
      insideMenuFunction = 0;
      showCurrentMenu();
    } else {
      // We're in a menu. Is it the top one?
      if (currentMenu->parentMenu == 0) {
        currentMenu = 0;
        showMessages();
      } else {
        // Show the parent menu
        currentMenu = currentMenu->parentMenu;
        showCurrentMenu;
      }
    }
  }
}

/*
 * Define the actions to take when the RIGHT button is pressed
 */
void rightButtonPressed() {
  commonButtonFunctions();
  
  if (currentMenu != 0) {
    incMenuCursor(currentMenu);
    showCurrentMenu();
  } else {
    // We're in the messages view
    if (messageCursor < (numberOfMessages - 1)) {
      messageCursor++;
      showMessages();
    }
  }
}

/*
 * Define the actions to take when the LEFT button is pressed
 */
void leftButtonPressed() {
  commonButtonFunctions();
  
  if (currentMenu != 0) {
    decMenuCursor(currentMenu);
    showCurrentMenu();
  } else {
    // We're in the messages view
    if (messageCursor > 0) {
      messageCursor--;
      showMessages();
    }
  }
}

void (*buttonFunctions[4]) ();

void showConnectionState() {
  lcd.setCursor(16, 0);
  lcd.print("[");
  lcd.setCursor(19, 0);
  lcd.print("]");
  lcd.setCursor(17, 0);
  if (client.connected() && ((millis() - timeSinceLastLiveConnAttempt) < 15000)) {
    //Serial.println("Already connected, no need to do anything");
  } else {
    //Serial.println("Not connected, testing the connection properly if it was a while since we last tried");
    if (millis() - timeSinceLastLiveConnAttempt > 30000) {
      client.stop();
      
      char tempBuffer[20];
      strcpy_P(tempBuffer, (char*)pgm_read_word(&(httpStrings[3])));  // Copy "www.google.com" out of flash strings
      client.connect(tempBuffer, 80); // Connects to www.google.com, and returns 1 for success
      timeSinceLastLiveConnAttempt = millis();
    } else {
      //Serial.println("Not retrying yet - too frequent");
    }
  }
  
  if (client.connected()) {
    lcd.print((char)127);
    lcd.print((char)126);
  } else {
    lcd.print("  ");
  }
}

void showMessages() {
  char messageTitle[21];
  messageTitle[20] = '\0';
  
  // Reset our position in the menu structure to the root menu
  currentMenu = 0;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  
  sprintf(messageTitle, "Msgs (%d/%d)", messageCursor + 1, numberOfMessages);
  lcd.print(messageTitle);
  
  if (numberOfMessages > 0) {
    printText(messages[messageCursor]);
  }
  
  showConnectionState();
}

void printText(char *message) {
  char nextLine[21];
  nextLine[20] = '\0';
  byte messageLength = strlen(message);
  
  // First line
  strncpy(nextLine, message, 20);
  lcd.setCursor(0, 1);
  lcd.print(nextLine);
  
  // Second line
  if (messageLength > 20) {
    strncpy(nextLine, message + 20, 20);
    lcd.setCursor(0, 2);
    lcd.print(nextLine);
  }
  
  // Third line
  if (messageLength > 40) {
    strncpy(nextLine, message + 40, 20);
    lcd.setCursor(0, 3);
    lcd.print(nextLine);
  }
}

/*
 * See if there is any data available to read
 * from serial (USB) in. If there is, read it
 * and process it accordingly.
 */
void checkForSerialData() {
  while (Serial.available()) {
    messages[numberOfMessages][serialRXCursor] = Serial.read();
    serialRXComplete = 0; // Not complete yet
    
    if (messages[numberOfMessages][serialRXCursor] == ':') {
      // Message complete
      messages[numberOfMessages][serialRXCursor] = '\0';
      serialRXComplete = 1;
      serialRXCursor = 0;
      numberOfMessages++;
      showMessages();
      break;
    } else {
      // Increment the cursor in our buffer. If we've hit our limit
      // then go back to zero and start filling from there again until
      // we hit a colon.
      serialRXCursor++;
      if (serialRXCursor == 60) {
        serialRXCursor = 0;
      }
    }
  }
}

/*
 * Reset to default settings
 */
void resetToDefaults() {
  //Serial.println("Resetting to defaults");
  EEPROM.write(0, 255);
  EEPROM.write(1, 255);
  EEPROM.write(2, 255);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reset to factory");
  lcd.setCursor(0, 1);
  lcd.print("settings?");
}

/*
 * Test method
 */
void showSettings() {
  char propertyValue[30];
  propertyValue[0] = '\0';
  readProperty("data", propertyValue);
  if (strlen(propertyValue) > 0) {
    //Serial.println("Found data value");
    //Serial.println(propertyValue); 
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Data: ");
    lcd.print(propertyValue);
  }

  propertyValue[0] = '\0';
  readProperty("blttimer", propertyValue);
  if (strlen(propertyValue) > 0) {
    //Serial.println("Found blttimer value");
    //Serial.println(propertyValue);
    lcd.setCursor(0, 1);
    lcd.print("Backlight: ");
    lcd.print(propertyValue); 
    lcd.print("(s)");
  }
  
  propertyValue[0] = '\0';
  readProperty("data", propertyValue);
  if (strlen(propertyValue) > 0) {
    //Serial.println("Found data value");
    //Serial.println(propertyValue); 
  }
  
  propertyValue[0] = '\0';
  readProperty("unknown", propertyValue);
  if (strlen(propertyValue) > 0) {
    //Serial.println("Found unknown value");
    //Serial.println(propertyValue); 
  }
}

/*
 * Read the URL-encoded post form data from EEPROM and print it out.
 *
 * The first 3 bytes of EEPROM are reserved for counter data (bytes 1 and 2) and data length (byte 3).
 *
 * We only read up to 50 chars of property value and then give up to prevent buffer overflow.
 */
void readProperty(char* propertyName, char* propertyValue) {
  short byte0 = EEPROM.read(0);
  short byte1 = EEPROM.read(1);
  short dataLength = EEPROM.read(2);
  short dataCursor = (byte0 << 8) | (byte1);
  
  // (255 & 255) is -1
  if (dataCursor == -1) {
    // No settings saved yet
    //Serial.println("No settings saved yet");
  } else {
    //Serial.println("Reading settings...");
    //Serial.print("Byte 0: ");
    //Serial.println(byte0);
    //Serial.print("Byte 1: ");
    //Serial.println(byte1);
    //Serial.print("Byte 2: ");
    //Serial.println(dataLength);
    //Serial.print("Data: ");
    for (short i = 0; i < dataLength; i++) {
      //Serial.print((char)EEPROM.read(dataCursor + i));
    }
    //Serial.println("");
    
    // Read each property into a local array one at a time
    short localCursor = 0;
    
    for (short i = 0; i < dataLength; i++) {
      
      // Only read up to 50 bytes of property name, otherwise we could get buffer overflow
      if (localCursor < 50) {
        propertyValue[localCursor] = EEPROM.read(dataCursor + i);
      
        if (propertyValue[localCursor] == '&' || i == (dataLength - 1)) {
        
          if (i == (dataLength - 1)) {
            propertyValue[localCursor+1] = '\0';
            //Serial.println("Found end data");
          } else {
            propertyValue[localCursor] = '\0';
            //Serial.println("Found an ampersand");
          }
        
          char* propertyNameLocation = strstr(propertyValue, propertyName);
        
          if (propertyNameLocation != NULL) {
            //Serial.println("Found prop we want");
            // The property we just read from EEPROM is the one we were looking for
            char* propertyValueLocation = strstr(propertyValue, "=");
          
            if (propertyValueLocation != NULL) {
              //Serial.println("Found = sign");
              // We've now moved on to the '=' which means the remainder is the property value
              strcpy(propertyValue, propertyValueLocation+1);
              //Serial.println("Property value found");
              //Serial.println(propertyValue);
              break;
            } else {
              // There's something wrong! There should be an '=' sign. Print some error here?
            }
          } else {
            // We'e found a property which isn't the one we're looking for
            propertyValue[0] = '\0';
            localCursor = 0;
          }
        } else {
          localCursor++;
        }
      } else {
        break;
      }
    }
  }
  
  //lcd.clear();
  //lcd.setCursor(0, 0);
  //lcd.print("Settings read");
  //lcd.setCursor(0, 1);
  //lcd.print(propertyValue);
}

/*
 * Take a URL-encoded post form string and
 * save each of the individual properties to
 * EEPROM.
 */
void saveSettings(char * httpFormData) {
  
  if (strlen(httpFormData) > 256) {
    // This is too much data - we don't want to store it. Shouldn't ever happen.
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Config data too long");
    turnBacklightOn();
  } else {
    //Serial.println("Saving settings:");
  
    //Serial.println("3 b b s...");
    //Serial.println(EEPROM.read(0));
    //Serial.println(EEPROM.read(1));
    //Serial.println(EEPROM.read(2));
  
    // The first 2 bytes in EEPROM indicate (big endian) where the data should be
    // stored. On every re-write of the settings the counter is incremented
    // so we rotate the section of EEPROM that data is stored to. EEPROM has
    // 100k writes before it's beyond it's guaranteed lifetime so this 
    // extends it by rotating through the different sections of EEPROM.
    short previousDataCursor = (EEPROM.read(0) << 8) | EEPROM.read(1);
    short previousDataLength = EEPROM.read(2);
    short newDataLength = strlen(httpFormData);
    short newDataCursor = 0;
  
    if (newDataLength > 256) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Error. Settings too");
      lcd.setCursor(0, 1);
      lcd.print("long for EEPROM");
      delay(10000);
    }
  
    // FYI the 3rd byte in EEPROM indicates the saved data length.
  
    // If the first 2 bytes are 255 each, it means the factory defaults are
    // set. Now we can set the cursor to zero.
    if (previousDataCursor == -1) {
      //Serial.println("Setting cursor bytes to zero");
      EEPROM.write(0, 0);
      EEPROM.write(1, 0);
      newDataCursor = 3;
    } else {
      // If the cursor is initialised to some value, move it on
      // past the previous data length so we're writing to a new area in EEPROM
      newDataCursor = previousDataCursor + previousDataLength;
      if ((newDataCursor + newDataLength) > 1023) {
        newDataCursor = 3;
      }
    }
  
    //Serial.print("New cursor = ");
    //Serial.println(newDataCursor);
  
    //Serial.print("New length = ");
    //Serial.println(newDataLength);
  
    // Now store the new cursor and new data length back in the first 3 bytes
    EEPROM.write(0, (newDataCursor >> 8) & 255);
    EEPROM.write(1, newDataCursor & 255);
    if (newDataLength != previousDataLength) EEPROM.write(2, newDataLength);
  
    //Serial.println("f 3 b a s...");
    //Serial.println(EEPROM.read(0));
    //Serial.println(EEPROM.read(1));
    //Serial.println(EEPROM.read(2));
  
    //Serial.println("The data is: ");
    //Serial.println(httpFormData);
  
    //Serial.println("Writing the data...");
    for (short i = 0; i < newDataLength; i++) {
      EEPROM.write(newDataCursor + i, httpFormData[i]);
    }
    //Serial.println("Saved");
  
    turnBacklightOn();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Settings Saved");
  }
}

/*
 * Take an ethernet client and consume chars from
 * it until we have reached the end of the HTTP headers
 */
void skipHTTPHeaders(EthernetClient serverClient) {
  
  char previousChar = '\0';
  char currentChar = '\0';
  
  while (serverClient.connected() && serverClient.available()) {
    currentChar = serverClient.read();
    //Serial.write(currentChar);
    if (currentChar == '\n' && previousChar == '\n') {
      // 2 newlines in a row - the next char will be HTTP post data
      break;
    }
    
    if (currentChar == '\r') {
      // Ignore
    } else {
      previousChar = currentChar;
    }
  }
}

/*
 * Open port 80, see if anyone connects, and
 * if they do handle their HTTP request.
 */
void checkForHTTPConnections() {
  // See if a client has attached to port 80
  EthernetClient serverClient = server.available();
  
  if (serverClient) {
    char buffer[100];
    char dataFromBrowser[100];
    boolean newSettingsReceived = false;
    
    // Skip the HTTP headers
    skipHTTPHeaders(serverClient);
    short i = 0;
    while (serverClient.connected()) {
      while (serverClient.available() && i < 100) {
        char c = serverClient.read();
        //Serial.write(c);
        dataFromBrowser[i++] = c;
      }
      
      // If thre's still data available after 100 chars, we don't want it. Give up.
      if (serverClient.available()) {
        // Send the HTTP response headers
        strcpy_P(buffer, (char*)pgm_read_word(&(httpErrorStrings[0])));  // Copy "HTTP/1.1 413 Request Entity too Large" out of flash strings
        serverClient.println(buffer);
        strcpy_P(buffer, (char*)pgm_read_word(&(httpErrorStrings[1])));  // Copy "Content-Type: text/html" out of flash strings
        serverClient.println(buffer);
        strcpy_P(buffer, (char*)pgm_read_word(&(httpErrorStrings[2])));  // Copy "Connection: close" out of flash strings
        serverClient.println(buffer);  // the connection will be closed after completion of the response
        serverClient.println();
      } else {
        dataFromBrowser[i] = '\0';
      
        newSettingsReceived = i > 0;
      
        if (newSettingsReceived) {
          saveSettings(dataFromBrowser);
        }
      
        // Send the HTTP response headers
        strcpy_P(buffer, (char*)pgm_read_word(&(httpResponseStrings[0])));  // Copy "HTTP/1.1 200 OK" out of flash strings
        serverClient.println(buffer);
        strcpy_P(buffer, (char*)pgm_read_word(&(httpResponseStrings[1])));  // Copy "Content-Type: text/html" out of flash strings
        serverClient.println(buffer);
        strcpy_P(buffer, (char*)pgm_read_word(&(httpResponseStrings[2])));  // Copy "Connection: close" out of flash strings
        serverClient.println(buffer);  // the connection will be closed after completion of the response
        serverClient.println();
      
        if (newSettingsReceived) {
          // We read a response from the HTML form so just return something like <html>Settings Saved</html>
          strcpy_P(buffer, (char*)pgm_read_word(&(httpResponseStrings[4])));
          serverClient.println(buffer);
          strcpy_P(buffer, (char*)pgm_read_word(&(httpResponseStrings[3])));
          serverClient.println(buffer);
        } else {
          // No form data received on this connection so must be a request for the HTML form
          for (int i = 4; i < 20; i++) {
            strcpy_P(buffer, (char*)pgm_read_word(&(httpResponseStrings[i])));  // Copy each line of HTML text out of flash strings
            serverClient.println(buffer);
          
            // Special case where we insert variable data
            if (i == 9) {
              buffer[0] = '\0';
              strcpy_P(buffer, (char*)pgm_read_word(&(miscStrings[0])));  // Copy the version information into the HTML response
              serverClient.println(buffer);
            }
            
            // Special case where we insert the value of the blttimer property
            if (i == 14) {
              buffer[0] = '\0';
              readProperty("blttimer", buffer); // Copy the blttimer property into the HTML response
              serverClient.println(buffer);
            }
          }
        }
      }
      
      // Flush data to the client
      serverClient.flush();
      
      // Give the browser a chance to receive the data
      delay(10);
      
      // Close the connection
      serverClient.stop();
      
      if (newSettingsReceived) {
        delay(2500);
        showMessages();
      }
    }    
  }
}

/*
 * Standard arduino startup routine
 */
void setup() {
  byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
  
  buttonFunctions[0] = okButtonPressed;
  buttonFunctions[1] = cancelButtonPressed;
  buttonFunctions[2] = leftButtonPressed;
  buttonFunctions[3] = rightButtonPressed;
  
  menus[0].numberOfItems = 5;
  menus[0].menuItems = (menuItem*) calloc(6, sizeof(struct menuItem));
  menus[0].parentMenu = 0; // Special case for the top menu
  menus[0].menuItems[0].menuStringNumber = 0;
  menus[0].menuItems[1].menuStringNumber = 1;
  menus[0].menuItems[2].menuStringNumber = 2;
  menus[0].menuItems[3].menuStringNumber = 3;
  menus[0].menuItems[4].menuStringNumber = 4; 
  menus[0].menuItems[5].menuStringNumber = 5;
  
  menus[0].menuItems[1].func = &printNetworkSettings;
  menus[0].menuItems[2].func = &pingServer;
  menus[0].menuItems[3].func = &showSettings;
  menus[0].menuItems[4].func = &resetToDefaults;
  menus[0].menuItems[5].func = &showVersionInfo;
  
  messages[0][0] = messages[1][0] = messages[2][0] = messages[3][0] = messages[4][0] = messages[5][0] = '\0';
  
  // Sample messages
  sprintf(messages[0], "Here's sample message number one");
  sprintf(messages[1], "And here's another message to display");
  sprintf(messages[2], "Message number three is slightly different, but not much");
  sprintf(messages[3], "Four four message four here's four message four");
  numberOfMessages = 4;
  
  pinMode(BACKLIGHTPIN, OUTPUT);
  analogWrite(BACKLIGHTPIN, 255);
  
  // Initialise the button read pin to input mode and enable the pull-up resistor
  pinMode(A0, INPUT);
  digitalWrite(A0, HIGH);
  pinMode(A1, INPUT);
  digitalWrite(A1, HIGH);
  pinMode(A2, INPUT);
  digitalWrite(A2, HIGH);
  pinMode(A3, INPUT);
  digitalWrite(A3, HIGH);
  
  lcd.begin(20, 4);
  lcd.setCursor(0, 0);
  // Print a message to the LCD.
  lcd.print("Starting...");
  
  turnBacklightOn();
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  
  // this check is only needed on the Leonardo:
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    //Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
  
#if defined(DEV)
  printNetworkSettings();
  delay(4000);
#endif
  
  timeSinceLastLiveConnAttempt = millis() - 60000;
  
  showMessages();
}

/*
 * The standard Arduino loop function
 */
void loop() {
  
  // Read the saved blttimer value
  int backlightTimer = DEFAULTBACKLIGHTTIME;
  char backlightProperty[51];
  backlightProperty[0] = '\0';
  readProperty("blttimer", backlightProperty);
  if (strlen(backlightProperty) > 0) {
   // Convert the char array into an int
   backlightTimer = atoi(backlightProperty);
   if (backlightTimer > MAXBACKLIGHTTIME) {
     backlightTimer = MAXBACKLIGHTTIME;
   } else if (backlightTimer == 0) {
     backlightTimer = DEFAULTBACKLIGHTTIME;
   }
  }
  
  if ((millis() - timeSinceBacklightOn) > (1000 * backlightTimer)) {
    // No activity - turn backlight off and go back to the main screen
    if (backlightOn) turnBacklightOff();
    
    if (currentMenu) {
      // We were in a menu      
      showMessages();
    }
  }
  
  byte i = 0;
  for (i = 0; i < 4; i++) {
    
    buttonReadState = digitalRead(A0 + i);
  
    // If the button is pressed, print on screen
    if (buttonReadState == LOW && (buttonState & (1 << i))) {
      buttonState = buttonState ^ (1 << i);
      
      // Call the relevant function to act on the button
      (*buttonFunctions[i])();
    }
    else if (buttonReadState == HIGH && !(buttonState & (1 << i))) {
      buttonState = buttonState | (1 << i);
    }
  }
  
  delay(20);
  
  checkForSerialData();
  
  delay(5);
  
  checkForHTTPConnections();
  
  delay(10);
}



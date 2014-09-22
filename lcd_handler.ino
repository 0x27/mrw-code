/*
  DHCP-based IP printer
 
 This sketch uses the DHCP extensions to the Ethernet library
 to get an IP address via DHCP and print the address obtained.
 using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 
 created 12 April 2011
 modified 9 Apr 2012
 by Tom Igoe
 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>
#include <avr/pgmspace.h>
//#include <PubSubClient.h>

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// Strings for displaying on the LCD - stored in flash memory to save space in SRAM
prog_char menuItem01[] PROGMEM = "        Menu"; 
prog_char menuItem02[] PROGMEM = "- View network info";
prog_char menuItem03[] PROGMEM = "- Ping server";
prog_char menuItem04[] PROGMEM = "- Item Three";
prog_char menuItem05[] PROGMEM = "- Item Four";
prog_char menuItem06[] PROGMEM = "- Item Five";
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

// Strings for sending to a connected browser
prog_char httpResponseString01[] PROGMEM = "HTTP/1.1 200 OK"; 
prog_char httpResponseString02[] PROGMEM = "Content-Type: text/html";
prog_char httpResponseString03[] PROGMEM = "Connection: close";
prog_char httpResponseString04[] PROGMEM = "<!DOCTYPE HTML>";
prog_char httpResponseString05[] PROGMEM = "<html>";
prog_char httpResponseString06[] PROGMEM = "<title>Config Utility</title>";
prog_char httpResponseString07[] PROGMEM = "<h3>Config Utility</h3>";
prog_char httpResponseString08[] PROGMEM = "<p>Change display settings</p>";
prog_char httpResponseString09[] PROGMEM = "<form name=\"form1\">";
prog_char httpResponseString10[] PROGMEM = "<input type=\"text\" name=\"input1\"/>";
prog_char httpResponseString11[] PROGMEM = "</form>";
prog_char httpResponseString12[] PROGMEM = "</html>";
PROGMEM const char *httpResponseStrings[] =
{   
  httpResponseString01,
  httpResponseString02,
  httpResponseString03,
  httpResponseString04,
  httpResponseString05,
  httpResponseString06,
  httpResponseString07,
  httpResponseString08,
  httpResponseString09,
  httpResponseString10,
  httpResponseString11,
  httpResponseString12
};

byte numberOfMessages = 0;
byte messageCursor = 0;
char messages[6][60];
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
#define MAXTIMEBACKLIT 40
int brightness = 255;

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

void testItemThree() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Test of item 3");
}

void testItemFour() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Test of item 4");
}

void okButtonPressed() {
  commonButtonFunctions();
  
  // If we're in a menu, call the next function. Otherwise do nothing for the time being.
  if (currentMenu != 0) {
    insideMenuFunction = 1;
    (currentMenu->menuItems[currentMenu->selectionCursor + 1].func)();
  }
}

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
      serialRXCursor++;
    }
  }
}

void checkForHTTPConnections() {
  // See if a client has attached to port 80
  EthernetClient serverClient = server.available();
  
  if (serverClient) {
    char buffer[35];
    
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (serverClient.connected()) {
      if (serverClient.available()) {
        char c = serverClient.read();
        //Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          strcpy_P(buffer, (char*)pgm_read_word(&(httpResponseStrings[0])));  // Copy "HTTP/1.1 200 OK" out of flash strings
          serverClient.println(buffer);
          strcpy_P(buffer, (char*)pgm_read_word(&(httpResponseStrings[1])));  // Copy "HTTP/1.1 200 OK" out of flash strings
          serverClient.println(buffer);
          strcpy_P(buffer, (char*)pgm_read_word(&(httpResponseStrings[2])));  // Copy "HTTP/1.1 200 OK" out of flash strings
          serverClient.println(buffer);  // the connection will be closed after completion of the response
          serverClient.println();
          strcpy_P(buffer, (char*)pgm_read_word(&(httpResponseStrings[3])));  // Copy "HTTP/1.1 200 OK" out of flash strings
          serverClient.println(buffer);
          strcpy_P(buffer, (char*)pgm_read_word(&(httpResponseStrings[4])));  // Copy "HTTP/1.1 200 OK" out of flash strings
          serverClient.println(buffer);
          strcpy_P(buffer, (char*)pgm_read_word(&(httpResponseStrings[5])));  // Copy "HTTP/1.1 200 OK" out of flash strings
          serverClient.println(buffer);
          strcpy_P(buffer, (char*)pgm_read_word(&(httpResponseStrings[6])));  // Copy "HTTP/1.1 200 OK" out of flash strings
          serverClient.println(buffer);
          strcpy_P(buffer, (char*)pgm_read_word(&(httpResponseStrings[7])));  // Copy "HTTP/1.1 200 OK" out of flash strings
          serverClient.println(buffer);
          strcpy_P(buffer, (char*)pgm_read_word(&(httpResponseStrings[8])));  // Copy "HTTP/1.1 200 OK" out of flash strings
          serverClient.println(buffer);
          strcpy_P(buffer, (char*)pgm_read_word(&(httpResponseStrings[9])));  // Copy "HTTP/1.1 200 OK" out of flash strings
          serverClient.println(buffer);
          strcpy_P(buffer, (char*)pgm_read_word(&(httpResponseStrings[10])));  // Copy "HTTP/1.1 200 OK" out of flash strings
          serverClient.println(buffer);
          // output the value of each analog input pin
          serverClient.print("a 10</br>");
          serverClient.print(timeSinceLastLiveConnAttempt);
          strcpy_P(buffer, (char*)pgm_read_word(&(httpResponseStrings[11])));  // Copy "HTTP/1.1 200 OK" out of flash strings
          serverClient.println(buffer);
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    
    // give the web browser time to receive the data
    delay(5);
    // close the connection:
    serverClient.stop();
  }
}

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
  menus[0].menuItems[3].func = &testItemThree;
  menus[0].menuItems[4].func = &testItemFour;
  
  messages[0][0] = messages[1][0] = messages[2][0] = messages[3][0] = messages[4][0] = messages[5][0] = messages[6][0] = messages[7][0] = '\0';
  
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
  
  timeSinceLastLiveConnAttempt = millis() - 60000;
  
  showMessages();
}

void loop() {
  if ((millis() - timeSinceBacklightOn) > (MAXTIMEBACKLIT * 1000)) {
    // No activity - turn backlight off
    if (backlightOn) turnBacklightOff();
    
    if (currentMenu) {
      // We were in a menu
      currentMenu = 0;
      showMessages();
    }
  }
  
  byte i = 0;
  for (i = 0; i < 4; i++) {
    
    buttonReadState = digitalRead(A0 + i);
    //Serial.println(buttonReadState);
  
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



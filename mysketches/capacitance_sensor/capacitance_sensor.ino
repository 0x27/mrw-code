#include <CapacitiveSensor.h>

/*
 * CapitiveSense Library Demo Sketch
 * Paul Badger 2008
 * Uses a high value resistor e.g. 10 megohm between send pin and receive pin
 * Resistor effects sensitivity, experiment with values, 50 kilohm - 50 megohm. Larger resistor values yield larger sensor values.
 * Receive pin is the sensor pin - try different amounts of foil/metal on this pin
 * Best results are obtained if sensor foil and wire is covered with an insulator such as paper or plastic sheet
 */

#define SIZE_OF_AVERAGE_WINDOW 30

//CapacitiveSensor   cs_4_2 = CapacitiveSensor(4,2);        // 10 megohm resistor between pins 4 & 2, pin 2 is sensor pin, add wire, foil
//CapacitiveSensor   cs_4_5 = CapacitiveSensor(4,5);        // 10 megohm resistor between pins 4 & 6, pin 6 is sensor pin, add wire, foil
CapacitiveSensor   cs_4_8 = CapacitiveSensor(4,8);        // 10 megohm resistor between pins 4 & 8, pin 8 is sensor pin, add wire, foil

int i = 0;
int dataSet[SIZE_OF_AVERAGE_WINDOW];

void setup()                    
{

   //cs_4_8.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example
   Serial.begin(57600);

}

void loop()                    
{
    long start = millis();
    //long total1 =  cs_4_2.capacitiveSensor(30);
    //long total2 =  cs_4_5.capacitiveSensor(30);
    long total3 =  cs_4_8.capacitiveSensor(30);
    
    dataSet[i] = (int)total3;
    i++;
    
    if (i == 30) {
      i = 0;
      
      int j = 0;
      long average = 0;
      for (j = 0; j < SIZE_OF_AVERAGE_WINDOW; j++) {
        average = average + dataSet[j];
      }
      average = average / SIZE_OF_AVERAGE_WINDOW;
      Serial.println(average);
      delay(1000);
    } else {
      delay(10);
    }
    
    //Serial.print(millis() - start);        // check on performance in milliseconds
    //Serial.print("\t");                    // tab character for debug window spacing

    //Serial.print(total1);                  // print sensor output 1
    //Serial.print("\t");
    //Serial.print(total2);                  // print sensor output 2
    //Serial.print("\t");
    //Serial.println(total3);                // print sensor output 3

    //delay(100);                             // arbitrary delay to limit data to serial port 
}

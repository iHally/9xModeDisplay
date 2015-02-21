/*

      MODE DISPLAY v1.07
 
     Hally's Quad Assist 



This code falls under the WTFPL licence http://www.wtfpl.net/

         DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE 
                    Version 2, December 2004 

 Copyright (C) 2004 Sam Hocevar <sam@hocevar.net> 

 Everyone is permitted to copy and distribute verbatim or modified 
 copies of this license document, and changing it is allowed as long 
 as the name is changed. 

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE 
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION 

  0. You just DO WHAT THE FUCK YOU WANT TO.
  
*/

#include <Wire.h>
//pressure sensor
#include <BMP085.h>

BMP085 dps = BMP085();
long Temperature = 0, Pressure = 0, Altitude = 0;

//oled screen
#include <GOFi2cOLED.h>
GOFi2cOLED GOFoled;

//global variables
int ModePin = A0; // select the input pin for the Mode Potentiometer 150k 3k
int BatPin = A1; // input pin for the main power voltage divider 150k 33k

int pos = 0; //front position of graph
int gmax = 50;  //graph max (altitude change in cm)
int gmin = -50; //graph min (altitude change in cm)
void setup()
{
  //Pressure sensor
  Wire.begin();
  dps.init();
  
  //Splash Screen
  // default slave address is 0x3D - Some screens like the one linked in the readme is 0x3C
  GOFoled.init(0x3C);  //initialze  OLED display
  //GOFoled.setRotation(2); //upsidedown in case your sceen is mounted the wrong way ;-)
  GOFoled.display(); // show splashscreen - will show in only 1 directrion. change your splash with LCDAssist and ammend CPP file in GOFoled library
  delay(2000);
  //GOFoled.clearDisplay();
  
  GOFoled.setTextSize(1);
  GOFoled.setTextColor(WHITE);
  GOFoled.setCursor(0,55);
  GOFoled.print("V "); 
  GOFoled.print(1.07);
  
  GOFoled.display();
  delay(1000);
  
  GOFoled.clearDisplay();
  GOFoled.display();
}


//Modes in Prefered Order. Change to suit your needs
const unsigned int number_of_modes = 6;
char *modes[number_of_modes] = {"Stabilise", "Alt Hold", " Loiter", "   Auto", "   RTL", "S Simple"};

   static unsigned int mode_index = 10;
   static unsigned int mode_index_previous = 10;
   
   const int numReadings = 15.0;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
long total = 0;                  // the running total
long average = 0;                // the average


void CheckMode(){
  
   int val = analogRead(ModePin);
   //map input voltage. min = 0V max= 1.96V
   //find max pin value 0-1024, 0-3.3V
   //    1.96/3.3 = 0.596
   //    0.596*1024 = 610
   
   // map input to 0-5V
   int temp = map(val, 0, 610, 0, 500);

   //from excel sheet find voltage values to match ardupilot radio ppm 
   if (temp <= 80) {
     mode_index = 0;
   } else if (temp <= 140) {
     mode_index = 1;
   } else if (temp <= 230) {
     mode_index = 2;
   } else if (temp <= 300) {
     mode_index = 3;
   } else if (temp <= 410) {
     mode_index = 4;
   } else {
     mode_index = 5;
   }
 
 
 //if new mode detected update display
 if (mode_index != mode_index_previous) {
      mode_index_previous = mode_index;
      GOFoled.clearArea(10,12,116,17); //fill rectangle x,y,w,h
      GOFoled.setCursor(10,12);
      GOFoled.setTextSize(2);
      GOFoled.print(String(modes[mode_index]));
      GOFoled.setTextSize(1);
  }
}

void Graph(void){
  
  pos++;//increment graph position
  
  if(pos>=128){ //screen overflow management
    pos=0; 
  }
     
     //display altitude
      GOFoled.clearArea(2,32,62,7); //fill rectangle x,y,w,h
      GOFoled.setCursor(2,32);
      GOFoled.print("Alt:");
      GOFoled.print(average/100.0);
    if((average > gmax)||(average < gmin)){ //graph overflow management
      gmax=average;
      gmin=-gmax;
    }
    int number = map(average, gmin, gmax, 45, 64);
    GOFoled.drawPixel( pos, number, WHITE);
    //clear front of graph
    GOFoled.clearArea(pos+1,45,14,19); //fill rectangle x,y,w,h
}



void clearline(int indent, int line, int letters)
{
  //clear line
  GOFoled.clearArea(indent,line*7,letters*5,7); //fill rectangle x,y,w,h
}

void DisplayBat(void)
{
  //battery display 150k 33k
  clearline(47,0,18);
  GOFoled.setCursor(47,0);
  
  // map low voltage and high voltage of battery through voltage divider
  // bat range 9.3V-12.6V to pin range 1.579V-2.197V
  // 2.198/3.3 = 0.666 
  // 0.666*1024 = 682 - max
  //
  // 1.579/3.3 = 0.478
  // 0.478*1024 = 490 - min
  
  //display voltage
  GOFoled.print(map(analogRead(BatPin),490,682,93,126)/10.0);
  GOFoled.print("v");
  GOFoled.setCursor(93,0);
  GOFoled.print(map(analogRead(BatPin),481,652,0,100));
  GOFoled.print("%");
}

void loop()
{
  //mode display
  CheckMode();
  
  //temp display
  dps.getTemperature(&Temperature);
  //clearline(65,32,11);
  GOFoled.clearArea(65,32,63,7); //fill rectangle x,y,w,h
  GOFoled.setCursor(65,32);
  GOFoled.print("Temp:");
  GOFoled.print(Temperature/10.0);
  
  //graph altitude
  dps.getAltitude(&Altitude);
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = Altitude;
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;
  // if we're at the end of the array...
  if (readIndex >= numReadings){
    // ...wrap around to the beginning:
    readIndex = 0;
    // calculate the average:
    average = total / numReadings;
    // send it to the computer as ASCII digits
    Graph();
    DisplayBat();
  }
  
  GOFoled.display();
  //delay(10);
}

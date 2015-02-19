
#include <Wire.h>
#include <BMP085.h>

BMP085 dps = BMP085();
long Temperature = 0, Pressure = 0, Altitude = 0;

#include <GOFi2cOLED.h>
GOFi2cOLED GOFoled;
int x=0;
int y=40;

int ModePin = A0;    // select the input pin for the Mode Potentiometer
int BatPin = A1; // 15k 3k

int pos = 0;

void setup()
{
  Wire.begin();
  dps.init();
  
  // default slave address is 0x3D
  GOFoled.init(0x3C);  //initialze  OLED display
  //GOFoled.setRotation(2); //upsidedown
  GOFoled.display(); // show splashscreen
  delay(2000);
  //GOFoled.clearDisplay();
  
  GOFoled.setTextSize(1);
  GOFoled.setTextColor(WHITE);
  GOFoled.setCursor(0,55);
  
  GOFoled.print("V ");
  //GOFoled.println(-1234); 
  GOFoled.print(1.06);  
  /*
  GOFoled.setTextColor(BLACK, WHITE); // 'inverted' text
  GOFoled.println(3.14159,5);
  GOFoled.setTextSize(2);
  GOFoled.setTextColor(WHITE);
  GOFoled.print("0x"); GOFoled.println(0xDEADBEEF, HEX);
  */
  GOFoled.display();
  delay(2000);
  GOFoled.clearDisplay();
  GOFoled.display();
}

const unsigned int number_of_modes = 6;
char *modes[number_of_modes] = {"Stabilise", "Alt Hold", " Loiter", "   Auto", "   RTL", "S Simple"};

   static unsigned int mode_index = 10;
   static unsigned int mode_index_previous = 10;
   
   const int numReadings = 10.0;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
long total = 0;                  // the running total
long average = 0;                // the average

void CheckMode(){
  
  int val = analogRead(ModePin);
   //val = map(val, 0, 1023, 0, 4); input value range shifted to allow for battery type
   //int val = 950;
   int temp = map(val, 0, 610, 0, 500);

 
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
  
 if (mode_index != mode_index_previous) {
      mode_index_previous = mode_index;
      //active = 1;
      //LcdClearLn(0);
      GOFoled.clearArea(10,12,116,17); //fill rectangle x,y,w,h
      GOFoled.setCursor(10,12);
      GOFoled.setTextSize(2);
      GOFoled.print(String(modes[mode_index]));
      GOFoled.setTextSize(1);
  }
}

void Graph(void){
  
  pos++;
  if(pos>=128){
    pos=0;
  }
     
     //display altitude
      GOFoled.clearArea(2,32,62,7); //fill rectangle x,y,w,h
      GOFoled.setCursor(2,32);
      GOFoled.print("Alt:");
      GOFoled.print(average/100.0);
    
    int number = map(average, -750, 750, 45, 64);
    GOFoled.drawPixel( pos, number, WHITE);
    GOFoled.clearArea(pos+1,45,14,19); //fill rectangle x,y,w,h
}



void clearline(int indent, int line, int letters)
{
  //clear line
  GOFoled.clearArea(indent,line*7,letters*5,7); //fill rectangle x,y,w,h

}

void loop()
{
  
  //delay(3000);
  //GOFoled.clearDisplay();
  //GOFoled.setCursor(0,0);
  //GOFoled.print("0x"); GOFoled.println(0xDEADBEEF);
  //GOFoled.clearArea(80,0,48,7); //fill rectangle x,y,w,h
  
  //mode display
  CheckMode();
  
  //battery display 15k 3k
  clearline(60,0,15);
  GOFoled.setCursor(60,0);
  GOFoled.print(map(analogRead(BatPin),481,652,9.2,12.6));
  GOFoled.print("v");
  GOFoled.setCursor(90,0);
  GOFoled.print(map(analogRead(BatPin),481,652,0,100));
  GOFoled.print("%");
  
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
  }
  
  
  GOFoled.display();
  //delay(10);
 
}

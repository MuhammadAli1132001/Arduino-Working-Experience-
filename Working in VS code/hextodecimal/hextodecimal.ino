#include<LiquidCrystal.h>
#include <Arduino.h>
int rs = 13, en = 12, d4 = 5, d5 = 10, d6 = 3, d7 = 2;
LiquidCrystal display(rs,en,d4,d5,d6,d7);

long decvalue = 0;
int input ;
char hexvalue[10];

void setup() {
  
  Serial.begin(115200);
  display.begin(16,2);
}

void loop() {

  Serial.println("\n\n\nenter hex value");
  delay(10);
  while(Serial.available() <= 0);

  
  input = Serial.readBytesUntil('\n', hexvalue, sizeof(hexvalue)-1);
  hexvalue[input] = '\0';
  Serial.print("\nEnter Hex value is: ");
  Serial.print(hexvalue);
  hextodecimal(hexvalue);
  
  lcdupdate();
  delay(100);
}


void lcdupdate(){
  
  display.clear();
  display.setCursor(0,0);
  display.print("enterhexvalue:");
  display.print(hexvalue);
  display.setCursor(0,1);
  display.print("dec value: ");
  display.print(decvalue);
  
}
void hextodecimal(char *stringvalue){

  decvalue = strtol(stringvalue, NULL, 16);
  Serial.print("\nConverted string by buildin function is: ");
  Serial.print(decvalue);  
 
}

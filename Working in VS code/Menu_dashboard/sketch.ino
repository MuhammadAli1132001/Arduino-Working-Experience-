#include <LiquidCrystal.h>


// LCD setup
const int Rs = 13, En = 11, D4 = 6, D5 = 7, D6 = 8, D7 = 9;
LiquidCrystal display(Rs, En, D4, D5, D6, D7);



void setup() {
  Serial.begin(115200);
  display.begin(16, 2);
}

void updateDisplay() {
  display.clear();
  display.setCursor(0, 0);
  display.print("btn1 ");
  display.print("btn2 ");
  display.print("btn3");
}

void loop() {
  unsigned long currentMillis = millis();            //time since powered
  updateDisplay();
  Serial.println("updated..");
    display.setCursor(0, 1);

    display.print("slection");

  delay(1000);
}
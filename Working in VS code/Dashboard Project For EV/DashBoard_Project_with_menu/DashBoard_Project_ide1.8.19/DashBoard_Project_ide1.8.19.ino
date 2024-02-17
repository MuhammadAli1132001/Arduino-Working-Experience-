#include <Adafruit_TFTLCD.h>
#include <Adafruit_GFX.h>
#include <TouchScreen.h>
#include <MCUFRIEND_kbv.h>

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

#define TS_MINX 122
#define TS_MINY 111
#define TS_MAXX 500
#define TS_MAXY 680

#define YP A1
#define XM A2
#define YM 7
#define XP 6

/*
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin
*/

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

uint8_t DistanceRange = 60;
uint8_t BatteryThresold = 15;

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 334);

boolean buttonEnabled = true;
boolean buttonEnabledMenu = true;
boolean buttonEnabledDistance = false;
boolean buttonEnabledThreshold = false;
boolean buttonUp = true;
boolean buttonDown = true;
boolean buttonSelect = false;

uint8_t menu = 0;
uint8_t state = 0;

float battery_capacity = 0;
float distance = 0;
float current = 0;
float voltage = 0;
float power = 0;
float powerkilo = 0;

float prebattery = 0;
float predistance = 0;
float currbattery = 50;
float currdistance = 0;

float i = 0;
float j = 0;

void setup()
{

  Serial.begin(115200);

  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  tft.drawRect(0, 0, 319, 240, YELLOW);

  delay(2000);
  tft.fillScreen(WHITE);
  tft.setCursor(30, 40);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("Dashboard For EV");

  tft.setCursor(115, 80);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("Project\n\n            by");

  tft.setCursor(55, 155);
  tft.setTextColor(BLUE);
  tft.setTextSize(2);
  tft.print("Chip Soul Tech");

  tft.fillRect(50, 180, 210, 40, YELLOW);
  tft.drawRect(50, 180, 210, 40, RED);
  tft.setCursor(60, 190);
  tft.setTextColor(BLUE);
  tft.setTextSize(2);
  tft.print("Click For detail");
}

void loop()
{
    // TSPoint is object encslt x,y and pressure measurment and getPoint measure, TO Make clickable button on these pixels area
    TSPoint p = ts.getPoint();

    if (p.z > ts.pressureThreshhold)
    {

      p.x = map(p.x, TS_MAXX, TS_MINX, 0, 320);
      p.y = map(p.y, TS_MAXY, TS_MINY, 0, 480);

      // if (menu == 0){

      if (p.x > -180 && p.x < 250 && p.y > 300 && p.y < 430 && buttonEnabled && state == 0)
      { // checking button clikable range

        buttonEnabledMenu = true;
        menu = 0;
        buttonEnabled = false;

        Serial.println("\nDashboard main ");
        Serial.print("press position  ");

        Serial.print(p.x);
        Serial.print(",");
        Serial.print(p.y);
        delay(30);

        pinMode(XM, OUTPUT);
        pinMode(YP, OUTPUT);
        tft.fillScreen(BLACK);
        tft.setTextColor(RED, BLACK);
        tft.setCursor(55, 180);
        tft.print("100");
        tft.setCursor(50, 120);
        tft.print("Battery");
        tft.setTextColor(BLUE, BLACK);
        tft.setCursor(212, 180);
        tft.print(DistanceRange);
        tft.setCursor(200, 120);
        tft.print("Distance");
        tft.setCursor(30, 5);

        tft.fillRect(0, 0, 320, 45, BLACK);
        tft.setTextColor(YELLOW, BLACK);
        tft.print("EV Dashboard Monitor");
        tft.setTextColor(WHITE, BLACK);

        tft.fillRect(100, 200, 90, 30, YELLOW);
        tft.drawRect(100, 200, 90, 30, RED);
        tft.setCursor(120, 210);
        tft.setTextColor(BLUE);
        tft.setTextSize(2);
        tft.print("Menu");
        tft.setTextColor(WHITE, BLACK);

        tft.fillRect(55, 150, 60, 20, BLACK);
        tft.fillRect(212, 150, 60, 20, BLACK);

        // while (menu == 0)
        // {

        tft.setCursor(160, 45);
        tft.print(menu, 1);
        // TSPoint p = ts.getPoint();
        // if (p.z > ts.pressureThreshhold) {
        // p.x = map(p.x, TS_MAXX, TS_MINX, 0, 320);
        // p.y = map(p.y, TS_MAXY, TS_MINY, 0, 480);
        prebattery = currbattery;
        predistance = currdistance;

        battery_capacity = random(0, 100); // Assigning Random value for demonstration
        distance = (battery_capacity * DistanceRange / 100);
        voltage = random(0, 66);
        current = random(0, 99);
        power = voltage * current;

        Serial.println("\nBattery Remaining: " + String(battery_capacity) + "%"); // printing measurments on serial monitor
        Serial.println("Distance Remaining: " + String(distance) + "KM");

        Serial.println("Voltage: " + String(voltage) + "V");
        Serial.println("Current: " + String(current) + "A");

        if (power >= 1000)
        { // Power in kilowatt
          powerkilo = power / 1000;
          Serial.println("Power: " + String(powerkilo) + "KW");
        }

        else
        { // Power in watt
          Serial.println("Power: " + String(power) + "W");
        }

        currbattery = battery_capacity;
        currdistance = distance;

        tft.setCursor(0, 30);
        tft.print("Voltage:");
        tft.print(voltage, 1);
        tft.print("V");
        tft.setCursor(160, 30);
        tft.print("Current:");
        tft.print(current, 1);
        tft.print("A");
        tft.setCursor(55, 150);
        tft.print(battery_capacity, 1);
        tft.setCursor(212, 150);
        tft.print(distance, 1);

        i = j = 0;
        i = map(prebattery, 0, 100, 0, 300);
        j = map(currbattery, 0, 100, 0, 300);

        for (i; i <= j; i = i + 0.1)
        {

          float j = i - 150;
          float angle = (j / 57.2958) - 1.57;
          float x1 = 80 + cos(angle) * 55;
          float y1 = 130 + sin(angle) * 55;
          float x2 = 80 + cos(angle) * 75;
          float y2 = 130 + sin(angle) * 75;
          tft.drawLine(x1, y1, x2, y2, RED); // making battery graphics
          x1 = x2 = y1 = y2 = 0;
        }
        i = map(prebattery, 0, 100, 0, 300);
        j = map(currbattery, 0, 100, 0, 300);

        for (i - 2; i >= j; i = i - 0.05)
        {

          float j = i - 150;
          float angle = (j / 57.2958) - 1.57;
          float x1 = 80 + cos(angle) * 55;
          float y1 = 130 + sin(angle) * 55;
          float x2 = 80 + cos(angle) * 75;
          float y2 = 130 + sin(angle) * 75;
          tft.drawLine(x1, y1, x2, y2, BLACK); // removing battery graphics
          // tft.drawLine(x1+160, y1,x2+160,y2, MAGENTA);
          x1 = 0;
          x2 = 0;
          y1 = 0;
          y2 = 0;
        }

        i = map(predistance, 0, 100, 0, 300);
        j = map(currdistance, 0, 100, 0, 300);

        for (i; i <= j; i = i + 0.1)
        {
          float j = i - 150;
          float angle = (j / 57.2958) - 1.57;
          float x1 = 80 + cos(angle) * 55;
          float y1 = 130 + sin(angle) * 55;
          float x2 = 80 + cos(angle) * 75;
          float y2 = 130 + sin(angle) * 75;

          tft.drawLine(x1 + 160, y1, x2 + 160, y2, BLUE); // making Distance graphics
          x1 = x2 = y1 = y2 = 0;
        }
        i = map(predistance, 0, 100, 0, 300);
        j = map(currdistance, 0, 100, 0, 300);

        for (i - 2; i >= j; i = i - 0.05)
        {
          float j = i - 150;
          float angle = (j / 57.2958) - 1.57;
          float x1 = 80 + cos(angle) * 55;
          float y1 = 130 + sin(angle) * 55;
          float x2 = 80 + cos(angle) * 75;
          float y2 = 130 + sin(angle) * 75;

          tft.drawLine(x1 + 160, y1, x2 + 160, y2, BLACK); // Removing Distance graphics
          x1 = x2 = y1 = y2 = 0;
        }

        p.x = 200;
        p.y = 300;

        delay(500); // Refreshing Delay
      }

      if (p.x > -100 && p.x < 150 && p.y > 350 && p.y < 450 && buttonEnabledMenu && !buttonEnabled)
      { // button clikable range
        Serial.println("\nentered into menu");
        Serial.print("Press touch region  ");
        Serial.print(p.x);
        Serial.print(",");
        Serial.print(p.y);
        delay(30);

        menu = 1;
        buttonEnabledMenu = false;
        buttonEnabledDistance = true;
        buttonEnabledThreshold = true;
        pinMode(XM, OUTPUT);
        pinMode(YP, OUTPUT);

        tft.fillScreen(BLACK);
        tft.drawRect(0, 0, 319, 240, YELLOW);
        tft.fillRect(20, 60, 270, 60, BLUE);
        tft.drawRect(20, 60, 270, 60, RED);
        tft.setCursor(30, 80);
        tft.setTextColor(WHITE);
        tft.setTextSize(2);
        tft.print("Set Distance in KM ");
        tft.print(DistanceRange);

        tft.fillRect(20, 130, 270, 60, BLUE);
        tft.drawRect(20, 130, 270, 60, RED);
        tft.setCursor(30, 150);
        tft.setTextColor(WHITE);
        tft.setTextSize(2);
        tft.print("Set Battery Threshold");
      }

      if (p.x > -280 && p.x < 380 && p.y > 20 && p.y < 150 && buttonEnabledDistance && !buttonEnabledMenu)
      {

        buttonEnabledDistance = false;
        buttonSelect = true;
        Serial.println("\nSet Distance ");
        Serial.print("Press Touch Region ");
        Serial.print(p.x);
        Serial.print(",");
        Serial.print(p.y);
        delay(30);

        pinMode(XM, OUTPUT);
        pinMode(YP, OUTPUT);

        tft.fillScreen(BLACK);
        tft.fillRect(40, 80, 80, 60, GREEN);
        tft.setCursor(60, 100);
        tft.setTextColor(WHITE);
        tft.setTextSize(2);
        tft.print("UP");

        tft.fillRect(140, 80, 60, 60, BLUE);
        tft.setTextColor(WHITE);
        tft.setCursor(145, 100);
        tft.print(DistanceRange);
        tft.print("KM");

        tft.fillRect(220, 80, 80, 60, YELLOW);
        tft.setCursor(230, 100);
        tft.setTextColor(WHITE);
        tft.setTextSize(2);
        tft.print("Down");

        tft.fillRect(120, 160, 100, 60, RED);
        tft.setCursor(130, 180);
        tft.setTextColor(WHITE);
        tft.setTextSize(2);
        tft.print("Select");
      }

    if (p.x > -280 && p.x < 380 && p.y > 170 && p.y < 300 && buttonEnabledThreshold && !buttonEnabledMenu)
      {

        buttonEnabledThreshold = false;
        buttonSelect = true;
        Serial.println("\nSet Battery Threshold");
        Serial.print("Press Touch Region ");
        Serial.print(p.x);
        Serial.print(",");
        Serial.print(p.y);
        delay(30);

        pinMode(XM, OUTPUT);
        pinMode(YP, OUTPUT);

        tft.fillScreen(BLACK);
        tft.fillRect(40, 80, 80, 60, GREEN);
        tft.setCursor(60, 100);
        tft.setTextColor(WHITE);
        tft.setTextSize(2);
        tft.print("UP");
        
        tft.fillRect(140, 80, 60, 60, BLUE);
        tft.setTextColor(WHITE);
        tft.setCursor(145, 100);
        tft.print(BatteryThresold);
        tft.print("T");

        tft.fillRect(220, 80, 80, 60, YELLOW);
        tft.setCursor(230, 100);
        tft.setTextColor(WHITE);
        tft.setTextSize(2);
        tft.print("Down");

        tft.fillRect(120, 160, 100, 60, RED);
        tft.setCursor(130, 180);
        tft.setTextColor(WHITE);
        tft.setTextSize(2);
        tft.print("Select");
      }

    if (p.x > -240 && p.x < -80 && p.y > 40 && p.y < 180 && buttonUp)
      {

        if (!buttonEnabledDistance){

        Serial.println("\nDistance Range updated +");
        DistanceRange = DistanceRange + 1;

        pinMode(XM, OUTPUT);
        pinMode(YP, OUTPUT);

        tft.fillRect(140, 80, 60, 60, BLUE);
        tft.setTextColor(WHITE);
        tft.setCursor(145, 100);
        tft.print(DistanceRange);
        tft.print("KM  ");
        }

        if (!buttonEnabledThreshold){

        Serial.println("\nBattery Threshold updated +");
        BatteryThresold = BatteryThresold + 1;

        pinMode(XM, OUTPUT);
        pinMode(YP, OUTPUT);

        tft.fillRect(140, 80, 60, 60, BLUE);
        tft.setTextColor(WHITE);
        tft.setCursor(145, 100);
        tft.print(BatteryThresold);
        tft.print("T  ");
        }
      }

    if (p.x > 180 && p.x < 300 && p.y > 40 && p.y < 200 && buttonDown)
      {
        
        if (!buttonEnabledDistance){
        Serial.println("\nDistance Range updated -");

        DistanceRange = DistanceRange - 1;

        pinMode(XM, OUTPUT);
        pinMode(YP, OUTPUT);

        tft.fillRect(140, 80, 60, 60, BLUE);
        tft.setTextColor(WHITE);
        tft.setCursor(145, 100);
        tft.print(DistanceRange);
        tft.print("KM ");
        }

        if(!buttonEnabledThreshold){

        Serial.println("\nBattery Threshold updated -");
        BatteryThresold = BatteryThresold - 1;

        pinMode(XM, OUTPUT);
        pinMode(YP, OUTPUT);
        
        tft.fillRect(140, 80, 60, 60, BLUE);
        tft.setTextColor(WHITE);
        tft.setCursor(145, 100);
        tft.print(BatteryThresold);
        tft.print("T ");
        }
      }
      

      if (p.x > -220 && p.x < 200 && p.y > 300 && p.y < 400 && buttonSelect && !buttonEnabledMenu && !buttonEnabled)
      {

        buttonEnabled = true;
        menu = 0;
        buttonSelect = false;

        p.x = 200;
        p.y = 300;

        Serial.println("\nMenu selected ");
        pinMode(XM, OUTPUT);
        pinMode(YP, OUTPUT);
      }
    }
}
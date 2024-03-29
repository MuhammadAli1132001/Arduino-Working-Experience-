
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

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 334);

boolean buttonEnabled = true;

void setup() {

  Serial.begin(115200);
  
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  tft.drawRect(0, 0, 319, 240, YELLOW);

  
  for (int i = 100; i > 5; i -= 5) {
    tft.drawCircle(160, 120, i, GREEN);
    delay(30);
  }
  
  for(int i= 10; i<50; i+=5){
    tft.drawTriangle(200,360+i, 150+i, 460-i, 250-i, 460-i, BLUE);
  }
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

void loop() {
  TSPoint p = ts.getPoint();  //TO Make clickable button on these pixels area

  if (p.z > ts.pressureThreshhold) {
  
    p.x = map(p.x, TS_MAXX, TS_MINX, 0, 320);
    p.y = map(p.y, TS_MAXY, TS_MINY, 0, 480);

    if (p.x > -180 && p.x < 250 && p.y > 300 && p.y < 430 && buttonEnabled) {    //button clikable range
      Serial.println("Touch button press");
 
      Serial.println(p.x);
      Serial.println(p.y);
      delay(30);
      buttonEnabled = false;

      pinMode(XM, OUTPUT);
      pinMode(YP, OUTPUT);

 
  
  tft.fillScreen(WHITE);
  tft.drawRect(0, 0, 319, 240, YELLOW);
//     tft.setCursor(50, 70);
 //    tft.setTextColor(BLACK);
  //   tft.setTextSize(3);

  tft.fillRect(30, 50, 240, 40, YELLOW);
  tft.drawRect(30, 50, 240, 40, RED);
  tft.setCursor(55, 60);
  tft.setTextColor(BLUE);
  tft.setTextSize(2);
  tft.print("Battery Voltage:");
  
  tft.fillRect(30, 100, 240, 40, YELLOW);
  tft.drawRect(30, 100, 240, 40, RED);
  tft.setCursor(85, 110);
  tft.setTextColor(BLUE);
  tft.setTextSize(2);
  tft.print("Current:");
  
  tft.fillRect(30, 150, 280, 40, YELLOW);
  tft.drawRect(30, 150, 280, 40, RED);
  tft.setCursor(55, 160);
  tft.setTextColor(BLUE);
  tft.setTextSize(2);
  tft.print("Remaining Capacity:");
    }
  }
}
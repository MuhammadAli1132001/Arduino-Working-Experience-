# 1 "C:\\Users\\Muhammad Ali\\Documents\\Arduino\\Dashboard Project For EV\\FirstSlide_DashBoard_Project\\DashBoard_Project_ide1.8.19\\DashBoard_Project_ide1.8.19.ino"

# 3 "C:\\Users\\Muhammad Ali\\Documents\\Arduino\\Dashboard Project For EV\\FirstSlide_DashBoard_Project\\DashBoard_Project_ide1.8.19\\DashBoard_Project_ide1.8.19.ino" 2

# 5 "C:\\Users\\Muhammad Ali\\Documents\\Arduino\\Dashboard Project For EV\\FirstSlide_DashBoard_Project\\DashBoard_Project_ide1.8.19\\DashBoard_Project_ide1.8.19.ino" 2
# 6 "C:\\Users\\Muhammad Ali\\Documents\\Arduino\\Dashboard Project For EV\\FirstSlide_DashBoard_Project\\DashBoard_Project_ide1.8.19\\DashBoard_Project_ide1.8.19.ino" 2
# 23 "C:\\Users\\Muhammad Ali\\Documents\\Arduino\\Dashboard Project For EV\\FirstSlide_DashBoard_Project\\DashBoard_Project_ide1.8.19\\DashBoard_Project_ide1.8.19.ino"
/*

#define YP A2  // must be an analog pin, use "An" notation!

#define XM A3  // must be an analog pin, use "An" notation!

#define YM 8   // can be a digital pin

#define XP 9   // can be a digital pin

*/





Adafruit_TFTLCD tft(A3, A2, A1, A0, A4);

TouchScreen ts = TouchScreen(6, A1, A2, 7, 334);

boolean buttonEnabled = true;

void setup() {

  Serial.begin(115200);

  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  tft.setRotation(1);
  tft.fillScreen(0x0000);
  tft.drawRect(0, 0, 319, 240, 0xFFE0);


  for (int i = 100; i > 5; i -= 5) {
    tft.drawCircle(160, 120, i, 0x07E0);
    delay(30);
  }

  for(int i= 10; i<50; i+=5){
    tft.drawTriangle(200,360+i, 150+i, 460-i, 250-i, 460-i, 0x001F);
  }
  delay(2000);
  tft.fillScreen(0xFFFF);

  tft.setCursor(30, 40);
  tft.setTextColor(0x0000);
  tft.setTextSize(2);
  tft.print("Dashboard For EV");

  tft.setCursor(115, 80);
  tft.setTextColor(0x0000);
  tft.setTextSize(2);
  tft.print("Project\n\n            by");

  tft.setCursor(55, 155);
  tft.setTextColor(0x001F);
  tft.setTextSize(2);
  tft.print("Chip Soul Tech");

  tft.fillRect(50, 180, 210, 40, 0xFFE0);
  tft.drawRect(50, 180, 210, 40, 0xF800);
  tft.setCursor(60, 190);
  tft.setTextColor(0x001F);
  tft.setTextSize(2);
  tft.print("Click For detail");
}

void loop() {
  TSPoint p = ts.getPoint(); //TO Make clickable button on these pixels area

  if (p.z > ts.pressureThreshhold) {

    p.x = map(p.x, 500, 122, 0, 320);
    p.y = map(p.y, 680, 111, 0, 480);

    if (p.x > -180 && p.x < 250 && p.y > 300 && p.y < 430 && buttonEnabled) { //button clikable range
      Serial.println("Touch button press");

      Serial.println(p.x);
      Serial.println(p.y);
      delay(30);
      buttonEnabled = false;

      pinMode(A2, 0x1);
      pinMode(A1, 0x1);



      tft.fillScreen(0xFFFF);
      tft.drawRect(0, 0, 319, 240, 0xFFE0);
//     tft.setCursor(50, 70);
 //    tft.setTextColor(BLACK);
  //   tft.setTextSize(3);

  tft.fillRect(30, 50, 240, 40, 0xFFE0);
  tft.drawRect(30, 50, 240, 40, 0xF800);
  tft.setCursor(55, 60);
  tft.setTextColor(0x001F);
  tft.setTextSize(2);
  tft.print("Battery Voltage:");

  tft.fillRect(30, 100, 240, 40, 0xFFE0);
  tft.drawRect(30, 100, 240, 40, 0xF800);
  tft.setCursor(85, 110);
  tft.setTextColor(0x001F);
  tft.setTextSize(2.9);
  tft.print("Current:");

  tft.fillRect(30, 150, 280, 40, 0xFFE0);
  tft.drawRect(30, 150, 280, 40, 0xF800);
  tft.setCursor(55, 160);
  tft.setTextColor(0x001F);
  tft.setTextSize(2);
  tft.print("Remaining Capacity:");
    }
  }
}

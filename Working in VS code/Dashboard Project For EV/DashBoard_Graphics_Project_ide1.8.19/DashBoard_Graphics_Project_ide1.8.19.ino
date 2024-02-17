#include <Adafruit_TFTLCD.h>
#include <Adafruit_GFX.h>
#include <TouchScreen.h>
#include <MCUFRIEND_kbv.h>

//MCUFRIEND_kbv tft;

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

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

float battery_capacity =0;
float distance =0;
float current =0;
float voltage =0;
float power = 0;
float powerkilo = 0;

float prebattery =0;
float predistance =0;
float currbattery =50;
float currdistance =100;

float i=0;
float j=0;

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 334);
boolean buttonEnabled = true;

void setup(void)
{
    Serial.begin(115200);
    tft.reset();
    
    uint16_t ID = tft.readID();
    Serial.print("TFT ID = 0x");
    Serial.println(ID, HEX);
    Serial.println("Calibrate for your Touch Panel");
    if (ID == 0xD3D3) ID = 0x9486; // write-only shield
    tft.begin(ID);
    tft.setRotation(1);            //PORTRAIT
    tft.fillScreen(BLACK);
    tft.setTextColor(YELLOW, BLACK);
    tft.setTextSize(2);
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
  tft.print("2nd Dashboard For EV");

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

  
    // testing();
    tft.setTextSize(2);
    tft.setCursor(15, 240);
    tft.setTextColor(GREEN,BLACK);
    tft.print("Teach Me Something");
   // delay(1000);

}


void loop(void)
{

  TSPoint p = ts.getPoint();                     //TO Make clickable button on these pixels area

  if (p.z > ts.pressureThreshhold) {
  
    p.x = map(p.x, TS_MAXX, TS_MINX, 0, 320);
    p.y = map(p.y, TS_MAXY, TS_MINY, 0, 480);

    if (p.x > -180 && p.x < 250 && p.y > 300 && p.y < 430 && buttonEnabled) {    //checking button clikable range
      Serial.println("Touch button press position");
 
      Serial.println(p.x);
      Serial.println(p.y);
      delay(30);
      buttonEnabled = false;

      pinMode(XM, OUTPUT);
      pinMode(YP, OUTPUT);
    tft.fillScreen(BLACK);
    tft.setTextColor(RED,BLACK);
    tft.setCursor(55, 180);
    tft.print("100");
    tft.setCursor(50, 120);
    tft.print("Battery");
    tft.setTextColor(BLUE,BLACK);
    tft.setCursor(212, 180);
    tft.print("120");
    tft.setCursor(200, 120);
    tft.print("Distance");
    tft.setCursor(30, 5);
    tft.fillRect(0,0,320,45,BLACK);
    tft.setTextColor(YELLOW,BLACK);
    tft.print("EV Dashboard Monitor");
    tft.setTextColor(WHITE,BLACK);
    
    tft.fillRect(55, 150,60,20,BLACK);
    tft.fillRect(212,150,60,20,BLACK);

   while(1){
    tft.setCursor(0,30);
    tft.print("Voltage:");
    tft.print(voltage,1);
    tft.print("V");
    tft.setCursor(160,30);
    tft.print("Current:");
    tft.print(current,1);
    tft.print("A");
    tft.setCursor(55, 150);
    tft.print(battery_capacity,1);
    tft.setCursor(212, 150);
    tft.print(distance,1);
    i=map(prebattery,0,100,0,300);
    j=map(currbattery,0,100,0,300);
    
   for (i; i<=j; i=i+0.1)
    {
    float j=i-150 ;
    float angle = (j / 57.2958)  - 1.57; 
    float x1= 80 + cos(angle) * 55;
    float y1 = 130+sin(angle) * 55;
    float x2= 80 + cos(angle) * 75;
    float y2 = 130+sin(angle) * 75; 
    tft.drawLine(x1, y1,x2,y2, RED);                //making battery graphics
    }
   i=map(prebattery,0,100,0,300);
   j=map(currbattery,0,100,0,300); 
    
   for (i-2; i>=j; i=i-0.05)
    {
    float j=i-150 ;
    float angle = (j / 57.2958)  - 1.57; 
    float x1= 80 + cos(angle) * 55;
    float y1 = 130+sin(angle) * 55;
    float x2= 80 + cos(angle) * 75;
    float y2 = 130+sin(angle) * 75;
    tft.drawLine(x1, y1,x2,y2, BLACK);                //removing battery graphics
   // tft.drawLine(x1+160, y1,x2+160,y2, MAGENTA);
    }

    i=map(predistance,0,100,0,300);
    j=map(currdistance,0,100,0,300); 
      
   for (i; i<=j; i=i+0.1)
    {
    float j=i-150 ;
    float angle = (j / 57.2958)  - 1.57; 
    float x1= 80 + cos(angle) * 55;
    float y1 = 130+sin(angle) * 55;
    float x2= 80 + cos(angle) * 75;
    float y2 = 130+sin(angle) * 75;
    
    tft.drawLine(x1+160, y1,x2+160,y2, BLUE);         //making Distance graphics
    }
   i=map(predistance,0,100,0,300);
   j=map(currdistance,0,100,0,300); 
    
  for (i-2; i>=j; i=i-0.05)
    {
    float j=i-150 ;
    float angle = (j / 57.2958)  - 1.57; 
    float x1= 80 + cos(angle) * 55;
    float y1 = 130+sin(angle) * 55;
    float x2= 80 + cos(angle) * 75;
    float y2 = 130+sin(angle) * 75;
    
    tft.drawLine(x1+160, y1,x2+160,y2, BLACK);    //Removing Distance graphics
    }
 
  prebattery=currbattery;
  predistance=currdistance;

  battery_capacity =random(0,100);                    //Assigning Random value for demonstration
  distance =random(0,60);
  voltage =random(0,66);
  current =random(0,99);
  power = voltage * current;


  Serial.println("Battery: "+String(battery_capacity)+"%");     //printing measurments on serial monitor
  Serial.println("Speed: "+String(distance)+"km/h");   
  Serial.println("Voltage: "+String(voltage)+"V");    
  Serial.println("Current: "+String(current)+"A");

    if(power >= 1000){                                          //Power in kilowatt
      powerkilo= power / 1000;
      Serial.println("Power: "+String(powerkilo)+"KW");
    } 

    else{                                                       //Power in watt
      Serial.println("Power: "+String(power)+"W");
    }   

  currbattery =battery_capacity;
  currdistance=distance;
  delay(1000);                                               //Refreshing Delay
    }
  }
 }  
}

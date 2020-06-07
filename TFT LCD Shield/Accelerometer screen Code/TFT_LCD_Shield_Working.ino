#include <SPI.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <Wire.h>

#include <Fonts/FreeSansBold24pt7b.h>
// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET 10 // Can alternately just connect to Arduino's reset pin
//helped a lot to configure Wire.h communication https://forum.arduino.cc/index.php?topic=400992.0

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define LTBLUE    0xB6DF
#define LTTEAL    0xBF5F
#define LTGREEN         0xBFF7
#define LTCYAN    0xC7FF
#define LTRED           0xFD34
#define LTMAGENTA       0xFD5F
#define LTYELLOW        0xFFF8
#define LTORANGE        0xFE73
#define LTPINK          0xFDDF
#define LTPURPLE  0xCCFF
#define LTGREY          0xE71C

#define BLUE            0x001F
#define TEAL    0x0438
#define GREEN           0x07E0
#define CYAN          0x07FF
#define RED           0xF800
#define MAGENTA       0xF81F
#define YELLOW        0xFFE0
#define ORANGE        0xFD20
#define PINK          0xF81F
#define PURPLE    0x801F
#define GREY        0xC618
#define WHITE         0xFFFF
#define BLACK         0x0000

#define DKBLUE        0x000D
#define DKTEAL    0x020C
#define DKGREEN       0x03E0
#define DKCYAN        0x03EF
#define DKRED         0x6000
#define DKMAGENTA       0x8008
#define DKYELLOW        0x8400
#define DKORANGE        0x8200
#define DKPINK          0x9009
#define DKPURPLE      0x4010
#define DKGREY        0x4A49

#define accel_module (0x53)

byte values[6];
int calibrate;
int value;

boolean graph_1 = true;
boolean graph_2 = true;

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

void setup(void) {
  initWire();
  calibrate = calcValue();
  initTFT();
  for(int i=0;i<300;i+=20){
    DrawBarChartV(20,  285, 40, 250, 0, 300 , 50, i , 4 , 0, BLUE, DKBLUE, BLUE, WHITE, BLACK, "Accelerate", graph_1,15);
    DrawBarChartV(143,  285, 40, 250, 0, 300 , 50, i , 4 , 0, RED, DKRED, RED, WHITE, BLACK, "Brake", graph_2,0);
  }
  for(int i=300;i>0;i-=20){
    DrawBarChartV(20,  285, 40, 250, 0, 300 , 50, i , 4 , 0, BLUE, DKBLUE, BLUE, WHITE, BLACK, "Accelerate", graph_1,15);
    DrawBarChartV(143,  285, 40, 250, 0, 300 , 50, i , 4 , 0, RED, DKRED, RED, WHITE, BLACK, "Brake", graph_2,0);
  }
}

void loop(void) {
  value=calcValue()-calibrate;
   if((value>0)&&(value<=300)){
    DrawBarChartV(20,  285, 40, 250, 0, 300 , 50, value , 4 , 0, BLUE, DKBLUE, BLUE, WHITE, BLACK, "Accelerate", graph_1,15);
    DrawBarChartV(143,  285, 40, 250, 0, 300 , 50, 0 , 4 , 0, RED, DKRED, RED, WHITE, BLACK, "Brake", graph_2,0);
   }
   else if((value<0)&&(value>=-300)){
    DrawBarChartV(20,  285, 40, 250, 0, 300 , 50, 0 , 4 , 0, BLUE, DKBLUE, BLUE, WHITE, BLACK, "Accelerate", graph_1,15);
    DrawBarChartV(143,  285, 40, 250, 0, 300 , 50, abs(value) , 4 , 0, RED, DKRED, RED, WHITE, BLACK, "Brake", graph_2,0);
   }
}

void DrawBarChartV(double x , double y , double w, double h , double loval , double hival , double inc , double curval ,  int dig , int dec, unsigned int barcolor, unsigned int voidcolor, unsigned int bordercolor, unsigned int textcolor, unsigned int backcolor, String label, boolean & redraw,int wordAllign)
{
  double stepval, range;
  double my, level;
  double i, data;
  // draw the border, scale, and label once
  // avoid doing this on every update to minimize flicker
  if (redraw == true) {
    redraw = false;

    tft.drawRect(x - 1, y - h - 1, w + 2, h + 2, bordercolor);
    tft.setTextColor(textcolor, backcolor);
    tft.setTextSize(2);
    tft.setCursor(x-wordAllign , y + 10);
    tft.println(label);
    // step val basically scales the hival and low val to the height
    // deducting a small value to eliminate round off errors
    // this val may need to be adjusted
    stepval = ( inc) * (double (h) / (double (hival - loval))) - .001;
    for (i = 0; i <= h; i += stepval) {
      my =  y - h + i;
      tft.drawFastHLine(x + w + 1, my,  5, textcolor);
      // draw lables
      tft.setTextSize(1);
      tft.setTextColor(textcolor, backcolor);
      tft.setCursor(x + w + 12, my - 3 );
      data = hival - ( i * (inc / stepval));
      tft.println(Format(data, dig, dec));
    }
  }
  // compute level of bar graph that is scaled to the  height and the hi and low vals
  // this is needed to accompdate for +/- range
  level = (h * (((curval - loval) / (hival - loval))));
  // draw the bar graph
  // write a upper and lower bar to minimize flicker cause by blanking out bar and redraw on update
  tft.fillRect(x, y - h, w, h - level,  voidcolor);
  tft.fillRect(x, y - level, w,  level, barcolor);
  // write the current value
  tft.setTextColor(textcolor, backcolor);
  tft.setTextSize(2);
  tft.setCursor(x , y - h - 23);
  tft.println(Format(curval, dig, dec));

}

String Format(double val, int dec, int dig ) {
  int addpad = 0;
  char sbuf[20];
  String condata = (dtostrf(val, dec, dig, sbuf));


  int slen = condata.length();
  for ( addpad = 1; addpad <= dec + dig - slen; addpad++) {
    condata = " " + condata;
  }
  return (condata);
}

void initWire(){
  Wire.begin();
  Wire.beginTransmission(accel_module);
  Wire.write(0x2D);
  Wire.write(0);
  Wire.endTransmission();
  Wire.beginTransmission(accel_module);
  Wire.write(0x2D);
  Wire.write(16);
  Wire.endTransmission();
  Wire.beginTransmission(accel_module);
  Wire.write(0x2D);
  Wire.write(8);
  Wire.endTransmission();
}

void initTFT(){
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  tft.setRotation(0);
}

int calcValue(){
  int xyzregister = 0x32;
  int x;

  Wire.endTransmission();
  Wire.beginTransmission(accel_module);
  Wire.write(xyzregister);
  Wire.endTransmission();

  Wire.beginTransmission(accel_module);
  Wire.requestFrom(accel_module, 6);

  int i=0;
  while(Wire.available()){
    values[i] = Wire.read();
    i++;
  }
  Wire.endTransmission();

  x=(((int)values[1])<<8)|values[0];
    
  return x;
}


/*
 * Author - Marcus Young - www.marcyoung.us
 * Version 1.0 - First working version. Could use some code cleanup
 */
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#include <SD.h>
#include <SPI.h>
#include <DHT.h>
#include <avr/eeprom.h>

#if not defined USE_ADAFRUIT_SHIELD_PINOUT 
#error "For use with the shield, make sure to #define USE_ADAFRUIT_SHIELD_PINOUT in the TFTLCD.h library file"
#endif

// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0 
#define WHITE           0xFFFF

// These are the pins for the shield!
#define YP A1  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 7   // can be a digital pin
#define XP 6   // can be a digital pin
#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0 
#define SD_CS 5 

// This is the setup information for the DHT temperature sensor
#define DHTPIN A4
#define DHTTYPE DHT11 //DHT11|DHT22

// Information for the filename to read from the SD Card
#define IMAGE_FILE "beer.bmp"
#define DATA_FILE "data.txt"

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, 0);
File dataFile;

#define MINPRESSURE 10
#define MAXPRESSURE 1000

/* Button box info
 Arduino Orientation
 
 -------------------------_
 |                       |_| <-- power in
 |      ____xtop--       |
 |  y   |         |y     |
 | left |         |right |__
 |      |_________|      |  | <-- usb
 |        xbottom        |__|
 |_______________________|
 */
// These are the boundaries for the default beer touch area
#define BUTTONXTOP 50
#define BUTTONXBOTTOM 190
#define BUTTONYLEFT 220
#define BUTTONYRIGHT 90

#define SOLENOID 2
#define FLOWPIN 3

volatile uint8_t lastflowtap1state;  //saved high/low state from flow meters
volatile uint16_t ratetimer = 0;  //simple counter to determine 1 second lapsed
volatile uint16_t tap1pulsecount = 0;  // number of flow sensor rotations in 1 second
volatile uint16_t tap1flowrate = 0;  //rpm
volatile uint32_t pouredTime = 0;  //saved time of last pour in ms
volatile uint8_t justpoured = 0;  //flag to prevent multiple checkins from top-offs

// Interrupt is called once a millisecond
SIGNAL(TIMER0_COMPA_vect) {
  ratetimer++;
  if (ratetimer == 1000) {  // 1 second has lapsed - calc RPMs
    ratetimer = 0;
    tap1flowrate = tap1pulsecount * 60;
    tap1pulsecount = 0;
  }
  uint8_t x = digitalRead(FLOWPIN);
  if (x != lastflowtap1state) {
    lastflowtap1state = x;
    if (x == HIGH) tap1pulsecount++;
  }
}

void useInterrupt(boolean v) {
  if (v) {
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  } else {
    TIMSK0 &= ~_BV(OCIE0A);
  }
}

void setup(void) {
  Serial.begin(9600);
  Serial.println("Paint!");

  tft.reset();

  uint16_t identifier = tft.readRegister(0x0);
  tft.begin(identifier); 

  readSdCard();

  pinMode(2, OUTPUT);
  dht.begin();
  
  pinMode(FLOWPIN, INPUT);
  digitalWrite(FLOWPIN, HIGH);
  lastflowtap1state = digitalRead(FLOWPIN);
  
  useInterrupt(true);
  refreshKegInfo(0);
}

boolean currentPress = false;
//int lastDrawTime = -1;
void loop() {
/*  
  if(lastDrawTime == -1 || millis() - lastDrawTime > 3000) {
    drawInfoOnScreen();
    lastDrawTime = millis();
  }
*/
  
  Point p = ts.getPoint();
  // if you're sharing pins, you'll need to fix the directions of the touchscreen pins!
  //pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //pinMode(YM, OUTPUT);
  /* 
    Make sure that NUMSAMPLES in libraries/TouchScreen/TouchScreen.cpp is set to 3!
    The default is two, meaning that after each cycle it looks for a new pressure, which 
    throws of 'hold' detection, since the next cycle will be 0 (no change in pressure)
  */

  boolean previousPress = currentPress;
  currentPress = isPressed(p);
  
  if(previousPress != currentPress) {
    pour(currentPress);
  }
}

void refreshKegInfo(double amountPoured) {
  tft.fillRect(0,0 , tft.width(), 55, BLACK);
  tft.setCursor(0, 10);
  tft.setTextColor(RED);
  tft.setTextSize(2);
  enableSPI();
  
  dataFile = SD.open(DATA_FILE);
  int counter=0;
  int numOunces;
  double pintsLeft;
  String nameOfBeer;
  while(dataFile.available()) {
    if(counter++ == 0) {
      numOunces = dataFile.parseInt();
      pintsLeft = (numOunces-amountPoured) / 16;
    } else {
      nameOfBeer = nameOfBeer + (char)dataFile.read();
    }
  }
  dataFile.close();
  dataFile = SD.open(DATA_FILE,FILE_WRITE);
  dataFile.seek(0);
  dataFile.println((int)(numOunces-amountPoured));
  dataFile.close();
  disableSPI();
  double temp = dht.readTemperature();
  tft.print(pintsLeft); tft.print(" pints left of "); tft.println(nameOfBeer); tft.print(" @"); tft.print((temp*1.8)+32); tft.println("*F");
}

boolean isPressed(Point p) {
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {

    // turn from 0->1023 to tft.width
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);
    if(p.x > BUTTONXTOP && p.x < BUTTONXBOTTOM &&
      p.y > BUTTONYRIGHT && p.y < BUTTONYLEFT) {
        return true;
    }
  }
  return false;
}

void pour(boolean buttonPressed) {
  if(buttonPressed) {
    digitalWrite(SOLENOID, HIGH);   // turn the flow  on (HIGH is the voltage level)
    pouredTime = millis();
  } else {
    digitalWrite(SOLENOID, LOW);   // turn the flow off

    uint32_t timePassed = millis() - pouredTime; //stop the timer and caclulate length
    double ratePouredPerMs = (((tap1flowrate / 6.5) * 33.814 ) / 3600000); //estimate of amount flowed per millisecond
    double estimatedAmountPoured = ratePouredPerMs * timePassed;
    refreshKegInfo(estimatedAmountPoured);
    while(tap1flowrate > 0) {
      ;//let the flow meter finish spinning down before we let the cycles watch for more pours
    }
  }
}

/**************************************
  This section is for the image drawing
***************************************/

//Global identifiers for the 'wallpaper'
File bmpFile; 
int bmpWidth, bmpHeight;
uint8_t bmpDepth, bmpImageoffset;
int8_t saved_spimode;

void readSdCard() {
  tft.setRotation(0);
  if (!SD.begin(SD_CS)) { 
    return; 
  }
  bmpFile = SD.open(IMAGE_FILE);
  if (! bmpFile) { 
    while (1); 
  }
  if (! bmpReadHeader(bmpFile)) { 
    return; 
  }
  disableSPI();    // release SPI so we can use those pins to draw
  bmpdraw(bmpFile, 0, 0);
  // disable the SD card interface after we are done!
  //disableSPI();
}

//snip
#define BUFFPIXEL 20

void disableSPI(void) {
  saved_spimode = SPCR;
  SPCR = 0;
}

void enableSPI(void) {
  SPCR = saved_spimode; 
}

void bmpdraw(File f, int x, int y) {

  enableSPI();     // enable the hardware SPI to talk to the SD card
  bmpFile.seek(bmpImageoffset);
  disableSPI();    // release it so we can use those pins

  uint32_t time = millis();
  uint16_t p;
  uint8_t g, b;
  int i, j;

  uint8_t sdbuffer[3 * BUFFPIXEL];  // 3 * pixels to buffer
  uint8_t buffidx = 3*BUFFPIXEL;

  Serial.print("rotation = "); 
  Serial.println(tft.getRotation(), DEC);

  for (i=0; i< bmpHeight; i++) {
    // bitmaps are stored with the BOTTOM line first so we have to move 'up'

    if (tft.getRotation() == 3) {
      tft.writeRegister16(ILI932X_ENTRY_MOD, 0x1028);
      tft.goTo(x+i, y); 
    } 
    else if  (tft.getRotation() == 2) {
      tft.writeRegister16(ILI932X_ENTRY_MOD, 0x1020);
      tft.goTo(x+bmpWidth, y+i); 
    } 
    else if  (tft.getRotation() == 1) {
      tft.writeRegister16(ILI932X_ENTRY_MOD, 0x1018);
      tft.goTo(x+bmpHeight-1-i, y); 
    } 
    else if  (tft.getRotation() == 0) {
      tft.writeRegister16(ILI932X_ENTRY_MOD, 0x1030);
      tft.goTo(x, y+bmpHeight-i); 
    }

    for (j=0; j<bmpWidth; j++) {
      // read more pixels
      if (buffidx >= 3*BUFFPIXEL) {
        enableSPI();     // enable the hardware SPI to talk to the SD card
        bmpFile.read(sdbuffer, 3*BUFFPIXEL);
        disableSPI();    // release it so we can use those pins
        buffidx = 0;
      }

      // convert pixel from 888 to 565
      b = sdbuffer[buffidx++];     // blue
      g = sdbuffer[buffidx++];     // green
      p = sdbuffer[buffidx++];     // red

      p >>= 3;
      p <<= 6;

      g >>= 2;
      p |= g;
      p <<= 5;

      b >>= 3;
      p |= b;

      // write out the 16 bits of color
      tft.writeData(p);
    }
  }
  tft.writeRegister16(ILI932X_ENTRY_MOD, 0x1030);
  Serial.print(millis() - time, DEC);
  Serial.println(" ms");
}

boolean bmpReadHeader(File f) {
  // read header
  uint32_t tmp;

  if (read16(f) != 0x4D42) {
    // magic bytes missing
    return false;
  }

  // read file size
  tmp = read32(f);  
  Serial.print("size 0x"); 
  Serial.println(tmp, HEX);

  // read and ignore creator bytes
  read32(f);

  bmpImageoffset = read32(f);  
  Serial.print("offset "); 
  Serial.println(bmpImageoffset, DEC);

  // read DIB header
  tmp = read32(f);
  Serial.print("header size "); 
  Serial.println(tmp, DEC);
  bmpWidth = read32(f);
  bmpHeight = read32(f);


  if (read16(f) != 1)
    return false;

  bmpDepth = read16(f);
  Serial.print("bitdepth "); 
  Serial.println(bmpDepth, DEC);

  if (read32(f) != 0) {
    // compression not supported!
    return false;
  }

  Serial.print("compression "); 
  Serial.println(tmp, DEC);

  return true;
}

/*********************************************/

// These read data from the SD card file and convert them to big endian 
// (the data is stored in little endian format!)

// LITTLE ENDIAN!
uint16_t read16(File f) {
  uint16_t d;
  uint8_t b;
  b = f.read();
  d = f.read();
  d <<= 8;
  d |= b;
  return d;
}


// LITTLE ENDIAN!
uint32_t read32(File f) {
  uint32_t d;
  uint16_t b;

  b = read16(f);
  d = read16(f);
  d <<= 16;
  d |= b;
  return d;
}


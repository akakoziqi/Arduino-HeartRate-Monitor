#include <Adafruit_GFX.h>        //OLED libraries
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "MAX30105.h"           //MAX3010x library
#include "heartRate.h"          //Heart rate calculating algorithm

MAX30105 particleSensor;

const byte RATE_SIZE = 5; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;
bool beatSensed = false;


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define beeper 9

Adafruit_SSD1306 display(OLED_RESET); //Declaring the display name (display)

static const unsigned char PROGMEM noboom[] =
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const unsigned char PROGMEM boom[] =
{ 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00,
  0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00,
  0xF8, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0xF8,
  0x01, 0xC0, 0x00, 0x01, 0xD8, 0x01, 0xC0, 0x07, 0x01, 0xDC, 0x01, 0xE0, 0x07, 0x01, 0xDC, 0x01,
  0xE0, 0x07, 0x81, 0xDC, 0x03, 0xE0, 0x07, 0x81, 0xCC, 0x03, 0xE0, 0x07, 0x81, 0xCC, 0x03, 0xE0,
  0x0F, 0x81, 0xCE, 0x03, 0xE0, 0x0F, 0x81, 0x8E, 0x03, 0xE0, 0x0F, 0xC1, 0x8E, 0x03, 0xF0, 0x0D,
  0xC3, 0x8E, 0x03, 0x70, 0x1D, 0xC3, 0x8E, 0x03, 0x30, 0x19, 0xC3, 0x8F, 0x07, 0x38, 0x19, 0xC3,
  0x0F, 0x07, 0x3F, 0xF8, 0xFF, 0x07, 0x7F, 0x1F, 0xF8, 0xFF, 0x07, 0x7E, 0x1F, 0x00, 0x00, 0x07,
  0x60, 0x00, 0x00, 0x00, 0x07, 0x60, 0x00, 0x00, 0x00, 0x07, 0x60, 0x00, 0x00, 0x00, 0x07, 0x60,
  0x00, 0x00, 0x00, 0x03, 0x60, 0x00, 0x00, 0x00, 0x03, 0xE0, 0x00, 0x00, 0x00, 0x03, 0xE0, 0x00,
  0x00, 0x00, 0x03, 0xE0, 0x00, 0x00, 0x00, 0x03, 0xE0, 0x00, 0x00, 0x00, 0x01, 0xC0, 0x00, 0x00,
  0x00, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x01, 0xC0, 0x00, 0x00, 0x00,
  0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00,
};

static const unsigned char PROGMEM str_1[] =
{
  0x00, 0x40, 0x40, 0x40, 0x27, 0xFC, 0x20, 0x40, 0x03, 0xF8, 0x00, 0x40, 0xE7, 0xFE, 0x20, 0x00,
  0x23, 0xF8, 0x22, 0x08, 0x23, 0xF8, 0x22, 0x08, 0x2B, 0xF8, 0x32, 0x08, 0x22, 0x28, 0x02, 0x10,
};//请

static const unsigned char PROGMEM str_2[] =
{
  0x20, 0x40, 0x10, 0x40, 0x00, 0x40, 0xFE, 0x80, 0x20, 0xFE, 0x21, 0x08, 0x3E, 0x88, 0x24, 0x88,
  0x24, 0x88, 0x24, 0x50, 0x24, 0x50, 0x24, 0x20, 0x44, 0x50, 0x54, 0x88, 0x89, 0x04, 0x02, 0x02,
};//放

static const unsigned char PROGMEM str_3[] =
{
  0x7F, 0xFC, 0x44, 0x44, 0x7F, 0xFC, 0x01, 0x00, 0x7F, 0xFC, 0x01, 0x00, 0x1F, 0xF0, 0x10, 0x10,
  0x1F, 0xF0, 0x10, 0x10, 0x1F, 0xF0, 0x10, 0x10, 0x1F, 0xF0, 0x10, 0x10, 0xFF, 0xFE, 0x00, 0x00,
};//置

static const unsigned char PROGMEM str_4[] =
{
  0x01, 0x00, 0x02, 0x80, 0x04, 0x40, 0x0A, 0x20, 0x31, 0x18, 0xDF, 0xF6, 0x10, 0x10, 0x1F, 0xF0,
  0x10, 0x10, 0x1F, 0xF0, 0x10, 0x08, 0x11, 0x90, 0x10, 0x60, 0x12, 0x10, 0x14, 0x08, 0x18, 0x04,
};//食

static const unsigned char PROGMEM str_5[] =
{
  0x11, 0x00, 0x11, 0x04, 0x11, 0x38, 0x11, 0xC0, 0xFD, 0x02, 0x11, 0x02, 0x10, 0xFE, 0x14, 0x00,
  0x19, 0xFC, 0x31, 0x04, 0xD1, 0x04, 0x11, 0xFC, 0x11, 0x04, 0x11, 0x04, 0x51, 0xFC, 0x21, 0x04,
};//指

static const unsigned char PROGMEM str_6[] =
{
  0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x80, 0x00, 0x80, 0x04, 0x00, 0x04, 0x08, 0x24, 0x04,
  0x24, 0x04, 0x24, 0x02, 0x44, 0x02, 0x44, 0x12, 0x84, 0x10, 0x04, 0x10, 0x03, 0xF0, 0x00, 0x00,
};//心

static const unsigned char PROGMEM str_7[] =
{
  0x02, 0x00, 0x01, 0x00, 0x7F, 0xFC, 0x02, 0x00, 0x44, 0x44, 0x2F, 0x88, 0x11, 0x10, 0x22, 0x48,
  0x4F, 0xE4, 0x00, 0x20, 0x01, 0x00, 0xFF, 0xFE, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
};//率

static const unsigned char PROGMEM str_8[] =
{
  0x10, 0x40, 0x10, 0x40, 0x10, 0xA0, 0x10, 0xA0, 0xFD, 0x10, 0x12, 0x08, 0x35, 0xF6, 0x38, 0x00,
  0x54, 0x88, 0x50, 0x48, 0x92, 0x48, 0x11, 0x50, 0x11, 0x10, 0x10, 0x20, 0x17, 0xFE, 0x10, 0x00,
};//检

static const unsigned char PROGMEM str_9[] =
{
  0x00, 0x04, 0x27, 0xC4, 0x14, 0x44, 0x14, 0x54, 0x85, 0x54, 0x45, 0x54, 0x45, 0x54, 0x15, 0x54,
  0x15, 0x54, 0x25, 0x54, 0xE5, 0x54, 0x21, 0x04, 0x22, 0x84, 0x22, 0x44, 0x24, 0x14, 0x08, 0x08,
};//测

static const unsigned char PROGMEM str_10[] =
{
  0x00, 0x00, 0x1F, 0xF0, 0x10, 0x10, 0x1F, 0xF0, 0x10, 0x10, 0xFF, 0xFE, 0x00, 0x00, 0x1F, 0xF0,
  0x11, 0x10, 0x1F, 0xF0, 0x11, 0x10, 0x1F, 0xF0, 0x01, 0x00, 0x1F, 0xF0, 0x01, 0x00, 0x7F, 0xFC,
};//量

static const unsigned char PROGMEM str_11[] =
{
  0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x3F, 0xF8, 0x21, 0x08, 0x21, 0x08, 0x21, 0x08,
  0x21, 0x08, 0x21, 0x08, 0x3F, 0xF8, 0x21, 0x08, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
};//中

static const unsigned char PROGMEM str_12[] =
{
  0x00, 0x50, 0x7C, 0x50, 0x44, 0x50, 0x45, 0x52, 0x44, 0xD4, 0x7C, 0x58, 0x10, 0x50, 0x10, 0x58,
  0x10, 0xD4, 0x5D, 0x52, 0x50, 0x50, 0x50, 0x50, 0x50, 0x92, 0x5C, 0x92, 0xE1, 0x12, 0x02, 0x0E,
};//跳


void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //Start the OLED display
  display.display();
  delay(1000);
  // Initialize sensor
  particleSensor.begin(Wire, I2C_SPEED_FAST); //Use default I2C port, 400kHz speed
  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  pinMode(beeper, OUTPUT);
}

void loop() {
  long irValue = particleSensor.getIR();    //Reading the IR value it will permit us to know if there's a finger on the sensor or not

  if (checkForBeat(irValue) == true)                        //If a heart beat is detected
  {
    beatSensed = true;

    long delta = millis() - lastBeat;                   //Measure duration between two beats
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);           //Calculating the BPM

    if (beatsPerMinute < 255 && beatsPerMinute > 20)               //To calculate the average we strore some values (4) then do some math to calculate the average
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  if (irValue < 50000) {      //If no finger is detected it inform the user and put the average BPM to 0 or it will be stored for the next measure
    displayNoFinger();
  }

  if (beatSensed) {
    displayBeat();
    digitalWrite(beeper, HIGH);

    if ((millis() - lastBeat) > 50) {
      displayNoBeat();
      digitalWrite(beeper, LOW);
      beatSensed = false;
    }
  }
}

void displayBeat() {
  display.clearDisplay();                                 //Clear the display
  display.drawBitmap(16, 0, str_6, 16, 16, 1);
  display.drawBitmap(32, 0, str_12, 16, 16, 1);
  display.drawBitmap(80, 0, str_6, 16, 16, 1);
  display.drawBitmap(96, 0, str_7, 16, 16, 1);
  display.drawBitmap(12, 20, boom, 40, 40, WHITE);    //Draw the second picture (bigger heart)
  display.setTextSize(2);                                //And still displays the average BPM
  display.setTextColor(WHITE);
  display.setCursor(80, 20);
  display.println(beatAvg);
  display.setCursor(80, 44);
  display.println("BPM");
  display.display();
}

void displayNoBeat() {
  display.clearDisplay();                                 //Clear the display
  display.drawBitmap(16, 0, str_6, 16, 16, 1);
  display.drawBitmap(32, 0, str_12, 16, 16, 1);
  display.drawBitmap(80, 0, str_6, 16, 16, 1);
  display.drawBitmap(96, 0, str_7, 16, 16, 1);
  display.drawBitmap(12, 20, noboom, 40, 40, WHITE);    //Draw the second picture (bigger heart)
  display.setTextSize(2);                                //And still displays the average BPM
  display.setTextColor(WHITE);
  display.setCursor(80, 20);
  display.println(beatAvg);
  display.setCursor(80, 44);
  display.println("BPM");
  display.display();
}

void displayNoFinger() {
  display.clearDisplay();
  display.drawBitmap(32, 0, str_6, 16, 16, 1);
  display.drawBitmap(48, 0, str_7, 16, 16, 1);
  display.drawBitmap(64, 0, str_8, 16, 16, 1);
  display.drawBitmap(80, 0, str_9, 16, 16, 1);

  display.drawBitmap(24, 24, str_1, 16, 16, 1);
  display.drawBitmap(40, 24, str_2, 16, 16, 1);
  display.drawBitmap(56, 24, str_3, 16, 16, 1);
  display.drawBitmap(72, 24, str_4, 16, 16, 1);
  display.drawBitmap(88, 24, str_5, 16, 16, 1);
  display.display();
}

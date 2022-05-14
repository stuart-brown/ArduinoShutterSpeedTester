#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include "Fonts/FreeSans12pt7b.h"

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define SSD1306_NO_SPLASH 0

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LASER_OUTPUT 2
#define DETECTOR_INPUT 3

#define DARK  1
#define LIGHT !DARK

int last_read;
unsigned long ts_start_us;
int second_line_y_pos = 0;

void setup() 
{
  Serial.begin(115200);
  Serial.flush();

  pinMode(LASER_OUTPUT, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(DETECTOR_INPUT, INPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;) {};  // Don't proceed, loop forever
  }

  display.clearDisplay();
  //display.setFont(&FreeSans12pt7bBitmaps);
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);
  display.setCursor(0, 0);
  display.print("Ready");
  display.display();
  Serial.println("Ready...");

  second_line_y_pos = display.height() / 2;
  last_read = digitalRead(DETECTOR_INPUT);

  Serial.println("Turning on LASER_OUTPUT");
  digitalWrite(LASER_OUTPUT, HIGH);

  last_read = DARK;
  ts_start_us = micros();
}


//
//
//                  +----------------------+
//                  |                      |
//    --------------+                      +------------------
//
//                  |<----- ts_delta ----->|   
//             ts_start                  ts_end
//
//
//
//
void loop() 
{
  int new_read = digitalRead(DETECTOR_INPUT);
  if (last_read == DARK && new_read == LIGHT)
  {
    ts_start_us = micros();
  }

  if (last_read == LIGHT && new_read == DARK)
  {
    unsigned long ts_end_us = micros();
    double ts_delta_us = ((double)ts_end_us - (double)ts_start_us);
    Serial.print(ts_end_us);
    Serial.print(" - ");
    Serial.print(ts_start_us);
    Serial.print(" = ");
    Serial.print(ts_delta_us,2);
    Serial.println(" us");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(ts_delta_us / 1000.0, 1);
    display.print("ms");

    double shutter_speed = 1000000.0 / (double)ts_delta_us;
    Serial.print("Shutter Speed = 1/");
    Serial.println(shutter_speed, 1);
    display.setCursor(0, second_line_y_pos);
    display.print("1/");
    display.print(shutter_speed, 1);
    display.display();
    Serial.println("---");
  }

  if (new_read != last_read)
  {
    Serial.print("new_read:");
    Serial.print(new_read==DARK?"Dark":"Light");
    Serial.print("  last_read:");
    Serial.println(last_read==DARK?"Dark":"Light");
    last_read = new_read;
    digitalWrite(LED_BUILTIN, new_read);
  }
}

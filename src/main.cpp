#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PinChangeInterrupt.h>

#include "version.h"

// When turned on all the timestamps are printed to the serial console
// When turned off only the results of the calculations are printed
#define FULL_DEBUG 0

#define SCREEN_WIDTH_PIXELS 128 // OLED display width, in pixels
#define SCREEN_HEIGHT_PIXELS 64 // OLED display height, in pixels

// Font sizes for a single letter
// Text Size 3 = 18x24 pixels
// Text Size 2 = 12x16 pixels
// Text Size 1 = 6x8 pixels
#define TEXT_SIZE_3_WIDTH_PIXELS  18
#define TEXT_SIZE_3_HEIGHT_PIXELS 24
#define TEXT_SIZE_2_WIDTH_PIXELS  12
#define TEXT_SIZE_2_HEIGHT_PIXELS 16
#define TEXT_SIZE_1_WIDTH_PIXELS   6
#define TEXT_SIZE_1_HEIGHT_PIXELS  8

// Prevent the Ada Fruit GFX library from showing an Ada Fruit logo at boot
#define SSD1306_NO_SPLASH 0

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 2  0(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET 4        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // I2C address for OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH_PIXELS, SCREEN_HEIGHT_PIXELS, &Wire, OLED_RESET);

// Size of the blank gap between lines of text on the display
#define GAP_PIXELS 5

#define LASER_1_OUTPUT 5
#define LASER_2_OUTPUT 6
#define LASER_3_OUTPUT 7

#define DETECTOR_1_INPUT 2
#define DETECTOR_2_INPUT 3
#define DETECTOR_3_INPUT 4

volatile unsigned long ts_1_start_us = 0;
volatile unsigned long ts_1_end_us = 0;
volatile bool sensor_1_triggered = false;

volatile unsigned long ts_2_start_us = 0;
volatile unsigned long ts_2_end_us = 0;
volatile bool sensor_2_triggered = false;

volatile unsigned long ts_3_start_us = 0;
volatile unsigned long ts_3_end_us = 0;
volatile bool sensor_3_triggered = false;

double shutter_speed_ms = 0.0;
double fractional_shutter_speed = 0.0;
double curtain_1_travel_time_ms = 0.0;
double curtain_2_travel_time_ms = 0.0;

void detector1(void);
void detector2(void);
void detector3(void);

void setup()
{
  Serial.begin(115200);
  Serial.flush();

  pinMode(DETECTOR_1_INPUT, INPUT);
  pinMode(DETECTOR_2_INPUT, INPUT);
  pinMode(DETECTOR_3_INPUT, INPUT);

  pinMode(LASER_1_OUTPUT, OUTPUT);
  pinMode(LASER_2_OUTPUT, OUTPUT);
  pinMode(LASER_3_OUTPUT, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
    {
    }; // Don't proceed, loop forever
  }

  // No reset line on the display. So clear it and
  // wait for a short delay to make reboot obvious
  display.clearDisplay();
  display.display();
  delay(1); 

  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);
  display.setTextWrap(false);

  display.setTextSize(3);
  display.setCursor(TEXT_SIZE_3_WIDTH_PIXELS, (SCREEN_HEIGHT_PIXELS - TEXT_SIZE_3_HEIGHT_PIXELS - TEXT_SIZE_1_HEIGHT_PIXELS) / 2);
  display.print("Ready");

  display.setTextSize(1);
  display.setCursor(0, SCREEN_HEIGHT_PIXELS - TEXT_SIZE_1_HEIGHT_PIXELS);
  display.print("v");
  display.print(VERSION_MAJOR);
  display.print(".");
  display.print(VERSION_MINOR);
  display.print(".");
  display.print(VERSION_REV);

  display.display();

  Serial.println("Ready... turning on lasers!");
  digitalWrite(LASER_1_OUTPUT, HIGH);
  digitalWrite(LASER_2_OUTPUT, HIGH);
  digitalWrite(LASER_3_OUTPUT, HIGH);

  // setup interrupts for the laser detectors
  attachPCINT(digitalPinToPCINT(DETECTOR_1_INPUT), detector1, CHANGE);
  attachPCINT(digitalPinToPCINT(DETECTOR_2_INPUT), detector2, CHANGE);
  attachPCINT(digitalPinToPCINT(DETECTOR_3_INPUT), detector3, CHANGE);
}

//
//           S3      S2      S1
//            +       +       +
//       ---------------+   +-------------------
//                      |   |
//          Curtain 1   |   |   Curtain 2
//           Leading    |   |    Trailing
//         <----------  |   |  <----------
//                      |   |
//       ---------------+   +-------------------
//
//            +----------------------+
//            |                      |
//   S1 ------+                      +--------------------------
//            .       +----------------------+
//            .       |              .       |
//   S2 --------------+              .       +------------------
//            .       .       +----------------------+
//            .       .       |      .       .       |
//   S3 ----------------------+      .       .       +----------
//            .       .       .      .       .       .
//            .       .       .      .       .       .
//            .       .       .      .       .       .
//            |<------------->|      .       .       .
//            |   t_1_delta   |      .       .       .
//        ts_1_start  .   ts_3_start .       .       .
//                    .       .      .       .       .
//                    .       .      .       .       .
//                    |<-------------------->|       .
//                    |       t_2_delta      |       .
//              ts_2_start           .   ts_2_end    .
//                                   .       .       .
//                                   .       .       .
//                                   |<------------->|
//                                   |   t_3_delta   |
//                                ts_1_end        ts_3_end
//
//   Curtain 1 travel time = t_1_delta
//   Curtain 2 travel time = t_3_delta
//   Shutter speed = t_2_delta
//

void detector1(void) 
{
  if (digitalRead(DETECTOR_1_INPUT))
  {
    ts_1_end_us = micros();
    sensor_1_triggered = true;
  }
  else
  {
    ts_1_start_us = micros();
  }
}

void detector2(void) 
{
  int d2 = digitalRead(DETECTOR_2_INPUT);
  digitalWrite(LED_BUILTIN, d2 ^ 1);
  if (d2)
  {
    ts_2_end_us = micros();
    sensor_2_triggered = true;   
  }
  else
  {
    ts_2_start_us = micros();
  }
}

void detector3(void) 
{
  if (digitalRead(DETECTOR_3_INPUT))
  {
    ts_3_end_us = micros();
    sensor_3_triggered = true;
  }
  else
  {
    ts_3_start_us = micros();
  }
}

void loop()
{
  // --------- calculate ---------
  bool display_refresh = false;

    #if FULL_DEBUG
    if (sensor_1_triggered && sensor_3_triggered)
    {
      Serial.print("t1_s=");
      Serial.print(ts_1_start_us);
      Serial.print("us   t1_e=");
      Serial.print(ts_1_end_us);
      Serial.println("us");
    }
    if (sensor_2_triggered)
    {
      Serial.print("t2_s=");
      Serial.print(ts_2_start_us);
      Serial.print("us   t2_e=");
      Serial.print(ts_2_end_us);
      Serial.println("us");
    }
    if (sensor_1_triggered && sensor_3_triggered)
    {
      Serial.print("t3_s=");
      Serial.print(ts_3_start_us);
      Serial.print("us   t3_e=");
      Serial.print(ts_3_end_us);
      Serial.println("us");
    }
    #endif

  // Calculate sensor 2 independant from sensors 1 & 3 so that
  // a single sensor can be used to measure only shutter speed
  // or all 3 to measure shutter speed and curtain travel with
  // out needing a mode change switch
  if (sensor_2_triggered)
  {
    display_refresh = true;
    double shutter_speed_us = (double)(ts_2_end_us - ts_2_start_us);
    shutter_speed_ms = shutter_speed_us / 1000.0;
    fractional_shutter_speed = 1000000.0 / shutter_speed_us;
    Serial.print("Shutter Speed = ");
    Serial.print(shutter_speed_ms, 1);
    Serial.println(" ms");
    Serial.print("Fractional Shutter Speed = 1/");
    Serial.println(fractional_shutter_speed, 1);
    Serial.println("---");
    sensor_2_triggered = false;
  }
  if (sensor_1_triggered && sensor_3_triggered)
  {
    display_refresh = true;
    curtain_1_travel_time_ms = (double)(ts_3_start_us - ts_1_start_us)/1000.0;
    curtain_2_travel_time_ms = (double)(ts_3_end_us - ts_1_end_us)/1000.0;
    
    Serial.print("Curtain 1 travel time=");
    Serial.print(curtain_1_travel_time_ms,1);
    Serial.println(" ms");
    Serial.print("Curtain 2 travel time=");
    Serial.print(curtain_2_travel_time_ms,1);
    Serial.println(" ms");
    Serial.println("---");
    sensor_1_triggered = false;
    sensor_3_triggered = false;
  }

  // --------- display ---------
  if (display_refresh)
  {
    display.clearDisplay();

    display.setTextSize(2);

    display.setCursor(0, 0);
    display.print(shutter_speed_ms, 1);
    display.print("ms");

    display.setCursor(0, TEXT_SIZE_2_HEIGHT_PIXELS + GAP_PIXELS);
    display.print("1/");
    display.print(fractional_shutter_speed, 1);

    display.setTextSize(1);

    display.setCursor(0, SCREEN_HEIGHT_PIXELS - 2*TEXT_SIZE_1_HEIGHT_PIXELS - GAP_PIXELS);
    display.print("c1:");
    display.print(curtain_1_travel_time_ms, 1);
    display.print("ms");

    display.setCursor(0, SCREEN_HEIGHT_PIXELS - TEXT_SIZE_1_HEIGHT_PIXELS);
    display.print("c2:");
    display.print(curtain_2_travel_time_ms, 1);
    display.print("ms");

    display.display();
  }
}

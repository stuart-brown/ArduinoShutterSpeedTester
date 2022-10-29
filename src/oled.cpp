#include <SPI.h>
#include <Wire.h>              // wire library used for I2C
#include <Adafruit_GFX.h>      // include Adafruit graphics library
#include <Adafruit_SSD1306.h>  // Adafruit SSD1306 OLED library

#include "oled.h"
#include "version.h"

#define SCREEN_WIDTH_px  128 // display width, in pixels
#define SCREEN_HEIGHT_px  64 // display height, in pixels

// Text sizes
#define READY_TEXT_SIZE    3
#define VERSION_TEXT_SIZE  1

// Size of the blank gap between lines of text on the display
#define GAP_PIXELS 5

// pins for SSD1306 OLED connected to hardware I2C
// The pins for I2C are defined by the Wire-library. 
// For example:
//  On an arduino UNO:       A4(SDA), A5(SCL)
//  On an arduino MEGA 2560: 20(SDA), 21(SCL)
//  On an arduino LEONARDO:   2(SDA),  3(SCL)
#define OLED_RESET         -1  // NOT USED -> Reset pin # (or -1 if sharing Arduino reset pin)
#define OLED_I2C_ADDRESS 0x3C  // I2C address for OLED display

// Prevent the Ada Fruit GFX library from showing an Ada Fruit logo at boot
#define SSD1306_NO_SPLASH 0

// text size for a single letter
// size 1 is 6x8 pixels (wxh)
// size 2 is 12x16 pixels viz 2 x size 1
// size 3 is 18x24 pixels viz 3 x size 1
#define BASE_TEXT_WIDTH_px  6
#define BASE_TEXT_HEIGHT_px 8

// initialize SSD1306 OLED library
Adafruit_SSD1306 oled_display(SCREEN_WIDTH_px, SCREEN_HEIGHT_px, &Wire, OLED_RESET);

//---------------------------------------------------
// Helper function called once at startup to 
// setup the display (for example draw borders)
//---------------------------------------------------
void oled_setup()
{
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  // SSD1306_EXTERNALVCC = use externl voltage
  if (!oled_display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS))
  {
    Serial.println(F("SSD1306 begin() failed!"));
    for (;;)
    {
    }; // Don't proceed, loop forever
  }

  // No reset line on the display. So clear it and
  // wait for a short delay to make reboot obvious
  oled_display.clearDisplay();
  oled_display.display();
  delay(1);

  oled_display.setTextColor(SSD1306_WHITE);
  oled_display.cp437(true);
  oled_display.setTextWrap(false);

  oled_display.setTextSize(READY_TEXT_SIZE);
  oled_display.setCursor(READY_TEXT_SIZE*BASE_TEXT_WIDTH_px, (SCREEN_HEIGHT_px - BASE_TEXT_HEIGHT_px * (READY_TEXT_SIZE+VERSION_TEXT_SIZE)) / 2);
  oled_display.print("Ready");
  
  oled_display.setTextSize(VERSION_TEXT_SIZE);
  oled_display.setCursor(0, SCREEN_HEIGHT_px - VERSION_TEXT_SIZE*BASE_TEXT_HEIGHT_px);
  oled_display.print("v");
  oled_display.print(VERSION_MAJOR);
  oled_display.print(".");
  oled_display.print(VERSION_MINOR);
  oled_display.print(".");
  oled_display.print(VERSION_REV);

  oled_display.display();
}

//---------------------------------------------------
// Print values to the screen.
//---------------------------------------------------
void oled_show_values( double shutter_speed_1_ms, 
                      double shutter_speed_2_ms,
                      double shutter_speed_3_ms,
                      double fractional_shutter_speed_1, 
                      double fractional_shutter_speed_2,
                      double fractional_shutter_speed_3,
                      double curtain_1_travel_time_ms,
                      double curtain_2_travel_time_ms )
{
  const int SPEED_TEXT_SIZE = 2;
  const int TRAVEL_TEXT_SIZE = 1;

  oled_display.clearDisplay();

  oled_display.setTextSize(SPEED_TEXT_SIZE); 

  oled_display.setCursor(0, 0);
  oled_display.print(shutter_speed_2_ms, 1);
  oled_display.print("ms");

  oled_display.setCursor(0, SPEED_TEXT_SIZE*BASE_TEXT_HEIGHT_px + GAP_PIXELS);
  oled_display.print("1/");
  oled_display.print(fractional_shutter_speed_2, 1);

  oled_display.setTextSize(TRAVEL_TEXT_SIZE);

  oled_display.setCursor(0, SCREEN_HEIGHT_px - 2*TRAVEL_TEXT_SIZE*BASE_TEXT_HEIGHT_px - GAP_PIXELS);
  oled_display.print("c1:");
  oled_display.print(curtain_1_travel_time_ms, 1);
  oled_display.print("ms");

  oled_display.setCursor(SCREEN_WIDTH_px * 2/3, SCREEN_HEIGHT_px - 2*TRAVEL_TEXT_SIZE*BASE_TEXT_HEIGHT_px - GAP_PIXELS);
  oled_display.print("1/");
  oled_display.print(fractional_shutter_speed_1, 1);

  oled_display.setCursor(0, SCREEN_HEIGHT_px - TRAVEL_TEXT_SIZE*BASE_TEXT_HEIGHT_px);
  oled_display.print("c2:");
  oled_display.print(curtain_2_travel_time_ms, 1);
  oled_display.print("ms");

  oled_display.setCursor(SCREEN_WIDTH_px * 2/3, SCREEN_HEIGHT_px - TRAVEL_TEXT_SIZE*BASE_TEXT_HEIGHT_px);
  oled_display.print("1/");
  oled_display.print(fractional_shutter_speed_3, 1);

  oled_display.display();
}
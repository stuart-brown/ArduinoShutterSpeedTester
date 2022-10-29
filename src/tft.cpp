#include <SPI.h>               // TFT display connected via SPI
#include <Adafruit_ILI9341.h>  // Adafruit ILI9341 TFT library
#include <Adafruit_GFX.h>      // include Adafruit graphics library

#include "tft.h"
#include "version.h"

//  +-----------------------------------------------------------+
//  |                       Name (version)                      | <-- Version Text
//  |-----------------------------------------------------------|
//  |                       Shutter Speed                       | <-- Heading Text
//  |-------------------+-------------------+-------------------|
//  |                   |                   |                   |
//  |         1         |         1         |         1         | <-- Main Speed Text for center speed
//  |      -------      |      -------      |      -------      |     Minor Speed Text for edge speeds
//  |       xxx.x       |       xxx.x       |       xxx.x       | 
//  |                   |                   |                   |
//  |      xx.x ms      |      xx.x ms      |      xx.x ms      | <-- Minor Speed Text
//  |                   |                   |                   |
//  |-------------------+-------------------+-------------------|
//  |                    Curtain travel time                    | <-- Heading Text
//  |------------------------------+----------------------------|
//  |                              |                            |
//  |            yyy.y ms          |          yyy.y ms          | <-- Travel Time Text
//  |                              |                            |
//  +-----------------------------------------------------------+
//
#define SCREEN_WIDTH_px  320 // display width, in pixels
#define SCREEN_HEIGHT_px 240 // display height, in pixels

// Text sizes
// size 1 is 6 x 8 pixels (w x h)
// size 2 is 12 x 16 pixels viz 2 x size 1
// size 3 is 18 x 24 pixels viz 3 x size 1
#define BASE_TEXT_WIDTH_px       6
#define BASE_TEXT_HEIGHT_px      8

#define VERSION_TEXT_SIZE        1
#define HEADING_TEXT_SIZE        2
#define MINOR_SPEED_TEXT_SIZE    2
#define MAIN_SPEED_TEXT_SIZE     5
#define TRAVEL_TIME_TEXT_SIZE    3

// Positions
// These define the center top of each piece of text
// or the edges of heading boxes

// the space between two lines of text or text and borders
#define LINE_SPACE 6

// App name & Version text
#define VERSION_X                     (SCREEN_WIDTH_px/2)
#define VERSION_Y                     (LINE_SPACE)

// Box around "Shutter Speed" heading 
#define SHUTTER_SPEED_HEADING_LEFT    0
#define SHUTTER_SPEED_HEADING_RIGHT   (SCREEN_WIDTH_px)
#define SHUTTER_SPEED_HEADING_TOP     (VERSION_TEXT_SIZE*BASE_TEXT_HEIGHT_px+LINE_SPACE*2)
#define SHUTTER_SPEED_HEADING_BOTTOM  (SHUTTER_SPEED_HEADING_TOP+HEADING_TEXT_SIZE*BASE_TEXT_HEIGHT_px+LINE_SPACE*2)

// "Shutter Speed" heading
#define SHUTTER_SPEED_HEADING_X       ((SHUTTER_SPEED_HEADING_RIGHT-SHUTTER_SPEED_HEADING_LEFT)/2)
#define SHUTTER_SPEED_HEADING_Y       (SHUTTER_SPEED_HEADING_TOP+LINE_SPACE)

// Main shutter speed (centered)
#define MAIN_SPEED_X                  (SCREEN_WIDTH_px/2)
#define MAIN_SPEED_NUMERATOR_Y        (SHUTTER_SPEED_HEADING_BOTTOM + LINE_SPACE)
#define SPEED_FRACTION_LINE_Y         (MAIN_SPEED_NUMERATOR_Y+MAIN_SPEED_TEXT_SIZE*BASE_TEXT_HEIGHT_px+LINE_SPACE/2)
#define SPEED_DENOMINATOR_Y           (SPEED_FRACTION_LINE_Y+LINE_SPACE/2)
#define MAIN_SPEED_FRACTION_LINE_W    (MAIN_SPEED_TEXT_SIZE*BASE_TEXT_WIDTH_px*4)
#define MINOR_SPEED_FRACTION_LINE_W   (MINOR_SPEED_TEXT_SIZE*BASE_TEXT_WIDTH_px*4)

// Minor shutter speed, left & right - aligned to main shutter speed vertically
#define MINOR_SPEED_LEFT_X            (SCREEN_WIDTH_px*3/16)
#define MINOR_SPEED_RIGHT_X           (SCREEN_WIDTH_px*13/16)
#define MINOR_SPEED_NUMERATOR_Y       (SPEED_FRACTION_LINE_Y-LINE_SPACE/2-MINOR_SPEED_TEXT_SIZE*BASE_TEXT_HEIGHT_px)

// "Curtain Travel Time" heading box
#define TRAVEL_TIME_HEADING_LEFT      0
#define TRAVEL_TIME_HEADING_RIGHT     (SCREEN_WIDTH_px)
#define TRAVEL_TIME_HEADING_TOP       (SCREEN_HEIGHT_px-TRAVEL_TIME_TEXT_SIZE*BASE_TEXT_HEIGHT_px-HEADING_TEXT_SIZE*BASE_TEXT_HEIGHT_px-4*LINE_SPACE)
#define TRAVEL_TIME_HEADING_BOTTOM    (TRAVEL_TIME_HEADING_TOP+HEADING_TEXT_SIZE*BASE_TEXT_HEIGHT_px+LINE_SPACE*2)

// The measured times used to calculate shutter speeds
#define SHUTTER_TIME_Y                (TRAVEL_TIME_HEADING_TOP - MINOR_SPEED_TEXT_SIZE * BASE_TEXT_HEIGHT_px - LINE_SPACE)

// "Curtain Travel Time" heading
#define TRAVEL_TIME_HEADING_X         ((TRAVEL_TIME_HEADING_RIGHT-TRAVEL_TIME_HEADING_LEFT)/2)
#define TRAVEL_TIME_HEADING_Y         (TRAVEL_TIME_HEADING_TOP+LINE_SPACE)

// Travel time
#define TRAVEL_TIME_Y                 (TRAVEL_TIME_HEADING_BOTTOM + LINE_SPACE)

// Colours
#define BACKGROUND_COLOUR ILI9341_BLACK
#define TEXT_COLOUR       ILI9341_WHITE
#define HEADING_COLOUR    ILI9341_DARKCYAN
#define BORDER_COLOUR     ILI9341_WHITE

// pins for ILI9341 TFT display connected to hardware SPI
#define TFT_CS    8      // TFT CS  pin is connected to arduino pin 8
#define TFT_RST   9      // TFT RST pin is connected to arduino pin 9
#define TFT_DC    10     // TFT DC  pin is connected to arduino pin 10
#define TFT_MOSI  11     // SPI MOSI 
#define TFT_SCLK  13     // SPI Clock
#define TFT_MISO  12     // SPI MISO - is not used, 

// initialize ILI9341 TFT library
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST); // ILI9341 driver with hardware SPI using the default SPI peripheral
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST, TFT_MISO); // ILI9341 driver with Software SPI

//---------------------------------------------------
// Helper function called once at startup to 
// setup the display (for example draw borders)
//---------------------------------------------------
void tft_setup()
{
  char buffer[35];
  int16_t ulx,uly;
  uint16_t w,h;

  tft.begin();
  tft.fillScreen(BACKGROUND_COLOUR);

  tft.setRotation(1);
    
  // Print small app name and version at the top
  tft.setTextColor(TEXT_COLOUR);  
  tft.setTextSize(VERSION_TEXT_SIZE);
  snprintf(buffer, sizeof(buffer), "Shutter Speed Tester v%i.%i.%i", VERSION_MAJOR, VERSION_MINOR, VERSION_REV);
  tft.getTextBounds(buffer,0,0,&ulx,&uly,&w,&h);
  tft.setCursor(VERSION_X - w/2, VERSION_Y);
  tft.print(buffer);

  // Shutter speed table
  tft.fillRect(SHUTTER_SPEED_HEADING_LEFT, SHUTTER_SPEED_HEADING_TOP, (SHUTTER_SPEED_HEADING_RIGHT-SHUTTER_SPEED_HEADING_LEFT), (SHUTTER_SPEED_HEADING_BOTTOM-SHUTTER_SPEED_HEADING_TOP), HEADING_COLOUR);
  tft.drawRect(SHUTTER_SPEED_HEADING_LEFT, SHUTTER_SPEED_HEADING_TOP, (SHUTTER_SPEED_HEADING_RIGHT-SHUTTER_SPEED_HEADING_LEFT), (SHUTTER_SPEED_HEADING_BOTTOM-SHUTTER_SPEED_HEADING_TOP), BORDER_COLOUR);
  tft.setTextColor(TEXT_COLOUR);  
  tft.setTextSize(HEADING_TEXT_SIZE);
  snprintf(buffer, sizeof(buffer), "Shutter Speed");
  tft.getTextBounds(buffer,0,0,&ulx,&uly,&w,&h);
  tft.setCursor(SHUTTER_SPEED_HEADING_X - w/2, SHUTTER_SPEED_HEADING_Y);
  tft.print(buffer);

  tft.setTextSize(MINOR_SPEED_TEXT_SIZE);
  tft.setCursor(MINOR_SPEED_LEFT_X - MINOR_SPEED_TEXT_SIZE*BASE_TEXT_WIDTH_px/2, MINOR_SPEED_NUMERATOR_Y);
  tft.print("1");
  tft.drawFastHLine(MINOR_SPEED_LEFT_X - MINOR_SPEED_FRACTION_LINE_W/2, SPEED_FRACTION_LINE_Y, MINOR_SPEED_FRACTION_LINE_W, TEXT_COLOUR);
  tft.setCursor(MINOR_SPEED_LEFT_X - MINOR_SPEED_TEXT_SIZE*BASE_TEXT_WIDTH_px*2, SPEED_DENOMINATOR_Y);
  tft.print("XXXX");

  tft.setCursor(MINOR_SPEED_RIGHT_X - MINOR_SPEED_TEXT_SIZE*BASE_TEXT_WIDTH_px/2, MINOR_SPEED_NUMERATOR_Y);
  tft.print("1");
  tft.drawFastHLine(MINOR_SPEED_RIGHT_X - MINOR_SPEED_FRACTION_LINE_W/2, SPEED_FRACTION_LINE_Y, MINOR_SPEED_FRACTION_LINE_W, TEXT_COLOUR);
  tft.setCursor(MINOR_SPEED_RIGHT_X - MINOR_SPEED_TEXT_SIZE*BASE_TEXT_WIDTH_px*2, SPEED_DENOMINATOR_Y);
  tft.print("XXXX");

  tft.setTextSize(MAIN_SPEED_TEXT_SIZE);
  tft.setCursor(MAIN_SPEED_X - MAIN_SPEED_TEXT_SIZE*BASE_TEXT_WIDTH_px/2, MAIN_SPEED_NUMERATOR_Y);
  tft.print("1");
  tft.drawFastHLine(MAIN_SPEED_X-MAIN_SPEED_FRACTION_LINE_W/2, SPEED_FRACTION_LINE_Y, MAIN_SPEED_FRACTION_LINE_W, TEXT_COLOUR);
  tft.setCursor(MAIN_SPEED_X - MAIN_SPEED_TEXT_SIZE*BASE_TEXT_WIDTH_px*2, SPEED_DENOMINATOR_Y);
  tft.print("XXXX");

  // curtain travel time table
  tft.fillRect(TRAVEL_TIME_HEADING_LEFT, TRAVEL_TIME_HEADING_TOP, (TRAVEL_TIME_HEADING_RIGHT-TRAVEL_TIME_HEADING_LEFT), (TRAVEL_TIME_HEADING_BOTTOM-TRAVEL_TIME_HEADING_TOP), HEADING_COLOUR);
  tft.drawRect(TRAVEL_TIME_HEADING_LEFT, TRAVEL_TIME_HEADING_TOP, (TRAVEL_TIME_HEADING_RIGHT-TRAVEL_TIME_HEADING_LEFT), (TRAVEL_TIME_HEADING_BOTTOM-TRAVEL_TIME_HEADING_TOP), BORDER_COLOUR);
  tft.setTextColor(TEXT_COLOUR);  
  tft.setTextSize(HEADING_TEXT_SIZE);
  snprintf(buffer, sizeof(buffer), "Curtain Travel Time (ms)");
  tft.getTextBounds(buffer,0,0,&ulx,&uly,&w,&h);
  tft.setCursor(TRAVEL_TIME_HEADING_X - w/2, TRAVEL_TIME_HEADING_Y);
  tft.print(buffer);
  
  // outside border around edge of screen printed last so it is on top
  tft.drawRect(0, 0, SCREEN_WIDTH_px, SCREEN_HEIGHT_px, BORDER_COLOUR);
  
}

//---------------------------------------------------
// Print values to the screen.
//---------------------------------------------------
void tft_show_values( double shutter_speed_1_ms, 
                      double shutter_speed_2_ms,
                      double shutter_speed_3_ms,
                      double fractional_shutter_speed_1, 
                      double fractional_shutter_speed_2,
                      double fractional_shutter_speed_3,
                      double curtain_1_travel_time_ms,
                      double curtain_2_travel_time_ms )
{
  char buffer1[15];
  char buffer2[15];
  int16_t ulx,uly;
  uint16_t w1, h, w2;

  // Clear previous shutter speeds and times
  tft.fillRect(1, SPEED_DENOMINATOR_Y, SCREEN_WIDTH_px-2, TRAVEL_TIME_HEADING_TOP-SPEED_DENOMINATOR_Y-2, BACKGROUND_COLOUR);
  
  // Fractional Shutter Speeds
  tft.setTextColor(TEXT_COLOUR, BACKGROUND_COLOUR);
  tft.setTextSize(MAIN_SPEED_TEXT_SIZE);
  snprintf(buffer1, sizeof(buffer1), "%0.0f", fractional_shutter_speed_2);
  tft.getTextBounds(buffer1,0,0,&ulx,&uly,&w1,&h);
  tft.setCursor(MAIN_SPEED_X-w1/2, SPEED_DENOMINATOR_Y);
  tft.print(buffer1);

  tft.setTextSize(MINOR_SPEED_TEXT_SIZE);
  snprintf(buffer1, sizeof(buffer1), "%0.0f", fractional_shutter_speed_1);
  tft.getTextBounds(buffer1,0,0,&ulx,&uly,&w1,&h);
  tft.setCursor(MINOR_SPEED_LEFT_X-w1/2, SPEED_DENOMINATOR_Y);
  tft.print(buffer1);

  snprintf(buffer1, sizeof(buffer1), "%0.0f", fractional_shutter_speed_3);
  tft.getTextBounds(buffer1,0,0,&ulx,&uly,&w1,&h);
  tft.setCursor(MINOR_SPEED_RIGHT_X-w1/2, SPEED_DENOMINATOR_Y);
  tft.print(buffer1);

  // Measured times from which Fractional Shutter Speeds were calculated
  tft.setTextSize(MINOR_SPEED_TEXT_SIZE);
  snprintf(buffer1, sizeof(buffer1), "%0.1fms", shutter_speed_1_ms);
  tft.getTextBounds(buffer1,0,0,&ulx,&uly,&w1,&h);
  tft.setCursor(MINOR_SPEED_LEFT_X-w1/2, SHUTTER_TIME_Y);
  tft.print(buffer1);

  snprintf(buffer1, sizeof(buffer1), "%0.1fms", shutter_speed_2_ms);
  tft.getTextBounds(buffer1,0,0,&ulx,&uly,&w1,&h);
  tft.setCursor(MAIN_SPEED_X-w1/2, SHUTTER_TIME_Y);
  tft.print(buffer1);

  snprintf(buffer1, sizeof(buffer1), "%0.1fms", shutter_speed_3_ms);
  tft.getTextBounds(buffer1,0,0,&ulx,&uly,&w1,&h);
  tft.setCursor(MINOR_SPEED_RIGHT_X-w1/2, SHUTTER_TIME_Y);
  tft.print(buffer1);
  
  // Clear previous curtain travel time values
  tft.fillRect(1, TRAVEL_TIME_HEADING_BOTTOM+1, SCREEN_WIDTH_px-2, (SCREEN_HEIGHT_px-TRAVEL_TIME_HEADING_BOTTOM-2), BACKGROUND_COLOUR);
  // Curtain travel time values
  tft.setTextSize(TRAVEL_TIME_TEXT_SIZE);
  snprintf(buffer1, sizeof(buffer1), "%0.1f", curtain_1_travel_time_ms);
  tft.getTextBounds(buffer1,0,0,&ulx,&uly,&w1,&h);
  snprintf(buffer2, sizeof(buffer2), "%0.1f", curtain_2_travel_time_ms);
  tft.getTextBounds(buffer2,0,0,&ulx,&uly,&w2,&h);
  uint16_t gap = (SCREEN_WIDTH_px - w1 - w2)/3;
  tft.setCursor(gap, TRAVEL_TIME_Y);
  tft.print(buffer1);
  tft.setCursor(2*gap+w1, TRAVEL_TIME_Y);
  tft.print(buffer2);
}

void tft_colour_demo()
{
  const int TEXT_SIZE = 2;
  tft.setTextSize(TEXT_SIZE);
  
  tft.setCursor(0, 0 * BASE_TEXT_HEIGHT_px * TEXT_SIZE); 
  tft.setTextColor(ILI9341_NAVY); tft.print("Navy");

  tft.setCursor(0, 1 * BASE_TEXT_HEIGHT_px * TEXT_SIZE ); 
  tft.setTextColor(ILI9341_DARKGREEN); tft.print("Dark Green");
  
  tft.setCursor(0, 2 * BASE_TEXT_HEIGHT_px * TEXT_SIZE); 
  tft.setTextColor(ILI9341_DARKCYAN); tft.print("Dark Cyan");
  
  tft.setCursor(0, 3 * BASE_TEXT_HEIGHT_px * TEXT_SIZE);
  tft.setTextColor(ILI9341_MAROON); tft.print("Maroon");
  
  tft.setCursor(0, 4 * BASE_TEXT_HEIGHT_px * TEXT_SIZE);
  tft.setTextColor(ILI9341_PURPLE); tft.print("Purple");
  
  tft.setCursor(0, 5 * BASE_TEXT_HEIGHT_px * TEXT_SIZE);
  tft.setTextColor(ILI9341_OLIVE); tft.print("Olive");
  
  tft.setCursor(0, 6 * BASE_TEXT_HEIGHT_px * TEXT_SIZE);
  tft.setTextColor(ILI9341_LIGHTGREY); tft.print("Light Grey");
  
  tft.setCursor(0, 7 * BASE_TEXT_HEIGHT_px * TEXT_SIZE);
  tft.setTextColor(ILI9341_DARKGREY); tft.print("Dark Grey");
  
  tft.setCursor(0, 8 * BASE_TEXT_HEIGHT_px * TEXT_SIZE);
  tft.setTextColor(ILI9341_RED); tft.print("Red");
  
  tft.setCursor(0, 9 * BASE_TEXT_HEIGHT_px * TEXT_SIZE);
  tft.setTextColor(ILI9341_MAGENTA); tft.print("Magenta");
  
  tft.setCursor(0, 10 * BASE_TEXT_HEIGHT_px * TEXT_SIZE);
  tft.setTextColor(ILI9341_YELLOW); tft.print("Yellow");
  
  tft.setCursor(0, 11 * BASE_TEXT_HEIGHT_px * TEXT_SIZE);
  tft.setTextColor(ILI9341_WHITE); tft.print("White");
  
  tft.setTextColor(ILI9341_ORANGE);
  tft.setCursor(0, 12 * BASE_TEXT_HEIGHT_px * TEXT_SIZE); tft.print("Orange");
  
  tft.setCursor(0, 13 * BASE_TEXT_HEIGHT_px * TEXT_SIZE);
  tft.setTextColor(ILI9341_GREENYELLOW); tft.print("Green Yellow");
  
  tft.setCursor(0, 14 * BASE_TEXT_HEIGHT_px * TEXT_SIZE);
  tft.setTextColor(ILI9341_PINK); tft.print("Pink");
}
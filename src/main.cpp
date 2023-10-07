#include <Arduino.h>
#include <PinChangeInterrupt.h>

// When turned on all the timestamps and the results of
// the calculations are printed to the serial console
#define DEBUG 0

// choose which screen to use
// 0.96" OLED - connected via I2C
// 2.2"  TFT - connected via SPI
#define USE_OLED 0
#define USE_TFT 1

#if USE_OLED
#include "oled.h"
#endif

#if USE_TFT
#include "tft.h"
#endif

// pins for KY-008 Laser Diodes
#define LASER_DIODE_1_OUTPUT 5
#define LASER_DIODE_2_OUTPUT 6
#define LASER_DIODE_3_OUTPUT 7

// pins for ISO203 Laser Receivers
#define LASER_RECEIVER_1_INPUT 2
#define LASER_RECEIVER_2_INPUT 3
#define LASER_RECEIVER_3_INPUT 4

// variables set in interrupts
volatile uint32_t ts_1_start_us = 0;
volatile uint32_t ts_1_end_us = 0;
volatile bool speed_1_measured = false;
volatile bool curtain_1_measured = false;

volatile uint32_t ts_2_start_us = 0;
volatile uint32_t ts_2_end_us = 0;
volatile bool speed_2_measured = false;

volatile uint32_t ts_3_start_us = 0;
volatile uint32_t ts_3_end_us = 0;
volatile bool speed_3_measured = false;
volatile bool curtain_2_measured = false;

// variables for calculated values
double shutter_speed_1_ms = 0.0;
double shutter_speed_2_ms = 0.0;
double shutter_speed_3_ms = 0.0;
double fractional_shutter_speed_1 = 0.0;
double fractional_shutter_speed_2 = 0.0;
double fractional_shutter_speed_3 = 0.0;
double curtain_1_travel_time_ms = 0.0;
double curtain_2_travel_time_ms = 0.0;

// display update delay
#define DISPLAY_UPDATE_TRIGGER (UINT16_MAX / 2)
uint32_t display_update_counter = DISPLAY_UPDATE_TRIGGER;

//---------------------------------------------------
// Detector 1 interrupt handler
//---------------------------------------------------
void detector1(void)
{
  if (digitalRead(LASER_RECEIVER_1_INPUT))
  {
    ts_1_end_us = micros();
    speed_1_measured = true;
    curtain_1_measured = true;
  }
  else
  {
    ts_1_start_us = micros();
    speed_1_measured = false;
    curtain_1_measured = false;
  }
}

//---------------------------------------------------
// Detector 2 interrupt handler
//---------------------------------------------------
void detector2(void)
{
  if (digitalRead(LASER_RECEIVER_2_INPUT))
  {
    ts_2_end_us = micros();
    speed_2_measured = true;
  }
  else
  {
    ts_2_start_us = micros();
    speed_2_measured = false;
  }
}

//---------------------------------------------------
// Detector 3 interrupt handler
//---------------------------------------------------
void detector3(void)
{
  if (digitalRead(LASER_RECEIVER_3_INPUT))
  {
    ts_3_end_us = micros();
    speed_3_measured = true;
    curtain_2_measured = true;
  }
  else
  {
    ts_3_start_us = micros();
    speed_3_measured = false;
    curtain_2_measured = false;
  }
}

//---------------------------------------------------
// Setup function, called once at start
//---------------------------------------------------
void setup()
{
#if DEBUG
  Serial.begin(115200);
  Serial.flush();
#endif

  pinMode(LASER_RECEIVER_1_INPUT, INPUT);
  pinMode(LASER_RECEIVER_2_INPUT, INPUT);
  pinMode(LASER_RECEIVER_3_INPUT, INPUT);

  pinMode(LASER_DIODE_1_OUTPUT, OUTPUT);
  pinMode(LASER_DIODE_2_OUTPUT, OUTPUT);
  pinMode(LASER_DIODE_3_OUTPUT, OUTPUT);

#if USE_OLED
#if DEBUG
  Serial.println("Setup OLED");
#endif
  oled_setup();
#endif

#if USE_TFT
#if DEBUG
  Serial.println("Setup TFT");
#endif
  tft_setup();
#endif

// setup interrupts for the laser receivers
#if DEBUG
  Serial.println("Installing interrupt handlers");
#endif
  attachPCINT(digitalPinToPCINT(LASER_RECEIVER_1_INPUT), detector1, CHANGE);
  attachPCINT(digitalPinToPCINT(LASER_RECEIVER_2_INPUT), detector2, CHANGE);
  attachPCINT(digitalPinToPCINT(LASER_RECEIVER_3_INPUT), detector3, CHANGE);

#if DEBUG
  Serial.println("Ready... turning on lasers!");
#endif
  digitalWrite(LASER_DIODE_1_OUTPUT, HIGH);
  digitalWrite(LASER_DIODE_2_OUTPUT, HIGH);
  digitalWrite(LASER_DIODE_3_OUTPUT, HIGH);
}

//---------------------------------------------------
// Loop function, called repeatedly while running
//---------------------------------------------------
void loop()
{
  // --------- calculate ---------

  // Calculate shutter speeds independently so that a single sensor can
  // be used to measure only shutter speed or all 3 to measure shutter
  // speed and curtain travel with out needing a mode change switch
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
  //                    .              .       .       .
  //                    .              .       .       .
  //                    |<-------------------->|       .
  //                    |       t_2_delta      |       .
  //              ts_2_start           .   ts_2_end    .
  //                                   .               .
  //                                   .               .
  //                                   |<------------->|
  //                                   |   t_3_delta   |
  //                                ts_1_end        ts_3_end
  //
  //   Curtain 1 travel time = t_1_delta
  //   Curtain 2 travel time = t_3_delta
  //   Shutter speed = t_2_delta
  //
  if (speed_1_measured)
  {
    speed_1_measured = false;
    display_update_counter = 0;
    double shutter_speed_us = (double)(ts_1_end_us - ts_1_start_us);
    shutter_speed_1_ms = shutter_speed_us / 1000.0;
    fractional_shutter_speed_1 = 1000000.0 / shutter_speed_us;
#if DEBUG
    Serial.print("t1_s=");
    Serial.print(ts_1_start_us);
    Serial.print("us   t1_e=");
    Serial.print(ts_1_end_us);
    Serial.println("us");
    Serial.print("Speed 1 = ");
    Serial.print(shutter_speed_1_ms, 1);
    Serial.println(" ms");
    Serial.print("Fractional Speed 1 = 1/");
    Serial.println(fractional_shutter_speed_1, 1);
#endif
  }
  if (speed_2_measured)
  {
    speed_2_measured = false;
    display_update_counter = 0;
    double shutter_speed_us = (double)(ts_2_end_us - ts_2_start_us);
    shutter_speed_2_ms = shutter_speed_us / 1000.0;
    fractional_shutter_speed_2 = 1000000.0 / shutter_speed_us;
#if DEBUG
    Serial.print("t2_s=");
    Serial.print(ts_2_start_us);
    Serial.print("us   t2_e=");
    Serial.print(ts_2_end_us);
    Serial.println("us");
    Serial.print("Speed 2 = ");
    Serial.print(shutter_speed_2_ms, 1);
    Serial.println(" ms");
    Serial.print("Fractional Speed 2 = 1/");
    Serial.println(fractional_shutter_speed_2, 1);
    Serial.println("");
#endif
  }
  if (speed_3_measured)
  {
    speed_3_measured = false;
    display_update_counter = 0;
    double shutter_speed_us = (double)(ts_3_end_us - ts_3_start_us);
    shutter_speed_3_ms = shutter_speed_us / 1000.0;
    fractional_shutter_speed_3 = 1000000.0 / shutter_speed_us;
#if DEBUG
    Serial.print("t3_s=");
    Serial.print(ts_3_start_us);
    Serial.print("us   t3_e=");
    Serial.print(ts_3_end_us);
    Serial.println("us");
    Serial.print("Speed 3 = ");
    Serial.print(shutter_speed_3_ms, 1);
    Serial.println(" ms");
    Serial.print("Fractional Speed 3 = 1/");
    Serial.println(fractional_shutter_speed_3, 1);
    Serial.println("");
#endif
  }
  if (curtain_1_measured && curtain_2_measured)
  {
    curtain_1_measured = false;
    curtain_2_measured = false;
    display_update_counter = 0;
    // check which is bigger and calculate accordingly
    // so that travel direction does not matter
    if (ts_3_start_us > ts_1_start_us)
    {
      curtain_1_travel_time_ms = (double)(ts_3_start_us - ts_1_start_us) / 1000.0;
    }
    else
    {
      curtain_1_travel_time_ms = (double)(ts_1_start_us - ts_3_start_us) / 1000.0;
    }
    if (ts_3_end_us > ts_1_end_us)
    {
      curtain_2_travel_time_ms = (double)(ts_3_end_us - ts_1_end_us) / 1000.0;
    }
    else
    {
      curtain_2_travel_time_ms = (double)(ts_1_end_us - ts_3_end_us) / 1000.0;
    }

#if DEBUG
    Serial.print("Curtain 1 travel time=");
    Serial.print(curtain_1_travel_time_ms, 1);
    Serial.println(" ms");

    Serial.print("Curtain 2 travel time=");
    Serial.print(curtain_2_travel_time_ms, 1);
    Serial.println(" ms");

    Serial.println("");
#endif
  }

  // --------- display ---------
  if (display_update_counter == DISPLAY_UPDATE_TRIGGER)
  {
#if USE_OLED
    oled_show_values(shutter_speed_1_ms,
                     shutter_speed_2_ms,
                     shutter_speed_3_ms,
                     fractional_shutter_speed_1,
                     fractional_shutter_speed_2,
                     fractional_shutter_speed_3,
                     curtain_1_travel_time_ms,
                     curtain_2_travel_time_ms);
#endif

#if USE_TFT
    tft_show_values(shutter_speed_1_ms,
                    shutter_speed_2_ms,
                    shutter_speed_3_ms,
                    fractional_shutter_speed_1,
                    fractional_shutter_speed_2,
                    fractional_shutter_speed_3,
                    curtain_1_travel_time_ms,
                    curtain_2_travel_time_ms);
#endif
  }
  if (display_update_counter <= DISPLAY_UPDATE_TRIGGER)
  {
    display_update_counter++;
  }
}

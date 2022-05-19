#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Text Size 3 = 18x24 pixels
// Text Size 2 = 12x16 pixels
// Text Size 1 = 6x8 pixels
#define TEXT_SIZE_3_WIDTH  18
#define TEXT_SIZE_3_HEIGHT 24
#define TEXT_SIZE_2_WIDTH  12
#define TEXT_SIZE_2_HEIGHT 16
#define TEXT_SIZE_1_WIDTH  6
#define TEXT_SIZE_1_HEIGHT 8

#define SSD1306_NO_SPLASH 0

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LASER_1_OUTPUT 5
#define LASER_2_OUTPUT 6
#define LASER_3_OUTPUT 7

#define DETECTOR_1_INPUT 2
#define DETECTOR_2_INPUT 3
#define DETECTOR_3_INPUT 4

#define DARK  1
#define LIGHT !DARK

unsigned long ts_1_start_us = 0;
unsigned long ts_2_start_us = 0;
unsigned long ts_3_start_us = 0;
unsigned long ts_1_end_us = 0;
unsigned long ts_2_end_us = 0;
unsigned long ts_3_end_us = 0;

int sensor_1_last_read;
int sensor_2_last_read;
int sensor_3_last_read;

bool sensor_1_triggered = false;
bool sensor_2_triggered = false;
bool sensor_3_triggered = false;

double shutter_speed_us = 0.0;
double fractional_shutter_speed = 0.0;
double curtain_1_travel_time_us = 0.0;
double curtain_2_travel_time_us = 0.0;

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
    for (;;) {};  // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);
  display.setTextWrap(false);
  display.setTextSize(3);
  const char* msg = "Ready";
  uint16_t w = strlen(msg) * TEXT_SIZE_3_WIDTH;
  display.setCursor((SCREEN_WIDTH - w)/2, (SCREEN_HEIGHT - TEXT_SIZE_3_HEIGHT) / 2);
  display.print(msg);
  display.display();

  Serial.println("Ready... turning on lasers!");
  digitalWrite(LASER_1_OUTPUT, HIGH);
  digitalWrite(LASER_2_OUTPUT, HIGH);
  digitalWrite(LASER_3_OUTPUT, HIGH);

  sensor_1_last_read = digitalRead(DETECTOR_1_INPUT);
  sensor_2_last_read = digitalRead(DETECTOR_2_INPUT);
  sensor_3_last_read = digitalRead(DETECTOR_3_INPUT);

  unsigned long ts = micros();
  ts_1_start_us = ts;
  ts_2_start_us = ts;
  ts_3_start_us = ts;
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
void loop() 
{
  unsigned long read_ts = micros();
  int sensor_1_read = digitalRead(DETECTOR_1_INPUT);
  int sensor_2_read = digitalRead(DETECTOR_2_INPUT);
  int sensor_3_read = digitalRead(DETECTOR_3_INPUT);

  // ----------- Sensor 1 ------------
  if (sensor_1_last_read == DARK && sensor_1_read == LIGHT)
  {
    ts_1_start_us = read_ts;
  }
  else if (sensor_1_last_read == LIGHT && sensor_1_read == DARK)
  {
    ts_1_end_us = read_ts;
    sensor_1_triggered = true;
    Serial.println("Sensor 1 Triggered");
  } 
  if (sensor_1_read != sensor_1_last_read)
  {
     sensor_1_last_read = sensor_1_read;
  }

  // ----------- Sensor 2 ------------
  if (sensor_2_last_read == DARK && sensor_2_read == LIGHT)
  {
    ts_2_start_us = read_ts;
  }
  else if (sensor_2_last_read == LIGHT && sensor_2_read == DARK)
  {
    ts_2_end_us = read_ts;
    sensor_2_triggered = true;
    Serial.println("Sensor 2 Triggered");
  } 
  if (sensor_2_read != sensor_2_last_read)
  {
     sensor_2_last_read = sensor_2_read;
  }

  // ----------- Sensor 3 ------------
  if (sensor_3_last_read == DARK && sensor_3_read == LIGHT)
  {
    ts_3_start_us = read_ts;
  }
  else if (sensor_3_last_read == LIGHT && sensor_3_read == DARK)
  {
    ts_3_end_us = read_ts;
    sensor_3_triggered = true;
    Serial.println("Sensor 3 Triggered");
  } 
  if (sensor_3_read != sensor_3_last_read)
  {
     sensor_3_last_read = sensor_3_read;
  }

  // --------- display ---------
  bool display_refresh = false;

  // If only sensor 2 is triggered then we are only testing shutter
  // speed and not curtain travel times
  if (sensor_2_triggered)
  {
    sensor_2_triggered = false;
    display_refresh = true;
    shutter_speed_us = (double)(ts_2_end_us - ts_2_start_us);
    fractional_shutter_speed = 1000000.0 / shutter_speed_us;
    Serial.print("Shutter Speed = ");Serial.print(shutter_speed_us / 1000.0, 1); Serial.println(" ms");
    Serial.print("Fractional Shutter Speed = 1/");Serial.println(fractional_shutter_speed, 1);
  }
  if (sensor_1_triggered && sensor_3_triggered)
  {
    sensor_1_triggered = false;
    sensor_3_triggered = false;
    display_refresh = true;
    curtain_1_travel_time_us = (double)(ts_3_start_us - ts_1_start_us);
    curtain_2_travel_time_us = (double)(ts_3_end_us - ts_1_end_us);
    Serial.print("Curtain 1 travel time=");Serial.print(curtain_1_travel_time_us/1000);Serial.println(" ms");
    Serial.print("Curtain 2 travel time=");Serial.print(curtain_2_travel_time_us/1000);Serial.println(" ms");
  }
  if (display_refresh)
  {
    display.clearDisplay();

    display.setTextSize(2);

    display.setCursor(0, 0);
    display.print(shutter_speed_us / 1000.0, 1);
    display.print("ms");

    display.setCursor(0, TEXT_SIZE_2_HEIGHT);
    display.print("1/");
    display.print(fractional_shutter_speed, 1);

    display.setTextSize(1);

    display.setCursor(0, SCREEN_HEIGHT - TEXT_SIZE_1_HEIGHT - 1);
    display.print("c1:");
    display.print(curtain_1_travel_time_us / 1000.0, 1);
    display.print("ms");

    display.setCursor(SCREEN_WIDTH/2, SCREEN_HEIGHT - TEXT_SIZE_1_HEIGHT - 1);
    display.print("c2:");
    display.print(curtain_2_travel_time_us / 1000.0, 1);
    display.print("ms");

    display.display();

    Serial.println("---");
  }
}

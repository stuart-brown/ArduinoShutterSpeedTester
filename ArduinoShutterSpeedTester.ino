#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <Fonts/FreeMono12pt7b.h>

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

#define LASER_1_OUTPUT 5
#define LASER_2_OUTPUT 6
#define LASER_3_OUTPUT 7

#define DETECTOR_1_INPUT 2
#define DETECTOR_2_INPUT 3
#define DETECTOR_3_INPUT 4

#define DARK  1
#define LIGHT !DARK

int second_line_y_pos = 0;

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
  //display.setFont(&FreeMono12pt7b);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);
  display.setTextWrap(false);
  display.setTextSize(3);
  const char* msg = "Ready";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((display.width() - w)/2, (display.height() - h) / 2);
  display.print(msg);
  display.display();
  Serial.println("Ready... turning on lasers!");
  digitalWrite(LASER_2_OUTPUT, HIGH);

  sensor_2_last_read = digitalRead(DETECTOR_2_INPUT);
  unsigned long ts = micros();
  ts_2_start_us = ts;
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
  int sensor_2_read = digitalRead(DETECTOR_2_INPUT);
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
  // --------- display ---------
  if (sensor_2_triggered)
  {
    sensor_2_triggered = false;
    double shutter_speed_s2_us = (double)(ts_2_end_us - ts_2_start_us);
    double fractional_shutter_speed_s2 = 1000000.0 / shutter_speed_s2_us;
    Serial.print("Shutter Speed S2 = ");Serial.print(shutter_speed_s2_us / 1000.0, 1); Serial.println(" ms");
    Serial.print("Shutter Speed S2 = 1/");Serial.println(fractional_shutter_speed_s2, 1);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(shutter_speed_s2_us / 1000.0, 1);
    display.print("ms");

    display.setCursor(0, display.height()/2);
    display.print("1/");
    display.print(fractional_shutter_speed_s2, 1);
    display.display();

    Serial.println("---");
  }
}

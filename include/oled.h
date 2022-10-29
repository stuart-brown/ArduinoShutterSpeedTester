#ifndef OLED_H
#define OLED_H

void oled_setup();
void oled_show_values( double shutter_speed_1_ms, 
                       double shutter_speed_2_ms,
                       double shutter_speed_3_ms,
                       double fractional_shutter_speed_1, 
                       double fractional_shutter_speed_2,
                       double fractional_shutter_speed_3,
                       double curtain_1_travel_time_ms,
                       double curtain_2_travel_time_ms );
#endif /* OLED_H */
# Arduino Shutter Speed Tester

A project to build a shutter speed tester using KY-008 laser diodes, TLS257 Light-to-voltage converter, OLED  display, all driven by an Arduino Nano.

## Status
This is a work in progress.

### What works:
* Single measurement at center of shutter curtain.
* Display of curtain open time and corresponding shutter speed as a fraction
* Calibration check

### To do:
* 2 more sensors for curtain travel times
* Housing
* Display optimisation
* Ability to measure cameras like Barnack Leicas where you cannot access the rear of the shutter curtain.
* Project documentation. Add references, datasheets and circuit diagrams. Add images to this readme

# Calibration
Calibration was done by building an STM32 project to drive a KY-008 laser diode,  producing a 100ms on pulse once per second. The on pulse was measured using an oscilloscope hooked up to the laser diode,pulse width turned out to be 99.8ms.

The STM32 calibration device was then use to illuminate the TLS257 receiver on the shutter speed tester to check that the tester displayed the correct time.

The rise and fall time of the TLS257 where also checked using the oscilloscope.
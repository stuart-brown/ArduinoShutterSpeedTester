# Arduino Shutter Speed Tester


A project to build a shutter speed tester using KY-008 laser diodes, TLS257 Light-to-voltage converter, OLED  display, all driven by an Arduino Nano.

<br>
<br>

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

<br>
<br>

# Calibration
Calibration was done by building an STM32 project to drive a KY-008 laser diode,  producing a 100ms on pulse once per second. The on pulse was measured using an oscilloscope hooked up to the laser diode,pulse width turned out to be 99.8ms.

The STM32 calibration device was then use to illuminate the TLS257 receiver on the shutter speed tester to check that the tester displayed the correct time.

The rise and fall time of the TLS257 where also checked using the oscilloscope.

<br>
<br>

# References and Interesting Reading:
* [ISO 516:2019 Camera shutters — Timing — General definition and mechanical shutter measurements](https://www.iso.org/standard/70966.html) - This is __*the standard*__ for testing shutter speeds. You need to buy the standard, but there are various previews available online, enough to glean useful information from.

* [The Way to Modern Shutter Speed Measurement Methods A, Historical Overview, by Gyula Simon, Gergely Vakulya, and Márk Rátosi](https://www.mdpi.com/1424-8220/22/5/1871) - A paper that delves into various methods to measure shutter speeds.

* [National Camera, Technician Course, Testing Shutter Speeds](https://learncamerarepair.com/downloads/pdf/NatCam-Shutter-Test-Guide.pdf)

* [Charts of Nominal, and Precise Actual camera
Shutter speed, f stop and ISO goal values](https://www.scantips.com/lights/math.html) &  [Camera Math of photography settings and EV](https://www.scantips.com/lights/fstop2.html) - These pages contain an enlightening discussion about 'Nominal' vs 'Accurate' values and the maths behind cameras.


<br>
<br>

# Some other shutter speed testers & projects
* http://forum.mflenses.com/the-shutter-speed-meter-project-t11387.html

* https://www.pentaxforums.com/forums/8-film-slrs-compact-film-cameras/409300-optical-measurements-pentax-me-super-shutter-speed.html

* https://www.pentaxforums.com/forums/173-general-photography/359823-true-shutter-speeds-compared-nominal-shutter-speeds-actual-measurements.html

* [Laser Shutter Tester](https://community.element14.com/challenges-projects/project14/makingtime/b/blog/posts/laser-shutter-tester)

* [VFMOTO camera shutter tester](https://www.ebay.com/itm/154737256704) - A commercial product sold on EBay

* [Film Camera Tester](https://github.com/srozum/film_camera_tester/wiki) - This one is available as a DIY kit on or assembled from [Tindie](https://www.tindie.com/products/srozum/film-camera-tester/)

* [Speed tester by cameradactyl](https://github.com/cameradactyl/Shutter-Timer) A very similar project to mine :)

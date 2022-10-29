# Arduino Shutter Speed Tester

A project to build a shutter speed tester using KY-008 laser diodes, ISO203 Laser Receiver & TFT display, all driven by an Arduino Nano.
This project is setup for VSCode with Platform IO. See https://dronebotworkshop.com/platformio/

<br>

## Status
This is a work in progress.


### What works:
* SMeasurement of shutter speeds & curtain travel times.
* Display shutter speeds as fractions and corresponding time measurements
* Display of curtain travel times
* Calibration check


### To do:
* Ability to measure cameras like Barnack Leicas where you cannot access the rear of the shutter curtain.
* Finish project documentation: references, datasheets, circuit diagrams, etc. 
* Add images to this readme

<br>
<br>

# Calibration
Calibration was done using a calibration device consisting of a STM32 Nucleo-F303RE development board driving a KY-008 laser diode. This produced light pulses of varying widths, the width of the pulse can be changed by pressing the 'user' button on the dev board. The width of the ON pulses was measured using a digital storage oscilloscope to confirm their accuracy.

The STM32 calibration device was then use to illuminate the laser receiver on the shutter speed tester to check that the tester measured and displayed the correct time.

The rise and fall times of the laser detector were also checked using the oscilloscope.

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

* [Speed tester](https://github.com/cameradactyl/Shutter-Timer) by cameradactyl - A project very similar to this one :)

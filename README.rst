Arduino Examples
================

A collection of code examples for Arduino boards and other similar devices
(ESP8266, Teensy, etc.) collected from my various experiments.


esp8266-temperature
-------------------

`This set of sketches <esp8266-temperature/>`_ demonstrates measuring temperature
from an ESP8266 development board with a thermistor, then submitting that value
periodically over the network to an InfluxDB database so it can be analysed and
graphed.


wifi-button
-----------

`This sketch <wifi-button/>`_ is the source code that powers my WiFi button, a
device that uses an ESP32 chip in deep sleep mode to control my lights while
preserving battery power.


gps-tracker
-----------

`This sketch <gps-tracker/>`_ is a Particle-compatible sketch that uses a GPS
breakout module to track a car as it moves.



License
-------

All code in this repository is under the Apache 2 license.

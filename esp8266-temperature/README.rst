ESP8266 Temperature Sensing
===========================

A pair of Arduino sketches that illustrate using an ESP8266 development board
along with a thermistor to build a wireless temperature sensor.

The ``basic`` sketch just measures the temperature and logs it to the serial
console so you can calibrate the thermistor accurately.

The ``wifi`` sketch extends this to submit the data over WiFi to an InfluxDB
instance, which you can then query using Grafana. If you have InfluxDB exposed
so that you can talk to it using standard authentication, you can simply set
the constants at the top of the file and you're ready to go.

If you have a non-ESP8266 board, you can use the ``basic`` version by just changing
the analog pin, but you'll need to swap out the WiFi library in the ``wifi``
version for one supported by your platform/shield.

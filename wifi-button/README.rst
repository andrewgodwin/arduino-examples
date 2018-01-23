WiFi Button
===========

An ESP32-specific Arduino sketch that uses the chip's deep sleep mode to sleep
until a button is pressed, at which point it wakes up, sends a HTTP request
over WiFi (in my case, to my WeMo lightswitches), and then sleeps again, to
preserve battery power.

The code in here that does the HTTP request is specific to WeMo lightswitches
and their XML-RPC API; you can swap it for any other network request you'd like
(I'll be swapping one button to talk to a future IR emitter, likely via an
IFTTT webhook, in future).

You could likely try and do some variant of this with the ESP8266, but you'd
have to make the buttons trigger the reset circuit instead, and it can only
sleep for a maximum of around 70 minutes, so I opted for an ESP32.

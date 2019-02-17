GPS Tracker
===========

This is the source code for my `GPS Tracker project <https://www.youtube.com/watch?v=hIQv9oxIj08>`_.

It's not pure Arduino, but instead designed for the Particle IDE; you'll need to either upload it to their online IDE or use the downloadable one to make it work.

A basic wiring diagram is also included, to show how to link the modules together, complete with an optional extension of the Fix LED if you need it.

The code here uses the Particle event stream to save having to do HTTP itself; instead, there's a webhook configured in the integration that sends the data to a `Traccar <http://www.traccar.org>`_ server I run. Details below.

Traccar Webhook
---------------

To configure the webhook for Traccar, set it up like this (it uses the OSMAnd endpoint on Traccar):

* Target URL: ``http://your-traccar-server.com:5055``
* Request type: ``GET``
* Request format: Query parameters
* Query parameters::

    {
      "id": "{{{PARTICLE_DEVICE_ID}}}",
      "lat": "{{lat}}",
      "lon": "{{lng}}",
      "timestamp": "{{time}}",
      "hdop": "{{hdop}}",
      "altitude": "{{alt}}",
      "speed": "{{spd}}",
      "course": "{{crs}}"
    }

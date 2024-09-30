# Wifi Lightpanel for a camper roof

This project is for a van camper build.

We have an idea to create a bus-system _(ESPNOW)_ from the cockpit to a roof basket,
to reduce the number of cables through the roof and have less cable chaos.

## The Switch panel

.. is the Master ESP32 and built into a Switch panel with a little display for information 
_and a little logo_.
It has control-LED´s to see which light is on or is in malfunction.



## The Relais panel

.. will be built under the roof basket and will controll the lights around the basket. This is an ESP32 (slave) too.
This program is currently a dummy testing program to test features of the masterboard.

The lights will be switched over relays and should be monitored from an input on an ESP to see if a relays will stuck.
This should be sent to the ESP32 on the Cockpit to see it on the LED´s.

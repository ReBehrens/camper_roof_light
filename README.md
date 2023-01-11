# Wifi Lightpanel for a camper

This project is for a van camper build.

We have a idea to create a bus-connections _(ESPNOW)_ from the cockpit to a roof basket,
To reduce the number of cables through the roof and have less cable chaos.

## The Switch panel

.. is the Master ESP32 and build into a Switch panel with a Little Display for informations 
_and a little logo_.
It has control-LED´s to see which light is on.



## The Relais panel

.. will build under the roof basket and controlled the lights around the roof. This is an ESP32 (slave) too.
This program is currently a dummy testing program to test features of the masterboard.

The lights will switched over relays and should be monitored from an input on a ESP to see if a relays will stuck. This should be send to the ESP32 on the Cockpit to see it on the LED´s.

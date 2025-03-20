#ifndef SENSORS_H
#define SENSORS_H

#include <OneWire.h>
#include <DallasTemperature.h>
#include <RTClib.h>
#include <Arduino.h>

// Prototypen
void sensorsSetup();
void sensorsLoop();
float getTemp(DeviceAddress deviceAddress);

// Externe Deklarationen, damit andere Module die Sensor-Adressen kennen:
extern DeviceAddress insideThermometer;
extern DeviceAddress outsideThermometer;

#endif // SENSORS_H

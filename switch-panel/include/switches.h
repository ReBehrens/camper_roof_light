#ifndef SWITCHES_H
#define SWITCHES_H

#include <Arduino.h>

bool getQs1();
bool getQs2();
bool getQs3();
bool getQs4();
bool getQs5();
bool getQs6(); // flasher F/B
bool getQs7(); // flasher R/L
bool isEngineOn();

// Prototypen
void switchesSetup();
void switchesLoop();

#endif // SWITCHES_H

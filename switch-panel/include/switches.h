#ifndef SWITCHES_H
#define SWITCHES_H

#include <Arduino.h>

bool getQs1();
bool getQs2();
bool getQs3();
bool getQs4();
bool getQs5();
bool isEngineOn();

// Prototypen
void switchesSetup();
void switchesLoop();

#endif // SWITCHES_H

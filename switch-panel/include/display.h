#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <U8g2lib.h>

// Initialization and regular updating
void displaySetup();
void displayLoop();

// Display-functions
void logo(bool engineOn);
void warning();
void ready();
void lightActiv();
void temperature();
void clockTime();

#endif // DISPLAY_H

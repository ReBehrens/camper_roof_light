#ifndef GLOBALS_H
#define GLOBALS_H

#include <U8g2lib.h>

// ─────────────────────────────
// Hardware-Pin-Configuration
// ─────────────────────────────
// Inputs
const char SW1 = 33;
const char SW2 = 25;
const char SW3 = 26;
const char SW4 = 27;
const char SW5 = 34;
const char VCC = 36;
const char LIGHT = 39;

// Outputs
const char LED1 = 19;
const char LED2 = 18;
const char LED3 = 5;
const char LED4 = 17;
const char LED5 = 16;
const char LED6 = 23;

// ─────────────────────────────
// Sensor and peripheral configuration
// ─────────────────────────────

// OneWire for Temperature-Sensors
#define ONE_WIRE_PIN 15
#define TEMPERATURE_PRECISION 12

// RTC
#define DS3231_ADDRESS 0x68

// Display-configuration (Example: U8G2_SH1106_128X64_NONAME_F_HW_I2C)
const uint8_t DISPLAY_CLOCK_PIN = 22;
const uint8_t DISPLAY_DATA_PIN = 21;
const uint8_t DISPLAY_RESET_PIN = U8X8_PIN_NONE; // no Reset-connection

// ─────────────────────────────
// ESP-NOW / WLAN-Configuration
// ─────────────────────────────
static const String WIFI_SSID = "Slave";
static const String WIFI_PASSWORD = "Slave_1_Password";
#define CHANNEL 1

// ─────────────────────────────
// Time-Delays (in Millis)
// ─────────────────────────────
const unsigned long START_INTERVAL = 5000; // 5 Sec
const unsigned long SEND_INTERVAL = 500;   // 0,5 Sec
const unsigned long COOLDOWN = 1000;       // 1 Sec
extern unsigned long timeStamp;

// =============================
//        Debug-Modus
const bool debugMode = false;
// =============================

#endif // GLOBALS_H

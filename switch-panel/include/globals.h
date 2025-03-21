#ifndef GLOBALS_H
#define GLOBALS_H

#include <U8g2lib.h>

// ─────────────────────────────
// Hardware-Pin-Configuration
// ─────────────────────────────
// Inputs
const uint8_t SW1 = 33;
const uint8_t SW2 = 25;
const uint8_t SW3 = 26;
const uint8_t SW4 = 27;
const uint8_t SW5 = 34;
const uint8_t VCC = 36;
const uint8_t LIGHT = 39;

// Outputs
const uint8_t LED1 = 19;
const uint8_t LED2 = 18;
const uint8_t LED3 = 5;
const uint8_t LED4 = 17;
const uint8_t LED5 = 16;
const uint8_t LED6 = 23;

// ─────────────────────────────
// Sensor and peripheral configuration
// ─────────────────────────────

// OneWire for Temperature-Sensors
const uint8_t ONE_WIRE_PIN = 15;
const uint8_t TEMPERATURE_PRECISION = 12;

// RTC
#define DS3231_ADDRESS 0x68

// Display-configuration (Example: U8G2_SH1106_128X64_NONAME_F_HW_I2C)
const uint8_t DISPLAY_CLOCK_PIN = 22;
const uint8_t DISPLAY_DATA_PIN = 21;
const uint8_t DISPLAY_RESET_PIN = U8X8_PIN_NONE; // no Reset-connection

// ─────────────────────────────
// ESP-NOW / WLAN-Configuration
// ─────────────────────────────
static const char *WIFI_SSID = "Slave";
static const char *WIFI_PASSWORD = "Slave_1_Password";
const uint8_t ESPNOW_CHANNEL = 1;

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

static int warmup = 0;
#endif // GLOBALS_H

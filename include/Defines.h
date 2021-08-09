#pragma once

#include "Arduino.h"
///
///
///

#define STATICIPADDRESS 192,168,1,51
#define STATICGATEWAY 192,168,1,1
#define STATICSUBNETMASK 255,255,255,0
#define MULTICASTADDRESS 223,0,1,3
#define MULTICASTPORT 8066

#define BAUDRATE 115200


//TODO
//This matches what the article I found recommended: using D3, D4 and D8 for OUTPUT is OK, but it is recommended to not use them for INPUT.

///Pin Defines
// #define PIN_WATERLIMIT_INPUT A0
// #define PIN_WATERLIMIT_OUTPUT_HIGH D0
// #define PIN_ULTRASONIC_TRIGGER D0
// #define PIN_ULTRASONIC_ECHO A0
#define PIN_ULTRASONIC_ECHO D0
#define PIN_ULTRASONIC_TRIGGER D3

#define PIN_RELAIS_0 D4
#define PIN_RELAIS_1 D4
// #define PIN_RELAIS_1 D2
#define PIN_SDA D2
// #define PIN_SDA D3
#define PIN_SCL D1
#define PIN_BUTTON_LEFT D5
#define PIN_BUTTON_MIDDLE D6
#define PIN_BUTTON_RIGHT D7

#define PIN_TEMPERATURE_MESSURE 3 // RX


//OLD DEACTIVAED - CAUSE OF TEMPERAURE MESSURE
//Should be on GND
#define PIN_WIFI_RESET D8



enum WaterPumpModeType{AUTO, MANUELON, MANUELOFF};

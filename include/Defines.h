#pragma once

#include "Arduino.h"
///
///
///

#define STATICIPADDRESS 192,168,1,1
#define STATICGATEWAY 192,168,1,1
#define STATICSUBNETMASK 255,255,255,0


///Pin Defines
#define PIN_WATERLIMIT_INPUT D8
#define PIN_RELAIS_0 D1
#define PIN_RELAIS_1 D2
#define PIN_SDA D3
#define PIN_SCL D4
#define PIN_BUTTON_RIGHT D7
#define PIN_BUTTON_MIDDLE D6
#define PIN_BUTTON_LEFT D5

#define PIN_WIFI_RESET D0


enum WaterPumpModeType{AUTO, MANUELON, MANUELOFF};


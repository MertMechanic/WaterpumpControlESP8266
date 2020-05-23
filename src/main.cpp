#include <Arduino.h>
#include "CWifi.h"
#include "Clcd.h"
#include "Defines.h"

#include "CWaterPumpControl.h"
// CWifi& wifi = CWifi::getInstance();
// Clcd& lcd = Clcd::getInstance();

// #define PIN_WIFI_RESET D0
// CWaterPumpControl control;

void setup() {
      CWaterPumpControl::getInstance().init();
}

void loop() {
  CWaterPumpControl::getInstance().run();
}

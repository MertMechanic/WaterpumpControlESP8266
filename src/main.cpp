#include <Arduino.h>
#include "CWifi.h"
#include "Clcd.h"
#include "Defines.h"

#include "CWaterPumpControl.h"


//Basic Emergency Routine
Ticker emergencyTicker;
int watchdogCount = 0;
void ISRWatchDog()
{
  watchdogCount++;
  if (watchdogCount == 5)
  {
    Serial.println("Watchdog bites!");
    ESP.reset();
  }
  
}

void setup()
{
  //EmergencyTicker - attach
  //if the run() loop not quitting the watchdogcount gets increased
  //after 5 seconds - it will restart the esp

  CWaterPumpControl::getInstance().init();
  emergencyTicker.attach(1,ISRWatchDog);
}

void loop()
{
  watchdogCount = 0;
  CWaterPumpControl::getInstance().run();
}

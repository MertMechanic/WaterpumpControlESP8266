/**
 * @file main.cpp
 * @author Mathias Meister (mathias.meister@fh-erfurt.de)
 * @brief 
 * @version 0.1
 * @date 2020-06-11
 * 
 * @copyright Copyright (c) 2020
 * 
 */


#include <Arduino.h>
#include "CWifi.h"
#include "Clcd.h"
#include "Defines.h"

#include "CWaterPumpControl.h"

//Emergency Routine Setup START

//Basic Emergency Routine
Ticker emergencyTicker;
int watchdogCount = 0;
//Emergency Routine Setup END
/**
 * @brief 
 * 
 */
void ISREmergencyRestartWatchDog()
{
  watchdogCount++;
  if (watchdogCount == 5)
  {
    // Serial.println("Watchdog bites!");
    ESP.reset();
  }
  
}

void setup()
{
  //EmergencyTicker - attach
  //if the run() loop not quitting the watchdogcount gets increased
  //after 5 seconds - it will restart the esp

  CWaterPumpControl::getInstance().init();
  emergencyTicker.attach(1,ISREmergencyRestartWatchDog);
}

void loop()
{
  watchdogCount = 0;
  CWaterPumpControl::getInstance().run();
}

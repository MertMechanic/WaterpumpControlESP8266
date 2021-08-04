//**Waterpump Control Main Method START**//

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
#include "Defines.h"
#include <Arduino.h>
#include "CWaterPumpControl.h"

//Emergency Routine Setup START

//Basic Emergency Routine
Ticker emergencyTicker;
int watchdogCount = 0;

/**
 * @brief EmergenyRestartMethod
 * 
 */
void ISREmergencyRestartWatchDog()
{
  watchdogCount++;

  if (watchdogCount > 1)
  {
     Serial.print("Watchdog Count: ");
     Serial.println(watchdogCount);
  }

  if (watchdogCount == 10)
  {
    Serial.println("Watchdog bites!");
    ESP.reset();
  }
  
}

/**
 * @brief Arduino Setup Run before loop
 * 
 */
void setup()
{
  //EmergencyTicker - attach
  //if the run() loop not quitting the watchdogcount gets increased
  //after 20 seconds - it will restart the esp

  CWaterPumpControl::getInstance().init();
  emergencyTicker.attach(2,ISREmergencyRestartWatchDog);
}

/**
 * @brief Main loop Method
 * 
 */
void loop()
{
  watchdogCount = 0;
  CWaterPumpControl::getInstance().run();
}

#pragma once

#include "Adafruit_VL53L0X.h"

class CSensorAdafruit_VL53L0X
{
public:
    ~CSensorAdafruit_VL53L0X();

    static int initLongRange();
    static void printSerial();
    static int  doMeasureAndGetValue();
    static int  getMiddleValue();
    static bool doMeasure();                //Call this in Interrupt!

    static bool doDummyMeasure();           //Dummy implmentation for simulation
    static int  getMeasureValue();
    

 private:
     static void addNewValueToMiddleValueArray();
     static void initMiddleValueArray();
     static bool isMeasureOk();

 public:
     static Adafruit_VL53L0X m_lox;
     static VL53L0X_RangingMeasurementData_t m_Measure;
     static int m_MesureValue;

     static int m_counterOfFailedMessures;
     static const int s_maxFailedMessures = 25;

     static bool m_MeasureStatusOK;
     static const int s_MAX_RANGE = 2200;


     static bool m_DummyFlag;

     static const int s_MiddleValueArraySize = 50;
     static int m_MiddleValues[s_MiddleValueArraySize];
     static int m_CurrentBufferPosition;
};


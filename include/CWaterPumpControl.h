#pragma once

#include "SPI.h"
#include "CTimeWaterPump.h"
#include "Defines.h"
#include "Ticker.h"
#include "CTimeWaterPumpRingBuffer.h"
#include "CTemperatureSensor.h"
#include "CUltraSonicSensor.h"
#include "CWaterPumpControlAdditionalWebpages.h"

#include "CSensorAdafruit_VL53L0X.h"

class NTPClient                 ;
class AsyncWebserver            ;
class Clcd                      ;
class CWaterPump                ;
class CWifiBasic                ;
class WiFiUDP                   ;
class CWebServerBasic           ;


class CSensorAdafruit_VL53L0X;

enum e_FountainStatus_Flag { FILLED, EMPTY, OVERFILLED, OVEREMPTY, ERROR };
enum e_DisplayMode_Flag{ SINCEMODE, TOFMESSUREMODE };
enum e_SensorAdafruit_Status_Flag { ACTIVE, DISBALED };

class CWaterPumpControl
{
    

    //Stingleton START
public:

    static CWaterPumpControl &getInstance()
    {
        static CWaterPumpControl S;
        return S;
    }

    CWaterPumpControl(const CWaterPumpControl &) = delete;
    void operator=(CWaterPumpControl const &) = delete;

private:
    CWaterPumpControl();
    ~CWaterPumpControl();
    //Stingleton END

public:



    //Main Method of Waterpumpcontrol
    void run();
    void init();

    CWaterPump* getWaterPump();
    void changeModeToAuto();
    void changeModeToManuelOn();
    void changeModeToManuelOff();
    void setTurnOnDelay(int _TurnOnDelayInMinutes);

    CTimeWaterPumpRingBuffer *getSaveRunTimeReversed();
    CTimeWaterPumpRingBuffer *getStopRunTimeReversed();

    bool isWaterInFountain();
    // CTimeWaterPump *getRestartTimeWithDelay();
    CTimeWaterPump *getCurrentCWaterPumpControlTime();
    String* getWaterPumpModeAsStr(String *m_pDestinationStr);
    String* getFountainStatusAsString(String *_pFountainStatus);
    

    static const int S_SIZEOFTIMESSAVED = 3;

    static void readInputButtons();


    Ticker* getCallTickerInputButtons();
    Ticker* getCallTickeReadFountainFilled();
    Ticker* getLCDBacklightTicker();
    Ticker* getTemperatureTicker();
    Ticker* getCallTickerSensorAdafruit_VL53L0X();

    static void deattachTimerToBackLightTurnoff();

    static void readIsWaterInFountain();

    CTimeWaterPump *getRestartTimeWithDelay();

    TemperatureSensor* getTemperatureSensors();
    CUltraSonicSensor* getUltraSonicSensor();    

    static int  getWaterLimitMessure();
    static int  getWaterLimitMaxBorder();
    static int  getWaterLimitMinBorder();
    static void setWaterLimitMax(int _max);
    static void setWaterLimitMin(int _min);

    e_FountainStatus_Flag getFountainStatus();

    e_DisplayMode_Flag getDisplayModeFlag();
    void              setDisplayModeFlag(e_DisplayMode_Flag _flag);

    CSensorAdafruit_VL53L0X* getSensorAdafruit_VL53L0X();

private:
    NTPClient                  *m_pTimeClient;
    CWaterPump                 *m_pWaterpump;
    CWifiBasic                 *m_pWifi;
    WiFiUDP                    *m_pNtpUDP;
    CWebServerBasic            *m_pWebServer;  
    TemperatureSensor           m_TemperatureSensors;
    
    CSensorAdafruit_VL53L0X      m_SensorAdafruit_VL53L0X;
    e_SensorAdafruit_Status_Flag m_SensorAdafruit_Status;

    Clcd                       *m_pLcd;

    bool                        m_ModeHasChanged;
    e_DisplayMode_Flag           m_DisplayModeFlag;
    int                         m_LastSwitchCounter;
    CTimeWaterPump*             m_restartTimeWithDelay;         //A Date of Turning on 
    CTimeWaterPump              m_currentTime;

    static const int S_ACTIVATELIMITBORDER      = 4;
    static const int S_COUNTOFTEMPERAURESENSORS = 2;


    Ticker m_CallTickerInputButtons;
    Ticker m_CallTickerFountainFilled;
    Ticker m_CallTickerTemperatureMessure;
    Ticker m_CallTickerSensorAdafruit_VL53L0X;


    void InitSerialSetup();
    void setStartTimeWithDelay();
    void assignWaterPumpMode(WaterPumpModeType _mode);

//Save a Stop Time (max = S_COUNTOFTIMESAVE)
    void saveStopTime(CTimeWaterPump *_pTime);
    void saveRunTime(CTimeWaterPump *_pTime);


    CTimeWaterPumpRingBuffer m_LastPumpRunTimeRingBuffer;
    CTimeWaterPumpRingBuffer m_LastPumpStopTimeRingBuffer;
    
    CTimeWaterPumpRingBuffer* reverseTimeWaterPumpArray(CTimeWaterPumpRingBuffer *_pArray, CTimeWaterPumpRingBuffer *_pDestination);
    
    //OLD START
    // CTimeWaterPump* reverseTimeWaterPumpArray(CTimeWaterPump *_pArray, CTimeWaterPump *_pDestination);
    // CTimeWaterPump m_LastPumpStopTimeArray[S_COUNTOFTIMESAVE];
    // CTimeWaterPump m_LastPumpRunTimeArray[S_COUNTOFTIMESAVE];
    // CTimeWaterPump* reverseTimeWaterPumpArray(CTimeWaterPump *_pArray, CTimeWaterPump *_pDestination);
    //OLD END 

    //Counter is pointing to the position which need to be written next time ....
    int m_CurrentStopCounter;
    int m_CurrentRunCounter;
    static void attachTimerToInputButtons();
    static void attachTimerToReadFountainFilled();
    static void attachTimerToSensorAdafruit_VL53L0X();
    static void attachTimerToDummySensor();

    bool m_IsTimerTemperatureAndWaterLimitAttached;

    static e_FountainStatus_Flag S_FountainStatus;
    
    static int S_WaterLimitMessure;
    static int S_WaterLimitEmptyBorder;
    static int S_WaterLimitFullBorder;

};
#pragma once


#include "CTimeWaterPump.h"
#include "Defines.h"
#include "Ticker.h"
#include "CTimeWaterPumpRingBuffer.h"

class NTPClient       ;
class ESP8266WebServer;
class Clcd            ;
class CWaterPump      ;
class CWifi           ;
class WiFiUDP         ;
class CWebServer      ;

static bool S_FountainIsFilled;
static int S_WaterlimitCounter;                        //Counts the activation of the WaterLimitSwitch

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


    static const int S_SIZEOFTIMESSAVED = 3;

    static void readInputButtons();


    Ticker* getCallTickerInputButtons();
    Ticker* getCallTickeReadFountainFilled();
    Ticker* getLCDBacklightTicker();

    static void deattachTimerToBackLightTurnoff();

    static void readIsWaterInFountain();

    CTimeWaterPump *getRestartTimeWithDelay();

private:
    NTPClient   *m_pTimeClient;
    Clcd        *m_pLcd;
    CWaterPump  *m_pWaterpump;
    CWifi       *m_pWifi;
    WiFiUDP     *m_pNtpUDP;
    CWebServer  *m_pWebServer;

    bool m_ModeHasChanged;
    int  m_DisplayFlag;
    int  m_LastSwitchCounter;
    CTimeWaterPump* m_restartTimeWithDelay;  //A Date of Turning on 
    CTimeWaterPump m_currentTime;


    static const int S_ACTIVATELIMITBORDER = 4;


    Ticker m_CallTickerInputButtons;
    Ticker m_CallTickerFountainFilled;


    void InitSerialSetup();
    void setStartTimeWithDelay();
    void assignWaterPumpMode(WaterPumpModeType _mode);

//Save a Stop Time (max = S_COUNTOFTIMESAVE)
    void saveStopTime(CTimeWaterPump *_pTime);
    void saveRunTime(CTimeWaterPump *_pTime);



    CTimeWaterPumpRingBuffer* m_pLastPumpRunTimeRingBuffer;
    CTimeWaterPumpRingBuffer* m_pLastPumpStopTimeRingBuffer;
    
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

};
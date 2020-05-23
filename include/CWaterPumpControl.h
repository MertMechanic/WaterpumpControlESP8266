#pragma once


#include "CTimeWaterPump.h"
#include "Defines.h"

class NTPClient       ;
class ESP8266WebServer;
class Clcd            ;
class CWaterPump      ;
class CWifi           ;
class WiFiUDP         ;
class CWebServer      ;


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

    CTimeWaterPump *getSaveRunTimeReversed(CTimeWaterPump *_destination);
    CTimeWaterPump *getStopRunTimeReversed(CTimeWaterPump *_destination);

    bool isWaterInFountain();
    CTimeWaterPump *getRestartTimeWithDelay();
    CTimeWaterPump *getCurrentCWaterPumpControlTime();
    static const int S_COUNTOFTIMESAVE = 3;
private:
    NTPClient *m_pTimeClient;
    Clcd *m_pLcd;
    CWaterPump *m_pWaterpump;
    CWifi *m_pWifi;
    WiFiUDP *m_pNtpUDP;
    CWebServer *m_pWebServer;

    bool m_ModeHasChanged;
    int m_DisplayFlag;
    int m_LastSwitchCounter;
    CTimeWaterPump* m_restartTimeWithDelay;
    CTimeWaterPump m_currentTime;

    void InitSerialSetup();

    void setWaterIsEmpty();
    void setWaterIsNotEmpty();

    void setStartTimeWithDelay();

    void readInputButtons();
    void assignWaterPumpMode(WaterPumpModeType _mode);

//Save a Stop Time (max = S_COUNTOFTIMESAVE)
    void saveStopTime(CTimeWaterPump *_pTime);
    void saveRunTime(CTimeWaterPump *_pTime);

    CTimeWaterPump m_LastPumpStopTimeArray[S_COUNTOFTIMESAVE];
    CTimeWaterPump m_LastPumpRunTimeArray[S_COUNTOFTIMESAVE];
    CTimeWaterPump* reverseTimeWaterPumpArray(CTimeWaterPump *_pArray, CTimeWaterPump *_pDestination);

    //Counter is pointing to the position which need to be written next time ....
    int m_CurrentStopCounter;
    int m_CurrentRunCounter;

};
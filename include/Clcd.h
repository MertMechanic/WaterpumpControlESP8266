#pragma once

#include "Wire.h"
#include <LiquidCrystal_I2C.h>
#include "Arduino.h"
#include "Ticker.h"

enum displayStatus {AUTO_FULL, AUTO_EMPTY, MANUEL_ON, MANUEL_OFF, NONEDISPLAY_STATUS};

class Clcd
{
    

//BASIC SINGLETON START
public:
    static Clcd &getInstance()
    {
        static Clcd S;
        return S;
    }

    Clcd(const Clcd &) = delete;
    void operator=(Clcd const &) = delete;

private:
    Clcd();
    ~Clcd();
//BASIC SINGLETON END


private:
    //Init LCD Display
    LiquidCrystal_I2C *m_pLcd;
    
    //Storarge of the current showing strings
    String m_Line[2];
    
    displayStatus m_DisplayStatus;

    int m_DisplayFlag;
    const int m_CountOfRows = 2;
    const int m_CountOfSignsPerRows = 16 ;

    const String mClearLineStr = "                ";
    Ticker m_LCDBackLightTicker;
public:

    LiquidCrystal_I2C *getLCDInstance();
    
    void init(__uint8_t _sda_pin, __uint8_t _scl_pin);

    void setLine(String *_pText, int _lineNumber);
    void setLineA(String *_pText);
    void setLineB(String *_pText);

    void showManualON();
    void showManualOFF();

    void showWaterIsNotEmpty(String *_ptext);
    void showWaterIsEmpty(String *_pTime);

    void setDisplayText(String *_pTextA, String *_pTextB);
    void setDisplayText(char *_pTextA, char *_pTextB);

    void showWaterIsNotEmptySinceTime();

    void turnOnBacklightAndTurnOffLater();

    void setDisplayStatus(displayStatus _status);
    displayStatus getDisplayStatus();

    static void attachTimerToBackLightTurnoff();
    static void turnOffBackLight();

    void showTurnLoadingRoutine(int _delay, const char *_ploadingSign, bool _direction, String *_pText);

    Ticker* getLCDBacklightTicker();
};

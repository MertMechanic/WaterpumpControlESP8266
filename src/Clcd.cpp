#include "Clcd.h"
#include "Defines.h"
#include "CWaterPumpControl.h"

// #define debug

Clcd::Clcd()
{
}

Clcd::~Clcd()
{
}


void Clcd::init(__uint8_t _sda_pin, __uint8_t _scl_pin)
{
    this->m_DisplayFlag = 0;
    this->m_DisplayStatus = NONEDISPLAY_STATUS;

    m_pLcd = new LiquidCrystal_I2C(0x27, this->m_CountOfSignsPerRows, this->m_CountOfRows);

    // Wire.begin(PIN_SDA, PIN_SCL); //SDA //SCL
    Wire.begin(_sda_pin, _scl_pin); //SDA //SCL

    m_pLcd->init();
    m_pLcd->backlight();
    this->m_pLcd->setCursor(0, 1);
    m_pLcd->print("Setup is running...");
    this->m_pLcd->setCursor(0, 0);

    for (int line = 0; line < m_CountOfRows; line++)
    {
        for (int row = 0; row < m_CountOfSignsPerRows; row++)
        {
            this->m_pLcd->setCursor(row, line);
            this->m_pLcd->print(".");
            delay(50);
        }
    }
    this->m_Line[0] = "                "; //Should be 16 chars
    this->m_Line[1] = "                ";
}



void Clcd::setLine(String *_pText, int _lineNumber)
{
    this->turnOnBacklightAndTurnOffLater();
    if (this->m_Line[_lineNumber].compareTo(*_pText) != 0)
    {
        // Overrride complete String
        this->m_Line[_lineNumber] = *_pText;
        m_pLcd->setCursor(0, _lineNumber); // Cursor0 , Linea0
        m_pLcd->print(mClearLineStr);
        m_pLcd->setCursor(0, _lineNumber);
        m_pLcd->print(*_pText);
        this->m_Line[_lineNumber] = *_pText;


        //TODO TASK, just override real sign instead of updateing hole line
        // //Override just chars
        // //Check each char for change:
        // const char *str = this->m_Line[_lineNumber].c_str();
        // const char *newStr = _pText->c_str();
        // char newStrChar[17] = "                ";
        

        
        
        // Serial.print("NewChar is1:");
        // Serial.println(newStrChar);

        // if (_pText->length() < this->m_CountOfSignsPerRows)
        // {
        //     for (size_t i = 0; i < _pText->length(); i++)
        //     {
        //         newStrChar[i] = newStr[i];
        //     }            
        // }
        // Serial.print("NewChar is2:");
        // Serial.println(newStrChar);

        // for (size_t i = 0; i < this->m_CountOfSignsPerRows + 1 ; i++)
        // {
        //     if (str[i] != newStrChar[i])
        //     {
        //         m_pLcd->setCursor(i,_lineNumber);
        //         m_pLcd->print(newStrChar[i]);
        //     }
        // }
        
        // // this->m_Line[_lineNumber] = *_pText;
    }
}

void Clcd::setDisplayText(String *_pTextA, String *_pTextB)
{
    this->setLine(_pTextA, 0);
    this->setLine(_pTextB, 1);
}

void Clcd::setDisplayText(char *_pTextA, char *_pTextB)
{
    String a(*_pTextA);
    String b(*_pTextB);
    this->setDisplayText(&a,&b);
}


LiquidCrystal_I2C *Clcd::getLCDInstance()
{
    return this->m_pLcd;
}

void Clcd::showWaterIsEmpty(String *_pTtime)
{
    if (this->m_DisplayStatus != AUTO_EMPTY)
    {
        String line0("Brunnen ist Leer");
        String line1("Seit: ");
        line1 = line1 + *_pTtime;

#ifdef debug
        Serial.println(line0);
        Serial.println(this->m_Line[0]);
#endif
        Clcd::getInstance().setDisplayText(&line0, &line1);

        this->m_DisplayStatus = AUTO_EMPTY;
    }
}

void Clcd::showWaterIsNotEmptySinceTime()
{
    if (this->m_DisplayStatus != AUTO_FULL)
    {
        String line0("Brunnen gefuellt");
        String line1 = "Seit: ";
        String time;
        CWaterPumpControl::getInstance().getCurrentCWaterPumpControlTime()->getAsString(&time);

        line1 = line1 + time;

#ifdef debug
        Serial.println(line0);
        Serial.println(this->m_Line[0]);
#endif

        Clcd::getInstance().setDisplayText(&line0, &line1);

        this->m_DisplayStatus = AUTO_FULL;
    }
}

void Clcd::showManualON()
{
    //     1234567890123456

    String a("Auto  ON     OFF");
    String b(" V    XXX     V ");

    this->setDisplayText(&a, &b);

    this->m_DisplayStatus = MANUEL_ON;
}

void Clcd::showManualOFF()
{
    //     1234567890123456
    String a("Auto  ON     OFF");
    String b(" V     V     XXX");
    this->setDisplayText(&a, &b);

    this->m_DisplayStatus = MANUEL_OFF;
}

void Clcd::turnOnBacklightAndTurnOffLater()
{
    this->m_pLcd->backlight();
    this->attachTimerToBackLightTurnoff();
}

void Clcd::attachTimerToBackLightTurnoff()
{
    Clcd::getInstance().getLCDBacklightTicker()->attach_ms(5000, Clcd::turnOffBackLight);
}

void Clcd::turnOffBackLight()
{
    Clcd::getInstance().getLCDInstance()->noBacklight();
    Clcd::getInstance().getLCDBacklightTicker()->detach();
}

Ticker *Clcd::getLCDBacklightTicker()
{
    return &this->m_LCDBackLightTicker;
}

void Clcd::setDisplayStatus(displayStatus _status)
{
    this->m_DisplayStatus = _status;
}

displayStatus Clcd::getDisplayStatus()
{
    return this->m_DisplayStatus;
}

void Clcd::showTurnLoadingRoutine(int _delay, const char *_ploadingSign, bool _direction, String *_pText)
{
    String Sign = String(*_ploadingSign);

    Clcd::getLCDInstance()->clear();

    String lineA = String(*_pText);
    setLine(&lineA, 0);

    if (_direction)
    {
        for (int i = 0; i < m_CountOfSignsPerRows; i++)
        {
            Clcd::getLCDInstance()->setCursor(i, 1);
            Clcd::getLCDInstance()->print(Sign);
            delay(_delay);
        }
    }
    else
    {
        for (int i = m_CountOfSignsPerRows - 1; i >= 0; i--)
        {
            Clcd::getLCDInstance()->setCursor(i, 1);
            Clcd::getLCDInstance()->print(Sign);
            delay(_delay);
        }
    }
}

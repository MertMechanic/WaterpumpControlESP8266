#include "Clcd.h"
#include "Defines.h"
#include "CWaterPumpControl.h"

Clcd::Clcd()
{
}

Clcd::~Clcd()
{
}


void Clcd::init(__uint8_t _sda_pin, __uint8_t _scl_pin)
{
    this->m_DisplayFlag = 0;

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
    if (this->m_Line[_lineNumber] != *_pText)
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


    void Clcd::turnOffBackLight()
    {
        this->m_pLcd->noBacklight();
        CWaterPumpControl::getInstance().deattachTimerToBackLightTurnoff();
    }

void Clcd::setDisplayText(String *_pTextA, String *_pTextB)
{
    this->setLine(_pTextA, 0);
    this->setLine(_pTextB, 1);
}

LiquidCrystal_I2C* Clcd::getLCDInstance()
{
    return this->m_pLcd;
}

void Clcd::showMenu()
{
        this->getLCDInstance()->backlight();
        this->m_Line[0] = "Entering Menu...";
        this->m_Line[1] = " ...";
        this->setDisplayText(&m_Line[0], &m_Line[1]);
        delay(500);

              //     1234567890123456
        this->m_Line[0] = "Auto  ON     OFF";
        this->m_Line[1] = " V     V      V ";
        this->setDisplayText(&m_Line[0], &m_Line[1]);
}

void Clcd::showManualON()
{
                  //     1234567890123456
        this->m_Line[0] = "Auto  ON     OFF";
        this->m_Line[1] = " V    XXX     V ";
        this->setDisplayText(&m_Line[0], &m_Line[1]);
}


void Clcd::showManualOFF()
{
                  //     1234567890123456
        this->m_Line[0] = "Auto  ON     OFF";
        this->m_Line[1] = " V     V     XXX";
        this->setDisplayText(&m_Line[0], &m_Line[1]);
}



void Clcd::showWaterIsNotEmpty(String *_pTextLine2)
{
        String LineA("Brunnen gefuellt");
        this->setDisplayText(&m_Line[0], &m_Line[1]);
}

void Clcd::showWaterIsEmpty(String *_pTtime)
{
    String line1("Brunnen ist Leer");
    String line2("Seit: ");
    line2 = line2 + *_pTtime;
    this->setDisplayText(&line1, &line2);
}
void Clcd::showWaterIsNotEmptySinceTime()
{
    String since = "Seit: ";
    String time;
    CWaterPumpControl::getInstance().getCurrentCWaterPumpControlTime()->getAsString(&time);

    String output = since + time;
    this->showWaterIsNotEmpty(&output);
}
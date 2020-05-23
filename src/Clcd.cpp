#include "Clcd.h"
#include "Defines.h"
#include "CWaterPumpControl.h"

Clcd::Clcd()
{
}

Clcd::~Clcd()
{
    // delete m_pLcd;
}


void Clcd::init(__uint8_t _sda_pin, __uint8_t _scl_pin)
{
    this->mDisplayFlag = 0;

    this->mCountOfRows = 2;
    this->mCountOfSignsPerRows = 16;

    m_pLcd = new LiquidCrystal_I2C(0x27, this->mCountOfSignsPerRows, this->mCountOfRows);

    // Wire.begin(PIN_SDA, PIN_SCL); //SDA //SCL
    Wire.begin(_sda_pin, _scl_pin); //SDA //SCL

    m_pLcd->init();
    m_pLcd->backlight();
    this->m_pLcd->setCursor(0, 1);
    m_pLcd->print("Setup is running...");
    this->m_pLcd->setCursor(0, 0);

    for (int line = 0; line < mCountOfRows; line++)
    {
        for (int row = 0; row < mCountOfSignsPerRows; row++)
        {
            this->m_pLcd->setCursor(row, line);
            this->m_pLcd->print(".");
            delay(50);
        }
    }
    
}



void Clcd::setLineA(String *_pText)
{
    if (this->m_LineA != *_pText)
    {
        this->m_LineA = *_pText;
        m_pLcd->setCursor(0, 0); // Cursor0 , Linea0
        m_pLcd->print(mClearLineStr);
        m_pLcd->setCursor(0, 0);
        m_pLcd->print(*_pText);
    }
}

void Clcd::setLineB(String *_pText)
{
    if (this->m_LineB != *_pText)
    {
        this->m_LineB = *_pText;
        m_pLcd->setCursor(0, 1); // Cursor0 , Linea1
        m_pLcd->print(mClearLineStr);
        m_pLcd->setCursor(0, 1);
        m_pLcd->print(*_pText);
    }
}

void Clcd::setDisplayText(String *_pTextA, String *_pTextB)
{
    this->setLineA(_pTextA);
    this->setLineB(_pTextB);
}

LiquidCrystal_I2C* Clcd::getLCDInstance()
{
    return this->m_pLcd;
}

void Clcd::showMenu()
{
        this->getLCDInstance()->backlight();
        this->m_LineA = "Entering Menu...";
        this->m_LineB = " ...";
        this->setDisplayText(&m_LineA, &m_LineB);
        delay(500);

              //     1234567890123456
        this->m_LineA = "Auto  ON     OFF";
        this->m_LineB = " V     V      V ";
        this->setDisplayText(&m_LineA, &m_LineB);
}

void Clcd::showManualON()
{
                  //     1234567890123456
        this->m_LineA = "Auto  ON     OFF";
        this->m_LineB = " V    XXX     V ";
        this->setDisplayText(&m_LineA, &m_LineB);
}


void Clcd::showManualOFF()
{
                  //     1234567890123456
        this->m_LineA = "Auto  ON     OFF";
        this->m_LineB = " V     V     XXX";
        this->setDisplayText(&m_LineA, &m_LineB);
}



void Clcd::showWaterIsNotEmpty(String *_pTextLine2)
{
        String LineA("Brunnen gefuellt");
        this->setDisplayText(&LineA, _pTextLine2);
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
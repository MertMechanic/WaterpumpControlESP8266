
#include <NTPClient.h>
#include <WiFiUdp.h>


#include "Arduino.h"
#include "Defines.h"
#include "CWaterPumpControl.h"

#include "CWaterPump.h"
#include "CWifi.h"
#include "Clcd.h"

#include "CWaterPump.h"
#include "ESPAsyncWebServer.h"
#include "C2RelayModule.h"


CWaterPumpControl::CWaterPumpControl()
{
    //SaveTime
  this->m_LastPumpRunTimeArray = CTemplateRingBuffer<CTimeWaterPump>(this->S_SIZEOFTIMESSAVED);
  this->m_LastStopTimeArray = CTemplateRingBuffer<CTimeWaterPump>(this->S_SIZEOFTIMESSAVED);
  //STD Constructor - no output cause Serial setup is not finished....
}

CWaterPumpControl::~CWaterPumpControl()
{
  Serial.println("Calling destructor of WaterpumpControl");
  // delete this->m_pWebServer;
  // delete this->m_pWaterpump;
  // delete this->m_pNtpUDP;
  // delete this->m_pTimeClient;
  // delete this->m_pLcd;
}

void CWaterPumpControl::init()
{
  ///INIT PINS
  //
  //NOTE - The Normal 5V Pin can not be used as switich pin because 
  //       there is a problem when 5V is connected to input pin in 
  //       boot sequence
  //
  pinMode(PIN_WATERLIMIT_OUTPUT_HIGH, OUTPUT);    // Waterlimit Switch
  digitalWrite(PIN_WATERLIMIT_OUTPUT_HIGH, HIGH); // Waterlimit Pin HIGH
  pinMode(PIN_WATERLIMIT_INPUT, INPUT_PULLUP);    // Watrelimit Input Pin

  pinMode(PIN_RELAIS_0, OUTPUT);                  // Relais 0
  pinMode(PIN_RELAIS_1, OUTPUT);                  // Relais 1
  pinMode(PIN_BUTTON_LEFT, INPUT_PULLUP);         // Input Pins Button          
  pinMode(PIN_BUTTON_MIDDLE, INPUT_PULLUP);       // Input Pins Button 
  pinMode(PIN_BUTTON_RIGHT, INPUT_PULLUP);        // Input Pins Button 

  ///INIT SERIAL
  Serial.println("Init Serial Setup");
  this->InitSerialSetup();

  ///INIT DISPLAY
  Serial.println("Init LCD Setup");
  Clcd::getInstance().init(PIN_SDA, PIN_SCL);
  this->m_pLcd = &Clcd::getInstance();

  ////INIT WIFISetup
  //
  CWifi::getInstance().initWithLCD(Clcd::getInstance(), PIN_WIFI_RESET);

  this->m_pWifi = &CWifi::getInstance();
  this->m_pWebServer = &this->m_pWifi->getWebserver();

  this->m_pNtpUDP = new WiFiUDP();
  this->m_pTimeClient = new NTPClient(*m_pNtpUDP, "europe.pool.ntp.org", 3600, 60000);
  //
  ////INIT WIFI SETUP


  ///INIT TIME VALUES
  //
  m_pTimeClient->begin();
  //ChangeToSummerTime
  m_pTimeClient->setTimeOffset(7200);

  //Init Values for time saving
  for (int i = 0; i < this->S_SIZEOFTIMESSAVED; i++)
  {
    this->m_LastPumpRunTimeArray[i] = CTimeWaterPump(0, 0, 0);
    this->m_LastStopTimeArray[i] = CTimeWaterPump(0, 0, 0);
  }
  this->m_CurrentRunCounter = 0; //Increase every time when is saved ... < 3 set to 0
  this->m_CurrentStopCounter = 0;
  //
  ////End INIT TIME VALUES

  this->m_restartTimeWithDelay = nullptr;

  //Init RelaysModule
  this->m_pWaterpump = new CWaterPump(new C2RelayModule(PIN_RELAIS_0, PIN_RELAIS_1), 0, WaterPumpModeType::AUTO, false, 0);

  //Test Relays
  this->m_pWaterpump->runTestRelaysModuleWithDelayOf(200);



  this->attachTimerToInputButtons();

}

void CWaterPumpControl::InitSerialSetup()
{
  Serial.begin(9600);
}

void CWaterPumpControl::saveStopTime(CTimeWaterPump *_ptime)
{
  //DEBUG
  Serial.print("save STOP Time... to count:");
  Serial.println(this->m_CurrentStopCounter);
  
  if (_ptime != nullptr)
  {
    this->m_LastStopTimeArray.addValue(_ptime);

    // this->m_LastPumpStopTimeArray[this->m_CurrentStopCounter] = *_ptime;
    
    //increase Counter
    this->m_CurrentStopCounter++;
    if (this->m_CurrentStopCounter == this->S_SIZEOFTIMESSAVED + 1)
    {
      this->m_CurrentStopCounter = 0;
    }
  }

}
void CWaterPumpControl::saveRunTime(CTimeWaterPump *_ptime)
{
  
  //DEBUG
  Serial.print("save RUN Time... to count:");
  Serial.println(this->m_CurrentRunCounter);

  this->m_LastPumpRunTimeArray[this->m_CurrentRunCounter] = *_ptime;
  //increase Counter
  this->m_CurrentRunCounter++;
  if (this->m_CurrentRunCounter == this->S_SIZEOFTIMESSAVED + 1)
  {
    this->m_CurrentRunCounter = 0;
  }
}

//Main Method
void CWaterPumpControl::run()
{
  
  
  this->m_pWifi->run(); //Handle reset Button
                        //HTTP Requests
                        //DNS for AP
  
  if (!this->m_pWifi->isInAPMode())
  {

    if (this->m_ModeHasChanged)
    {
      switch (this->m_pWaterpump->getWaterPumpMode())
      {
      case AUTO:
        this->changeModeToAuto();
        break;
      case MANUELON:
        this->changeModeToManuelOn();
        break;
      case MANUELOFF:
        this->changeModeToManuelOff();
        break;
      default:
        break;
      }
      this->m_pWaterpump->SetTurnONDelay((this->m_pWaterpump->getTurnOnDelay()) + 1);
      Serial.print("Mode is: ");
      Serial.println(this->m_pWaterpump->getWaterPumpMode());
      this->m_ModeHasChanged = false;
    }
    else
    {
      switch (this->m_pWaterpump->getWaterPumpMode())
      {
      case AUTO:
        //Fountain filled and pump running
        if (this->isWaterInFountain() && this->m_pWaterpump->isWaterPumpRunning())
        {
          // Serial.println("Fountain full + Pump running....");

          this->m_pLcd->showWaterIsNotEmptySinceTime();
        }
        //Fountain filled and pump Stopped
        else if (this->isWaterInFountain() && this->m_pWaterpump->isWaterPumpStopped())
        {
          // Serial.println("Fountain full + Pump stopped....");
          this->m_pWaterpump->TurnOnWaterPump();

          this->saveRunTime(this->getCurrentCWaterPumpControlTime());

          this->m_pLcd->showWaterIsNotEmptySinceTime();

          //TODO IMPLEMENNT STARTDELAY
          // //No WaterpumpDelay is set
          // if (this->m_restartTimeWithDelay == nullptr)
          // {
          //   Serial.println("Fountain full + Pump stopped....");
          //   this->m_pWaterpump->TurnOnWaterPump();

          //   this->saveRunTime(this->getCurrentCWaterPumpControlTime());

          //   this->m_pLcd->showWaterIsNotEmptySinceTime();
          // }
          // //WaterpumpDelay is set
          // else
          // {
          //   //get time and check is time > delaytime
          //   CTimeWaterPump *pCurrentTime = this->getCurrentCWaterPumpControlTime();
          //   if (pCurrentTime < this->m_restartTimeWithDelay)
          //   {
          //     //Time is over ! Start Pump

          //     String a("time is not over");
          //     String b("TODO turnon!");
          //   }
          //   else
          //   {
          //     //Time is not over....
          //     String a("time is not over");
          //     String b("TODO countdown");
          //     this->m_pLcd->setDisplayText(&a, &b);
          //   }
          // }
        }
        //Fountain empty and pump running
        else if (!this->isWaterInFountain() && this->m_pWaterpump->isWaterPumpRunning())
        {
          // Serial.println("Fountain empty + Pump running....");
          this->m_pWaterpump->TurnOffWaterPump();
          this->saveStopTime(this->getCurrentCWaterPumpControlTime());
          String time;
          this->m_pLcd->showWaterIsEmpty(this->getCurrentCWaterPumpControlTime()->getAsString(&time));

          // this->setStartTimeWithDelay();
        }
        //Foundtain empty and pump Stopped
        else if (!this->isWaterInFountain() && this->m_pWaterpump->isWaterPumpStopped())
        {
          // Serial.println("Fountain empty + Pump stopped....");
          String time;
          this->m_pLcd->showWaterIsEmpty(this->getCurrentCWaterPumpControlTime()->getAsString(&time));
        }
        break;
      case MANUELON:
        this->m_pWaterpump->TurnOnWaterPump();
        this->m_pLcd->showManualON();
        break;
      case MANUELOFF:
        this->m_pWaterpump->TurnOffWaterPump();
        this->m_pLcd->showManualOFF();
        break;
      default:
        break;
      }
    }

    //Read Inputs
    // this->readInputButtons();

    //update Time....
    this->m_pTimeClient->update();

    this->m_pWebServer->getESP8266WebServer()->handleClient();

    // Wait 1s .... aka 1 Turn....
    // delay(100);
  }
}

//Return a reversed array of TimeWaterpump
CTemplateRingBuffer<CTimeWaterPump>* CWaterPumpControl::reverseTimeWaterPumpArray(CTemplateRingBuffer<CTimeWaterPump> *_pArray, CTemplateRingBuffer<CTimeWaterPump> *_pDestination)
{

  // CTimeWaterPump reversedTime[this->S_COUNTOFTIMESAVE];
  int lastSavedCounter = _pArray->getCurrentPosition();
  // int lastSavedCounter = this->m_CurrentRunCounter;

  for (size_t i = 0; i < _pArray->getBufferSize(); i++)
  {
    if (lastSavedCounter == 0)
    {
      lastSavedCounter = _pArray->getBufferSize();
    }
    lastSavedCounter--;
    _pDestination[i] = _pArray[lastSavedCounter];
  }
  return _pDestination;
}

CTemplateRingBuffer<CTimeWaterPump>* CWaterPumpControl::getSaveRunTimeReversed(CTemplateRingBuffer<CTimeWaterPump>* _destination)
{
  CTemplateRingBuffer<CTimeWaterPump> *reversed = reverseTimeWaterPumpArray(&this->m_LastPumpRunTimeArray, _destination);
  return reversed;
}

CTemplateRingBuffer<CTimeWaterPump>* CWaterPumpControl::getStopRunTimeReversed(CTemplateRingBuffer<CTimeWaterPump>* _destination)
{
  CTemplateRingBuffer<CTimeWaterPump> *reversed = reverseTimeWaterPumpArray(&this->m_LastStopTimeArray, _destination);
  return reversed;
}

CTimeWaterPump *CWaterPumpControl::getCurrentCWaterPumpControlTime()
{

  
  CTimeWaterPump time(this->m_pTimeClient->getHours(), this->m_pTimeClient->getMinutes(), this->m_pTimeClient->getSeconds());
  this->m_currentTime = time;

  //DEBUG
  // Serial.print("getCurrentWaterPumpControlTime");
  // String tmp;
  // Serial.println(*this->m_currentTime.getAsString(&tmp));

  return &this->m_currentTime;
}


void CWaterPumpControl::readInputButtons()
{

  //In Auto Mode
  if (CWaterPumpControl::getInstance().getWaterPump()->getWaterPumpMode() == AUTO)
  {
    //Left Button pressed
    if (digitalRead(PIN_BUTTON_LEFT) == LOW && digitalRead(PIN_BUTTON_MIDDLE) == HIGH && digitalRead(PIN_BUTTON_RIGHT) == HIGH)
    {
      Serial.println("left pressed in automode");
    }
    //Middle Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT) == HIGH && digitalRead(PIN_BUTTON_MIDDLE) == LOW && digitalRead(PIN_BUTTON_RIGHT) == HIGH)
    {
      Serial.println("middle pressed in automode");
      CWaterPumpControl::getInstance().getWaterPump()->setWaterPumpMode(WaterPumpModeType::MANUELON);
    }
    //Right Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT) == HIGH && digitalRead(PIN_BUTTON_MIDDLE) == HIGH && digitalRead(PIN_BUTTON_RIGHT) == LOW)
    {
      Serial.println("right pressed in automode");
            CWaterPumpControl::getInstance().getWaterPump()->setWaterPumpMode(WaterPumpModeType::MANUELOFF);
    }
  }
  else if (CWaterPumpControl::getInstance().getWaterPump()->getWaterPumpMode()  == MANUELON)
  {
    //Left Button pressed
    if (digitalRead(PIN_BUTTON_LEFT) == LOW && digitalRead(PIN_BUTTON_MIDDLE) == HIGH && digitalRead(PIN_BUTTON_RIGHT) == HIGH)
    {
      Serial.println("left pressed in MANUELON");
      CWaterPumpControl::getInstance().assignWaterPumpMode(WaterPumpModeType::AUTO);
    }
    //Middle Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT) == HIGH && digitalRead(PIN_BUTTON_MIDDLE) == LOW && digitalRead(PIN_BUTTON_RIGHT) == HIGH)
    {
      Serial.println("middle pressed in MANUELON");
    }
    //Right Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT) == HIGH && digitalRead(PIN_BUTTON_MIDDLE) == HIGH && digitalRead(PIN_BUTTON_RIGHT) == LOW)
    {
      Serial.println("right pressed in MANUELON");
            CWaterPumpControl::getInstance().getWaterPump()->setWaterPumpMode(WaterPumpModeType::MANUELOFF);
    }
  }
  else if (CWaterPumpControl::getInstance().getWaterPump()->getWaterPumpMode()  == MANUELOFF)
  {
    //Left Button pressed
    if (digitalRead(PIN_BUTTON_LEFT) == LOW && digitalRead(PIN_BUTTON_MIDDLE) == HIGH && digitalRead(PIN_BUTTON_RIGHT) == HIGH)
    {
      Serial.println("left pressed in MANUELOFF");
       CWaterPumpControl::getInstance().assignWaterPumpMode(WaterPumpModeType::AUTO);
    }
    //Middle Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT) == HIGH && digitalRead(PIN_BUTTON_MIDDLE) == LOW && digitalRead(PIN_BUTTON_RIGHT) == HIGH)
    {
      Serial.println("middle pressed in MANUELOFF");
      CWaterPumpControl::getInstance().getWaterPump()->setWaterPumpMode(WaterPumpModeType::MANUELON);
    }
    //Right Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT) == HIGH && digitalRead(PIN_BUTTON_MIDDLE) == HIGH && digitalRead(PIN_BUTTON_RIGHT) == LOW)
    {
      Serial.println("right pressed in MANUELOFF");
    }
  }
  else
  {
    //Nothing
  }
}

void CWaterPumpControl::changeModeToAuto()
{
  this->getWaterPump()->setWaterPumpMode(AUTO);
  if (this->m_DisplayFlag == false)
  {
    String showStr("lastMax:");
    String lastMaxStr;
    if (this->m_LastSwitchCounter == -1)
    {
      lastMaxStr = "---";
    }
    else
    {
      lastMaxStr = String(this->m_LastSwitchCounter);
      lastMaxStr = lastMaxStr + "s";
    }

    showStr = showStr + lastMaxStr;
    this->m_pLcd->setLine(&showStr, 1);
    this->m_DisplayFlag = true;
  }
  else
  {
    this->m_DisplayFlag = false;
    String time(this->m_pTimeClient->getFormattedTime());
    this->m_pLcd->setLine(&time, 1);
    this->m_pLcd->getLCDInstance()->backlight();
    this->attachTimerToBackLightTurnoff();
  }
}
void CWaterPumpControl::changeModeToManuelOn()
{
  this->getWaterPump()->setWaterPumpMode(MANUELON);
  this->m_pWaterpump->TurnOnWaterPump();
  String str(".._-.Manuel.-_..");
  String str2("------ON-------");
  this->m_pLcd->setDisplayText(&str, &str2);
  this->m_pLcd->getLCDInstance()->backlight();
  this->attachTimerToBackLightTurnoff();
}
void CWaterPumpControl::changeModeToManuelOff()
{

  this->getWaterPump()->setWaterPumpMode(MANUELOFF);
  this->m_pWaterpump->TurnOffWaterPump();
  String str(".._-.Manuel.-_..");
  String str2("-----OFF-------");
  this->m_pLcd->setDisplayText(&str, &str2);
  this->m_pLcd->getLCDInstance()->backlight();
  this->attachTimerToBackLightTurnoff();
}

void CWaterPumpControl::setTurnOnDelay(int _startDelayInMinutes)
{
  this->m_pWaterpump->SetTurnONDelay(_startDelayInMinutes);

  String outputLcdLine1("update Delay to:");
  String outputLcdLine2(_startDelayInMinutes);
  String min(" min");
  outputLcdLine2 += min;

  this->m_pLcd->setDisplayText(&outputLcdLine1, &outputLcdLine2);
}



bool CWaterPumpControl::isWaterInFountain()
{
  if (analogRead(PIN_WATERLIMIT_INPUT) > 200)
  {
    return true;
  }
  return false;
  
  // return digitalRead(PIN_WATERLIMIT_INPUT);
}

void CWaterPumpControl::assignWaterPumpMode(WaterPumpModeType _mode)
{
  if (this->m_pWaterpump->getWaterPumpMode() != _mode)
  {
    this->m_ModeHasChanged = true;
    this->m_pWaterpump->setWaterPumpMode(_mode);
  }
}

CWaterPump *CWaterPumpControl::getWaterPump()
{
  return this->m_pWaterpump;
}

//TODO
// void CWaterPumpControl::setStartTimeWithDelay()
// {
//   CTimeWaterPump LastStopTime;
//   LastStopTime = *this->getStopRunTimeReversed(&LastStopTime);

//   if (this->m_restartTimeWithDelay == nullptr)
//   {
//     this->m_restartTimeWithDelay = new CTimeWaterPump(LastStopTime);
//   }
//   else
//   {
//     *this->m_restartTimeWithDelay = LastStopTime;
//   }

//   int TurnOnDelayInMinutes = this->m_pWaterpump->getTurnOnDelay();

//   this->m_restartTimeWithDelay->addMinutes(TurnOnDelayInMinutes);
// }

// CTimeWaterPump *CWaterPumpControl::getRestartTimeWithDelay()
// {
//   return this->m_restartTimeWithDelay;
// }


void CWaterPumpControl::attachTimerToInputButtons()
{
  CWaterPumpControl::getInstance().getButtonCallTicker()->attach_ms(50, CWaterPumpControl::readInputButtons);
}

Ticker* CWaterPumpControl::getButtonCallTicker()
{
  return &this->m_ButtonCallTicker;
}



void CWaterPumpControl::attachTimerToBackLightTurnoff()
{
  CWaterPumpControl::getInstance().getLCDBacklightTicker()->attach_ms(5000, CWaterPumpControl::turnOffBackLight);
}

void CWaterPumpControl::turnOffBackLight()
{
  Clcd::getInstance().getLCDInstance()->noBacklight();
  CWaterPumpControl::getInstance().getLCDBacklightTicker()->detach();
}

Ticker* CWaterPumpControl::getLCDBacklightTicker()
{
  return &this->m_LCDBackLightTicker;
}


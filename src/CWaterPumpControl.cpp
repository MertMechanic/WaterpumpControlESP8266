
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

#define debug 1


CWaterPumpControl::CWaterPumpControl()
{
    //SaveTime
  
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
  // delete this->m_pLastPumpRunTimeRingBuffer ;
  // delete this->m_pLastPumpStopTimeRingBuffer;
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
  #ifdef debug
  Serial.println("Init Serial Setup");
  #endif
  this->InitSerialSetup();

  #ifdef debug
  Serial.println("Serial setup end...");
  #endif

  ///INIT DISPLAY
  #ifdef debug
  Serial.println("Init LCD Setup");
  #endif
  Clcd::getInstance().init(PIN_SDA, PIN_SCL);
  this->m_pLcd = &Clcd::getInstance();

  ////INIT WIFISetup
  //
  CWifi::getInstance().initWithLCD(Clcd::getInstance(), PIN_WIFI_RESET);

  this->m_pWifi = &CWifi::getInstance();
  this->m_pWebServer = &this->m_pWifi->getWebserver();

  this->m_pNtpUDP = new WiFiUDP();
  this->m_pTimeClient = new NTPClient(*m_pNtpUDP, "europe.pool.ntp.org", 3600, 60000);
  #ifdef debug
  Serial.println("WIFI setup end...");
  #endif
  //
  ////INIT WIFI SETUP


  ///INIT TIME VALUES
  //
  m_pTimeClient->begin();
  //ChangeToSummerTime
  m_pTimeClient->setTimeOffset(7200);


  this->m_pLastPumpRunTimeRingBuffer  = new CTimeWaterPumpRingBuffer();
  this->m_pLastPumpStopTimeRingBuffer = new CTimeWaterPumpRingBuffer();

  this->m_pLastPumpRunTimeRingBuffer->setBufferSize(this->S_SIZEOFTIMESSAVED);
  this->m_pLastPumpStopTimeRingBuffer->setBufferSize(this->S_SIZEOFTIMESSAVED);


  #ifdef debug 
  Serial.println( "Memory reserved for Ringbuffer");
  #endif

  //Init Values for time saving
  for (int i = 0; i < this->S_SIZEOFTIMESSAVED; i++)
  {
    CTimeWaterPump dummy(0, 0, 0);
    this->m_pLastPumpRunTimeRingBuffer->addValue(&dummy);

    this->m_pLastPumpStopTimeRingBuffer->addValue(&dummy);

  }
  #ifdef debug 
  Serial.println( "Dummy Values added to ringbuffers");
  #endif



  this->m_CurrentRunCounter = 0; //Increase every time when is saved ... < 3 set to 0
  this->m_CurrentStopCounter = 0;
  //
  ////End INIT TIME VALUES

  this->m_restartTimeWithDelay = nullptr;

  //Init RelaysModule
  this->m_pWaterpump = new CWaterPump(new C2RelayModule(PIN_RELAIS_0, PIN_RELAIS_1), 0, WaterPumpModeType::AUTO, false);
  #ifdef debug 
  Serial.println( "init RelaysModule end...");
  #endif

  //Test Relays
  String a("Relais test");
  String b("running");
  String c("end");
  this->m_pLcd->setLine(&a,0);
  this->m_pLcd->setLine(&b,1);
  this->m_pWaterpump->runTestRelaysModuleWithDelayOf(200);
  this->m_pLcd->setLine(&c,1);


  this->attachTimerToInputButtons();
  #ifdef debug 
  Serial.println( "Interrupttimer to Buttons attached ...");
  #endif

  // String line1("IPAddress is:");
  // String line2(this->m_pWifi->getIpAddress());
  // this->m_pLcd->setDisplayText(&line1, &line2);
  
  // this->m_ModeHasChanged = true;
  

}

void CWaterPumpControl::InitSerialSetup()
{
  Serial.begin(9600);
}

void CWaterPumpControl::saveStopTime(CTimeWaterPump *_ptime)
{
  #ifdef debug
  //DEBUG
  Serial.print("save STOP Time... to count:");
  Serial.println(this->m_CurrentStopCounter);
  #endif

  if (_ptime != nullptr)
  {

    this->m_pLastPumpStopTimeRingBuffer->addValue(_ptime);

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
  #ifdef debug
  //DEBUG
  Serial.print("save RUN Time... to count:");
  Serial.println(this->m_CurrentRunCounter);
  #endif

  this->m_pLastPumpRunTimeRingBuffer->addValue(_ptime);
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
      
      #ifdef debug
      // Serial.print("Mode is: ");
      // Serial.println(this->m_pWaterpump->getWaterPumpMode());
      #endif

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
          // #ifdef debug
          // Serial.println("Fountain full + Pump running....");
          // #endif

          CWaterPumpControl::getInstance().m_pLcd->showWaterIsNotEmptySinceTime();
        }
        //Fountain filled and pump Stopped
        else if (this->isWaterInFountain() && this->m_pWaterpump->isWaterPumpStopped())
        {

          // #ifdef debug
          // Serial.println("Fountain full + Pump stopped....");
          // #endif 

          this->m_pWaterpump->TurnOnWaterPump();

          String msg("Turn On  Waterpump");
          this->m_pLcd->showTurnLoadingRoutine(50, "#", true, &msg);
          this->saveRunTime(this->getCurrentCWaterPumpControlTime());


          CWaterPumpControl::getInstance().m_pLcd->showWaterIsNotEmptySinceTime();

          // TODO IMPLEMENNT STARTDELAY
          //No WaterpumpDelay is set
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
          String msg("Turn Off  Waterpump");
          this->m_pLcd->showTurnLoadingRoutine(50, "#", false, &msg);

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

    //update Time....
    this->m_pTimeClient->update();

    this->m_pWebServer->getESP8266WebServer()->handleClient();


  }
  else
  {
    String a("--- AP-Mode ----");
    String b("--  !Active! --");
    this->m_pLcd->setDisplayText(&a, &b);
  }
}

//Return a reversed array of TimeWaterpump
CTimeWaterPumpRingBuffer *CWaterPumpControl::reverseTimeWaterPumpArray(CTimeWaterPumpRingBuffer *_pArray, CTimeWaterPumpRingBuffer *_pDestination)
{

  int lastSavedCounter = _pArray->getCurrentPosition();

  for (int i = 0; i < _pArray->getBufferSize(); i++)
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

CTimeWaterPumpRingBuffer *CWaterPumpControl::getSaveRunTimeReversed()
{
  return this->m_pLastPumpRunTimeRingBuffer;
}

CTimeWaterPumpRingBuffer *CWaterPumpControl::getStopRunTimeReversed()
{
  return this->m_pLastPumpStopTimeRingBuffer;
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
      CWaterPumpControl::getInstance().m_pLcd->turnOnBacklightAndTurnOffLater();
    }
    //Middle Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT) == HIGH && digitalRead(PIN_BUTTON_MIDDLE) == LOW && digitalRead(PIN_BUTTON_RIGHT) == HIGH)
    {
      Serial.println("middle pressed in automode");
      CWaterPumpControl::getInstance().assignWaterPumpMode(WaterPumpModeType::MANUELON);
    }
    //Right Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT) == HIGH && digitalRead(PIN_BUTTON_MIDDLE) == HIGH && digitalRead(PIN_BUTTON_RIGHT) == LOW)
    {
      Serial.println("right pressed in automode");
      CWaterPumpControl::getInstance().assignWaterPumpMode(WaterPumpModeType::MANUELOFF);
    }
  }
  else if (CWaterPumpControl::getInstance().getWaterPump()->getWaterPumpMode() == MANUELON)
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
      CWaterPumpControl::getInstance().m_pLcd->turnOnBacklightAndTurnOffLater();
    }
    //Right Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT) == HIGH && digitalRead(PIN_BUTTON_MIDDLE) == HIGH && digitalRead(PIN_BUTTON_RIGHT) == LOW)
    {
      Serial.println("right pressed in MANUELON");
      CWaterPumpControl::getInstance().assignWaterPumpMode(WaterPumpModeType::MANUELOFF);
    }
  }
  else if (CWaterPumpControl::getInstance().getWaterPump()->getWaterPumpMode() == MANUELOFF)
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
      CWaterPumpControl::getInstance().assignWaterPumpMode(WaterPumpModeType::MANUELON);
      
    }
    //Right Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT) == HIGH && digitalRead(PIN_BUTTON_MIDDLE) == HIGH && digitalRead(PIN_BUTTON_RIGHT) == LOW)
    {
      Serial.println("right pressed in MANUELOFF");
      CWaterPumpControl::getInstance().m_pLcd->turnOnBacklightAndTurnOffLater();
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

  #ifdef debug
  Serial.println("change mode to Auto Method called....");
  #endif
  // if (this->m_DisplayFlag == false)
  // {
  //   String autoStr("Auto Mode");
  //   String showStr("lastMax:");
  //   String lastMaxStr;
  //   if (this->m_LastSwitchCounter == -1)
  //   {
  //     lastMaxStr = "---";
  //   }
  //   else
  //   {
  //     lastMaxStr = String(this->m_LastSwitchCounter);
  //     lastMaxStr = lastMaxStr + "s";
  //   }

  //   showStr = showStr + lastMaxStr;
  //   this->m_pLcd->setLine(&autoStr, 0);
  //   this->m_pLcd->setLine(&showStr, 1);
  //   this->m_DisplayFlag = true;
  // }
  // else
  // {
  //   this->m_DisplayFlag = false;
  //   String time(this->m_pTimeClient->getFormattedTime());
  //   this->m_pLcd->setLine(&time, 1);
    
  // }
}
void CWaterPumpControl::changeModeToManuelOn()
{
  #ifdef debug
  Serial.println("change mode to Manuel ON Method called....");
  #endif

  this->getWaterPump()->setWaterPumpMode(MANUELON);
  this->m_pWaterpump->TurnOnWaterPump();
  CWaterPumpControl::m_pLcd->showManualON();
}
void CWaterPumpControl::changeModeToManuelOff()
{
  #ifdef debug
  Serial.println("change mode to Manuel OFF Method called....");
  #endif

  this->getWaterPump()->setWaterPumpMode(MANUELOFF);
  this->m_pWaterpump->TurnOffWaterPump();
  CWaterPumpControl::m_pLcd->showManualOFF();
}

void CWaterPumpControl::setTurnOnDelay(int _startDelayInMinutes)
{
  #ifdef debug
  Serial.println("change mode to setTurnOnDelay Method called....");
  #endif

  this->m_pWaterpump->setTurnONDelay(_startDelayInMinutes);

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
  CWaterPumpControl::getInstance().getButtonCallTicker()->attach_ms(100, CWaterPumpControl::readInputButtons);
}

Ticker* CWaterPumpControl::getButtonCallTicker()
{
  return &this->m_ButtonCallTicker;
}




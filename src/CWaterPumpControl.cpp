
#include "Arduino.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "ArduinoOTA.h"
#include "Defines.h"
#include "CWaterPumpControl.h"
#include "CWaterPump.h"
#include "LittleFS.h"
#include "CFilesystem.h"
#include "Clcd.h"
#include "CAdditionalWebpages.h"
#include "CWaterPumpControlAdditionalWebpages.h"
#include "CWaterPump.h"
#include "C2RelayModule.h"
#include "CTemperatureSensor.h"
#include "CWifiBasic.h"
#include "CWebServerBasic.h"
#include "CSensorAdafruit_VL53L0X.h"

//** Commend this out for Debugging 
//
// #define debug 1
//#define noWIFIREST 1


int  CWaterPumpControl::S_WaterLimitMessure;
int  CWaterPumpControl::S_WaterLimitEmptyBorder;
int  CWaterPumpControl::S_WaterLimitFullBorder;
e_FountainStatus CWaterPumpControl::S_FountainStatus;

CWaterPumpControl::CWaterPumpControl()
{
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


// Ticker m_CallTickerSensor;

// void attachTimerToSensor()
// {
//   m_CallTickerSensor.attach_ms(75, CSensorAdafruit_VL53L0X::doMeasure);
// }


/**
 * @brief Init Method run - once before running run()
 * -> Define PinMode of Arduino 
 * 
 */
void CWaterPumpControl::init()
{
  ///INIT PINS
  //
  //NOTE - The Normal 5V Pin can not be used as switich pin because
  //       there is a problem when 5V is connected to input pin in
  //       boot sequence
  // //
  // pinMode   (PIN_WATERLIMIT_OUTPUT_HIGH, OUTPUT);    // Waterlimit Switch
  // digitalWrite(PIN_WATERLIMIT_OUTPUT_HIGH, HIGH);    // Waterlimit Pin HIGH
  // pinMode   (PIN_WATERLIMIT_INPUT, INPUT_PULLUP);    // Watrelimit Input Pin

  pinMode(PIN_RELAIS_0, OUTPUT);            // Relais 0
  pinMode(PIN_RELAIS_1, OUTPUT);            // Relais 1
  pinMode(PIN_BUTTON_LEFT, INPUT_PULLUP);   // Input Pins Button LEFT 
  pinMode(PIN_BUTTON_MIDDLE, INPUT_PULLUP); // Input Pins Button MIDDLE
  pinMode(PIN_BUTTON_RIGHT, INPUT_PULLUP);  // Input Pins Button RIGHT

///INIT SERIAL
#ifdef debug
  Serial.println("Init Serial Setup");
#endif

  this->InitSerialSetup();  //init Serial out for Monitoring

#ifdef debug
  Serial.println("Serial setup end...");
#endif


///INIT DISPLAY
#ifdef debug
  Serial.println("Init LCD Setup");
#endif

  //Init I2C Bus                                  //Init I2C Bus is not needed - will be done on LCD MODULE
  // Wire.begin(PIN_SCL, PIN_SDA);
  //Init LCD Display
  Clcd::getInstance().init();                     //Init LCD
  this->m_DisplayModeFlag = e_DisplayModeFlag::SINCEMODE;

///INIT TOF
#ifdef debug
  Serial.println("Init TOF Sensor");
#endif
  CSensorAdafruit_VL53L0X sensor;

while(sensor.initLongRange() != 0 )
{
  String str1 = "Failed init TOF";
  String str2 = "Check Wires!";

  Clcd::getInstance().setDisplayText(&str1, &str2);
  delay(300);
  Clcd::getInstance().turnOffBackLight();
  delay(100);
  Clcd::getInstance().turnOnBacklightAndTurnOffLater();
#ifdef debug
  Serial.println("Init TOF Sensor - FAILED");
#endif
}
#ifdef debug
  Serial.println("Init TOF Sensor - SUCCESS");
#endif

  

    // delay(100);
    // attachTimerToSensor();
    // delay(100);



  ////INIT WIFISetup
  //
  //**Init Wifi, Wifi with LCD for showing IP and settings on LCD
  //  Init WebServer Instance
  //  Init NTP UDP for getting TimeMessages via NTP
  //  Init TimeClient which is also used for Time
  this->m_pWifi       = &CWifiBasic::getInstance();    // Init Wifi 

  CWaterPumpControlAdditionalWebpages AdditionalWebpages;
  CAdditionalWebpages* p = (CAdditionalWebpages*)&AdditionalWebpages;      

  this->m_pWifi->init(PIN_WIFI_RESET, p);                                                 // Init Wifi 
  this->m_pWebServer  = &this->m_pWifi->getWebserver();                                   // Init Webserver
  this->m_pNtpUDP     = new WiFiUDP();                                                    // Init NTP UDP
  this->m_pTimeClient = new NTPClient(*m_pNtpUDP, "europe.pool.ntp.org", 3600, 60000);    // Init NTP Client
#ifdef debug
  Serial.println("WIFI setup end...");
#endif

  //
  ////INIT WIFI SETUP

  ///**** Start INIT TIME VALUES ****///
  //
  //
  m_pTimeClient->begin();               // Start NTP
  m_pTimeClient->setTimeOffset(7200);   // Change Time of NTP to Summertime

  /* Init RingBuffer for TimeSaving */

  this->m_LastPumpRunTimeRingBuffer.setBufferSize(this->S_SIZEOFTIMESSAVED);    //Set Buffersize of Ringbuffer
  this->m_LastPumpStopTimeRingBuffer.setBufferSize(this->S_SIZEOFTIMESSAVED);   //Set Buffersize of Ringbuffer

#ifdef debug
  Serial.println("Memory reserved for Ringbuffer");
#endif

  //Init Values for time saving
  for (int i = 0; i < this->S_SIZEOFTIMESSAVED; i++)
  {
    CTimeWaterPump dummy(0, 0, 0);
    this->m_LastPumpRunTimeRingBuffer.addValue(&dummy);

    this->m_LastPumpStopTimeRingBuffer.addValue(&dummy);
  }
#ifdef debug
  Serial.println("Dummy Values added to ringbuffers");
#endif

  this->m_CurrentRunCounter   = 0; //Increase every time when is saved ... < 3 set to 0
  this->m_CurrentStopCounter  = 0;
  //
  //
  ///**** End INIT TIME VALUES ****///

  this->m_restartTimeWithDelay = nullptr;

  //Init RelaysModule
  this->m_pWaterpump = new CWaterPump(new C2RelayModule(PIN_RELAIS_0, PIN_RELAIS_1), 0, WaterPumpModeType::AUTO, false);
#ifdef debug
  Serial.println("init RelaysModule end...");
#endif

//   //Test Relays
   String a("Relais test");                                      
   String b("running");                                        
   String c("end");                         
   Clcd::getInstance().setLine(&a, 0);
   Clcd::getInstance().setLine(&b, 1);
  // this->m_pWaterpump->runTestRelaysModuleWithDelayOf(200);

#ifdef debug
  Serial.println("init RTurnOffWaterPump()...");
#endif
  this->m_pWaterpump->TurnOffWaterPump();

#ifdef debug
  Serial.println("init print End()...");
#endif
  Clcd::getInstance().setLine(&c, 1);


  S_FountainStatus = EMPTY;

  m_IsTimerTemperatureAndWaterLimitAttached = false;
#ifdef debug
  Serial.println("attachtimer()...");
#endif
 this->attachTimerToInputButtons();
#ifdef debug
 Serial.println("Interrupttimer to Buttons attached ...");
#endif

   
   // pinMode(PIN_TEMPERATURE_MESSURE, INPUT);  //Temperature MEssure
   m_TemperatureSensors.init(CWaterPumpControl::S_COUNTOFTEMPERAURESENSORS, PIN_TEMPERATURE_MESSURE); 

#ifdef debug
 Serial.println("Tempinit Done ...");
#endif


// Reading Values from files
//
//

 String WaterLimitMax;
 String WaterLimitMin;
 String StartDelay;
 if (CFileSystem::getInstance().readFile("/watermax.txt", &WaterLimitMax))
 {
   this->S_WaterLimitEmptyBorder = WaterLimitMax.toInt();
   Serial.print("File watermax.txt readed success: ");
   Serial.println(this->S_WaterLimitEmptyBorder);
 }
 if (CFileSystem::getInstance().readFile("/watermin.txt", &WaterLimitMin))
 {
   this->S_WaterLimitFullBorder = WaterLimitMin.toInt();
   Serial.print("File watermin.txt readed success: ");
   Serial.println(this->S_WaterLimitFullBorder);
 }
 if (CFileSystem::getInstance().readFile("/startdelay.txt", &StartDelay))
 {
   this->setTurnOnDelay(StartDelay.toInt());
   Serial.print("File startdelay.txt readed success: ");
   Serial.println(StartDelay.toInt());
 }

 //Show all Files to Serial Terminal
 #ifdef debug
  CFileSystem::getInstance().listDir("/", 1);
 #endif
}

void CWaterPumpControl::InitSerialSetup()
{
  Serial.begin(BAUDRATE);
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

    this->m_LastPumpStopTimeRingBuffer.addValue(_ptime);

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

  this->m_LastPumpRunTimeRingBuffer.addValue(_ptime);
  //increase Counter
  this->m_CurrentRunCounter++;
  if (this->m_CurrentRunCounter == this->S_SIZEOFTIMESSAVED + 1)
  {
    this->m_CurrentRunCounter = 0;
  }
}



CTimeWaterPump* CWaterPumpControl::getRestartTimeWithDelay()
{
  return this->m_restartTimeWithDelay;
}

//Main Method
/**
 * @brief Looping Method run while 1
 * 
 */
void CWaterPumpControl::run()
{



#ifdef debug
  Serial.println("Wifi run()");
#endif
  this->m_pWifi->run(); //Handle reset Button
                        //HTTP Requests
                        //DNS for AP
                        //OTA loop
  //update Time....
  this->m_pTimeClient->update();

  //Check is not in AP mode
  if (!this->m_pWifi->isInAPMode())
  {

#ifdef debug
    Serial.println("update Temperature()");
#endif
    this->m_TemperatureSensors.updateTemperature();

    //TODO ADD MULTICAST FOR DATERECORDING....
    //String str("Hello World");
    //
    // this->m_pWifi->sendUDPMultiCast(&str);


    if(!m_IsTimerTemperatureAndWaterLimitAttached)
    {
      attachTimerToReadFountainFilled();
      attachTimerToSensorAdafruit_VL53L0X();

      this->m_IsTimerTemperatureAndWaterLimitAttached = true;
    }

    //Run Methods if mode changed ...
    //
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

          #ifdef debug
          Serial.println("Fountain full + Pump running....");
          #endif
          String lineA;
          String lineB;
          int    Messure;
          switch (this->m_DisplayModeFlag)
          {
          case e_DisplayModeFlag::SINCEMODE:
            Clcd::getInstance().showWaterIsNotEmptySinceTime();
            break;

          case e_DisplayModeFlag::TOFMESSUREMODE:

             Messure = CWaterPumpControl::getInstance().getSensorAdafruit_VL53L0X()->getMiddleValue();
            lineA = "Messung: ";

            if (Messure == -1)
            {
              lineA = lineA + "ERROR !";
            }
            else
            {
              lineA = lineA + String(Messure) + "mm";
            }

            lineB = "Min:" + String(this->S_WaterLimitEmptyBorder) + "Max:" + String(this->S_WaterLimitFullBorder);
            Clcd::getInstance().setDisplayText(&lineA, &lineB);
            break;

          default:
            break;
          }
          
        }
        //Fountain filled and pump Stopped
        else if (this->isWaterInFountain() && this->m_pWaterpump->isWaterPumpStopped())
        {

          #ifdef debug
          Serial.println("Fountain full + Pump stopped....");
          #endif

          int StartDelayInMinutes = CWaterPumpControl::getInstance().getWaterPump()->getTurnOnDelay();
          if (StartDelayInMinutes > 0 &&  this->m_restartTimeWithDelay == nullptr)
          {

            this->m_restartTimeWithDelay = new CTimeWaterPump(this->m_currentTime);
            this->m_restartTimeWithDelay->addMinutes(StartDelayInMinutes);

            String msg("Brunnen voll");
            Clcd::getInstance().showTurnLoadingRoutine(50, "+", true, &msg);
            
            msg = "Start bei";
            String msg2;
            
            msg2 = *(this->m_restartTimeWithDelay->getAsString(&msg2));
            Clcd::getInstance().setLine(&msg, 0);
            Clcd::getInstance().setLine(&msg2, 1);
          }
          else if(StartDelayInMinutes > 0 &&  this->m_restartTimeWithDelay != nullptr)
          {
            this->getCurrentCWaterPumpControlTime();
            //Serial.print( this->m_currentTime.getMinute());
            //Serial.print( this->m_restartTimeWithDelay->getMinute());
            //Serial.print("-");
            if (this->m_currentTime.getHour() == this->m_restartTimeWithDelay->getHour() && this->m_currentTime.getMinute() == this->m_restartTimeWithDelay->getMinute())
            {
              this->m_pWaterpump->TurnOnWaterPump();
              String msg("Turn On  Waterpump");
              Clcd::getInstance().showTurnLoadingRoutine(50, "#", true, &msg);
              this->saveRunTime(this->getCurrentCWaterPumpControlTime());
              Clcd::getInstance().showWaterIsNotEmptySinceTime();

              delete this->m_restartTimeWithDelay;
              this->m_restartTimeWithDelay = nullptr;
              
            }
          }
          else
          {
            this->m_pWaterpump->TurnOnWaterPump();
            String msg("Turn On  Waterpump");
             Clcd::getInstance().showTurnLoadingRoutine(50, "#", true, &msg);
            this->saveRunTime(this->getCurrentCWaterPumpControlTime());
            Clcd::getInstance().showWaterIsNotEmptySinceTime();

          }
          






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

          #ifdef debug
          Serial.println("Fountain empty + Pump running....");
          #endif

          this->m_pWaterpump->TurnOffWaterPump();
          #ifdef debug
          Serial.println("TurnOffWaterPump....End");
          #endif

          String msg("Turn Off  Waterpump");
          Clcd::getInstance().showTurnLoadingRoutine(50, "#", false, &msg);

          #ifdef debug
          Serial.println("showTurnLoadingRoutine....End");
          #endif


          this->saveStopTime(this->getCurrentCWaterPumpControlTime());
          String time;
          Clcd::getInstance().showWaterIsEmpty(this->getCurrentCWaterPumpControlTime()->getAsString(&time));
          // this->setStartTimeWithDelay();

          #ifdef debug
          Serial.println("Save StopTime and Show message on Display ....End");
          #endif

        }
        //Foundtain empty and pump Stopped
        else if (!this->isWaterInFountain() && this->m_pWaterpump->isWaterPumpStopped())
        {
          #ifdef debug
          Serial.println("Fountain empty + Pump stopped....");
          #endif
          
          if(this->m_restartTimeWithDelay != nullptr)
          {
            Serial.println("m_restartTimeWithDelay Deleting ....");
            delete this->m_restartTimeWithDelay ;
            this->m_restartTimeWithDelay = nullptr ;
          }

          #ifdef debug
          Serial.println("showWaterIsEmpty .... with time");
          #endif 

          String time;
          Clcd::getInstance().showWaterIsEmpty(this->getCurrentCWaterPumpControlTime()->getAsString(&time));
          
        }
        break;
      case MANUELON:
        this->m_pWaterpump->TurnOnWaterPump();
        Clcd::getInstance().showManualON();
        delete this->m_restartTimeWithDelay;
        this->m_restartTimeWithDelay = nullptr;
        break;
      case MANUELOFF:
        this->m_pWaterpump->TurnOffWaterPump();
        Clcd::getInstance().showManualOFF();
        delete this->m_restartTimeWithDelay;
        this->m_restartTimeWithDelay = nullptr;
        break;
      default:
        break;
      }
    }

    this->m_pWebServer->getESP8266WebServer()->handleClient();
    // Serial.println("Webserver handle Client....");
  }
  else
  {
    String a("--- AP-Mode ----");
    String b("--  !Active! --");
    Clcd::getInstance().setDisplayText(&a, &b);
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
  return &this->m_LastPumpRunTimeRingBuffer;
}

CTimeWaterPumpRingBuffer *CWaterPumpControl::getStopRunTimeReversed()
{
  return &this->m_LastPumpStopTimeRingBuffer;
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


/**
 * @brief Read input Buttons and Change the mode 
 * 
 */
void CWaterPumpControl::readInputButtons()
{

  //In Auto Mode
  if (CWaterPumpControl::getInstance().getWaterPump()->getWaterPumpMode() == AUTO)
  {
    //Left Button pressed
    if (digitalRead(PIN_BUTTON_LEFT)   == LOW  && 
        digitalRead(PIN_BUTTON_MIDDLE) == HIGH && 
        digitalRead(PIN_BUTTON_RIGHT)  == HIGH)
    {
      #ifdef debug
      Serial.println("left pressed in automode");
      #endif
      Clcd::getInstance().turnOnBacklightAndTurnOffLater();

      if(CWaterPumpControl::getInstance().getWaterPump()->getWaterPumpMode() == WaterPumpModeType::AUTO &&
         CWaterPumpControl::getInstance().getDisplayModeFlag() == e_DisplayModeFlag::SINCEMODE)
      {
        CWaterPumpControl::getInstance().setDisplayModeFlag(e_DisplayModeFlag::TOFMESSUREMODE);
        #ifdef debug
        Serial.println("e_DisplayModeFlag::TOFMESSUREMODE");
      #endif
      }
      else
      {
        CWaterPumpControl::getInstance().setDisplayModeFlag(e_DisplayModeFlag::SINCEMODE);
        #ifdef debug
        Serial.println("e_DisplayModeFlag::SINCEMODE");
        #endif

      }
    }
    //Middle Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT)   == HIGH &&
             digitalRead(PIN_BUTTON_MIDDLE) == LOW  && 
             digitalRead(PIN_BUTTON_RIGHT)  == HIGH)
    {
      Serial.println("middle pressed in automode");
      CWaterPumpControl::getInstance().assignWaterPumpMode(WaterPumpModeType::MANUELON);
    }
    //Right Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT)   == HIGH && 
             digitalRead(PIN_BUTTON_MIDDLE) == HIGH && 
             digitalRead(PIN_BUTTON_RIGHT)   == LOW)
    {
      Serial.println("right pressed in automode");
      CWaterPumpControl::getInstance().assignWaterPumpMode(WaterPumpModeType::MANUELOFF);
    }
  }
  else if (CWaterPumpControl::getInstance().getWaterPump()->getWaterPumpMode() == MANUELON)
  {
    //Left Button pressed
    if (digitalRead(PIN_BUTTON_LEFT) == LOW  && 
    digitalRead(PIN_BUTTON_MIDDLE)   == HIGH && 
    digitalRead(PIN_BUTTON_RIGHT)    == HIGH)
    {
      Serial.println("left pressed in MANUELON");
      CWaterPumpControl::getInstance().assignWaterPumpMode(WaterPumpModeType::AUTO);
    }
    //Middle Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT)   == HIGH && 
             digitalRead(PIN_BUTTON_MIDDLE) == LOW  && 
             digitalRead(PIN_BUTTON_RIGHT)  == HIGH)
    {
      Serial.println("middle pressed in MANUELON");
      Clcd::getInstance().turnOnBacklightAndTurnOffLater();
    }
    //Right Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT)   == HIGH && 
             digitalRead(PIN_BUTTON_MIDDLE) == HIGH && 
             digitalRead(PIN_BUTTON_RIGHT)  == LOW)
    {
      Serial.println("right pressed in MANUELON");
      CWaterPumpControl::getInstance().assignWaterPumpMode(WaterPumpModeType::MANUELOFF);
    }
  }
  else if (CWaterPumpControl::getInstance().getWaterPump()->getWaterPumpMode() == MANUELOFF)
  {
    //Left Button pressed
    if (digitalRead(PIN_BUTTON_LEFT)   == LOW  && 
        digitalRead(PIN_BUTTON_MIDDLE) == HIGH && 
        digitalRead(PIN_BUTTON_RIGHT)  == HIGH)
    {
      Serial.println("left pressed in MANUELOFF");
      CWaterPumpControl::getInstance().assignWaterPumpMode(WaterPumpModeType::AUTO);
    }
    //Middle Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT)   == HIGH && 
             digitalRead(PIN_BUTTON_MIDDLE) == LOW  && 
             digitalRead(PIN_BUTTON_RIGHT)  == HIGH)
    {
      Serial.println("middle pressed in MANUELOFF");
      CWaterPumpControl::getInstance().assignWaterPumpMode(WaterPumpModeType::MANUELON);
    }
    //Right Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT)   == HIGH && 
             digitalRead(PIN_BUTTON_MIDDLE) == HIGH && 
             digitalRead(PIN_BUTTON_RIGHT)  == LOW)
    {
      Serial.println("right pressed in MANUELOFF");
      Clcd::getInstance().turnOnBacklightAndTurnOffLater();
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
  Clcd::getInstance().showManualON();
}
void CWaterPumpControl::changeModeToManuelOff()
{
#ifdef debug
  Serial.println("change mode to Manuel OFF Method called....");
#endif

  this->getWaterPump()->setWaterPumpMode(MANUELOFF);
  this->m_pWaterpump->TurnOffWaterPump();
  Clcd::getInstance().showManualOFF();
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

  Clcd::getInstance().setDisplayText(&outputLcdLine1, &outputLcdLine2);
}

bool CWaterPumpControl::isWaterInFountain()
{
  if (S_FountainStatus == FILLED || S_FountainStatus == OVERFILLED)
  {
    return true;
  }
  
  return false;
}

e_FountainStatus CWaterPumpControl::getFountainStatus()
{
  return S_FountainStatus;
}

// void CWaterPumpControl::readIsWaterInFountain()
// {

//   Serial.println("readIsWaterInFountain");

//   int WaterLimitValue = CWaterPumpControl::getInstance().getUltraSonicSensor()->getDistanceInCM();
//   bool WaterMessureOk = false;

//   if (WaterLimitValue > 0)
//   {
//     CWaterPumpControl::S_WaterLimitMessure = WaterLimitValue;
    
//     Serial.print(S_WaterLimitMessure);
//     Serial.println("cm");

//     Serial.print("Max:");
//     Serial.print(S_WaterLimitEmptyBorder);
//     Serial.print(", Min:");
//     Serial.println(S_WaterLimitFullBorder);

//       if (CWaterPumpControl::S_WaterLimitMessure > CWaterPumpControl::S_WaterLimitEmptyBorder && CWaterPumpControl::S_WaterLimitMessure <= CWaterPumpControl::S_WaterLimitFullBorder)
//       {
//         CWaterPumpControl::S_FountainStatus = FILLED;
//         Serial.println("FILLED!");
//         return;
//       }
//       else if (CWaterPumpControl::S_WaterLimitMessure > CWaterPumpControl::S_WaterLimitEmptyBorder && CWaterPumpControl::S_WaterLimitMessure > CWaterPumpControl::S_WaterLimitFullBorder)
//       {
//         CWaterPumpControl::S_FountainStatus = OVEREMPTY;
//         Serial.println("OVEREMPTY!");
//         return;
//       }
//       else if (S_WaterLimitMessure == S_WaterLimitEmptyBorder && S_WaterLimitMessure < S_WaterLimitFullBorder)
//       {
//         CWaterPumpControl::S_FountainStatus = EMPTY;
//         Serial.println("EMPTY!");
//         return;
//       }
//       else if (S_WaterLimitMessure < S_WaterLimitEmptyBorder && S_WaterLimitMessure < S_WaterLimitFullBorder)
//       {
//         CWaterPumpControl::S_FountainStatus = OVERFILLED;
//         Serial.println("OVERFILLED!");
//         return;
//       }
//   }
//   else
//   {
//     CWaterPumpControl::S_WaterLimitMessure = -1;
//     CWaterPumpControl::S_FountainStatus = ERROR;
//     Serial.println("EMPTY!");
   
//   }

//   Serial.println("readIsWaterInFountain end");
//   return;
// }


void CWaterPumpControl::readIsWaterInFountain()
{
#ifdef debug
  Serial.println("readIsWaterInFountain");
#endif

  // int WaterLimitValue = CWaterPumpControl::getInstance().getUltraSonicSensor()->getDistanceInCM();
  int WaterLimitValue = CWaterPumpControl::getInstance().getSensorAdafruit_VL53L0X()->getMiddleValue();
  // int WaterLimitValue = CWaterPumpControl::getInstance().getSensorAdafruit_VL53L0X()->m_MesureValue;

  // int WaterLimitValue = 10;
  Serial.print("MessureValue: ");
  Serial.println(WaterLimitValue);

  if (WaterLimitValue > 0)
  {
    CWaterPumpControl::S_WaterLimitMessure = WaterLimitValue;
#ifdef debug
    Serial.print(S_WaterLimitMessure);
    Serial.println("cm");

    Serial.print("Max:");
    Serial.print(S_WaterLimitEmptyBorder);
    Serial.print(", Min:");
    Serial.println(S_WaterLimitFullBorder);
#endif

    if (CWaterPumpControl::S_WaterLimitMessure > CWaterPumpControl::S_WaterLimitEmptyBorder && CWaterPumpControl::S_WaterLimitMessure <= CWaterPumpControl::S_WaterLimitFullBorder)
    {
      CWaterPumpControl::S_FountainStatus = FILLED;
#ifdef debug
        Serial.println("FILLED!");
#endif
        return;
      }
      else if (CWaterPumpControl::S_WaterLimitMessure > CWaterPumpControl::S_WaterLimitEmptyBorder && CWaterPumpControl::S_WaterLimitMessure > CWaterPumpControl::S_WaterLimitFullBorder)
      {
        CWaterPumpControl::S_FountainStatus = OVEREMPTY;
#ifdef debug
        Serial.println("OVEREMPTY!");
#endif
        return;
      }
      else if (S_WaterLimitMessure == S_WaterLimitEmptyBorder && S_WaterLimitMessure < S_WaterLimitFullBorder)
      {
        CWaterPumpControl::S_FountainStatus = EMPTY;
#ifdef debug
        Serial.println("EMPTY!");
#endif
        return;
      }
      else if (S_WaterLimitMessure < S_WaterLimitEmptyBorder && S_WaterLimitMessure < S_WaterLimitFullBorder)
      {
        CWaterPumpControl::S_FountainStatus = OVERFILLED;
#ifdef debug
        Serial.println("OVERFILLED!");
#endif
        return;
      }
  }
  // else
  // {
  //   CWaterPumpControl::S_WaterLimitMessure = -1;
  //   CWaterPumpControl::S_FountainStatus = ERROR;
  //   Serial.println("ERROR!");
   
  // }

  Serial.println("readIsWaterInFountain end");
  return;
}


void CWaterPumpControl::setWaterLimitMax(int _max)
{
    S_WaterLimitEmptyBorder = _max;
}


void CWaterPumpControl::setWaterLimitMin(int _min)
{
    S_WaterLimitFullBorder = _min;
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


void CWaterPumpControl::attachTimerToInputButtons()
{
  CWaterPumpControl::getInstance().getCallTickerInputButtons()->attach_ms(100, CWaterPumpControl::readInputButtons);
}

Ticker *CWaterPumpControl::getCallTickerInputButtons()
{
  return &this->m_CallTickerInputButtons;
}

Ticker *CWaterPumpControl::getCallTickeReadFountainFilled()
{
  return &this->m_CallTickerFountainFilled;
}

Ticker* CWaterPumpControl::getTemperatureTicker()
{
   return &this->m_CallTickerTemperatureMessure;
}

Ticker* CWaterPumpControl::getCallTickerSensorAdafruit_VL53L0X()
{
  return &this->m_CallTickerSensorAdafruit_VL53L0X;
}


void CWaterPumpControl::attachTimerToReadFountainFilled()
{
  CWaterPumpControl::getInstance().getCallTickeReadFountainFilled()->attach_ms(100, CWaterPumpControl::readIsWaterInFountain );
}
void CWaterPumpControl::attachTimerToSensorAdafruit_VL53L0X()
{
  CWaterPumpControl::getInstance().getCallTickerSensorAdafruit_VL53L0X()->attach_ms(100, CSensorAdafruit_VL53L0X::doMeasure);
}

TemperatureSensor* CWaterPumpControl::getTemperatureSensors()
{
  return &this->m_TemperatureSensors;
}

// CUltraSonicSensor* CWaterPumpControl::getUltraSonicSensor()
// {
//   return &this->m_UltraSonicSensor;
// }

CSensorAdafruit_VL53L0X* CWaterPumpControl::getSensorAdafruit_VL53L0X()
{
  return &this->m_SensorAdafruit_VL53L0X;
}

int CWaterPumpControl::getWaterLimitMessure()
{
  return S_WaterLimitMessure;
}
int CWaterPumpControl::getWaterLimitMaxBorder()
{
  return S_WaterLimitEmptyBorder;
}
int CWaterPumpControl::getWaterLimitMinBorder()
{
  return S_WaterLimitFullBorder;
}



String* CWaterPumpControl::getWaterPumpModeAsStr(String *m_pDestinationStr)
{

  switch (CWaterPumpControl::getInstance().getWaterPump()->getWaterPumpMode())
  {
  case AUTO:
    *m_pDestinationStr = "AUTO";
    break;
  case MANUELON:
    *m_pDestinationStr = "MANUELON";
    break;
  case MANUELOFF:
    *m_pDestinationStr = "MANUELOFF";
    break;
  default:
    *m_pDestinationStr = "ERROR";
    break;
  };
  return m_pDestinationStr;
}

String* CWaterPumpControl::getFountainStatusAsString(String *_pFountainStatus)
{
  switch (CWaterPumpControl::getInstance().getFountainStatus())
  {
  case e_FountainStatus::FILLED:
    *_pFountainStatus = "FILLED";
    break;
  case e_FountainStatus::OVERFILLED:
    *_pFountainStatus = "OVERFILLED";
    break;
  case e_FountainStatus::EMPTY:
    *_pFountainStatus = "EMPTY";
    break;
  case e_FountainStatus::OVEREMPTY:
    *_pFountainStatus = "OVEREMPTY";
    break;
  case e_FountainStatus::ERROR:
    *_pFountainStatus = "ERROR";
    break;
  default:
    *_pFountainStatus = "ERROR";
    break;
  };
  return _pFountainStatus;
}

e_DisplayModeFlag CWaterPumpControl::getDisplayModeFlag()
{
  return this->m_DisplayModeFlag;
}

void CWaterPumpControl::setDisplayModeFlag(e_DisplayModeFlag _flag)
{
  this->m_DisplayModeFlag = _flag;
}
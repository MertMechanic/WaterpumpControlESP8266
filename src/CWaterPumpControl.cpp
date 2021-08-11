
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
e_FountainStatus_Flag CWaterPumpControl::S_FountainStatus;


static void Debug_SerialPrintOnDebug(char* _message)
{
#ifdef debug
  Serial.print(_message);
#endif
}

static void Debug_SerialPrintOnDebug(int _message)
{
#ifdef debug
  Serial.print(_message);
#endif
}

static void Debug_SerialPrintOnDebugLine(char *_message)
{
#ifdef debug
  Serial.println(_message);
#endif
}

static void Debug_SerialPrintOnDebugLine(int _message)
{
#ifdef debug
  Serial.println(_message);
#endif
}

CWaterPumpControl::CWaterPumpControl()
{
}

CWaterPumpControl::~CWaterPumpControl()
{
  Debug_SerialPrintOnDebugLine("Calling destructor of WaterpumpControl");
}

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
  Debug_SerialPrintOnDebugLine("Init Serial Setup");

  this->InitSerialSetup();  //init Serial out for Monitoring
  
  Debug_SerialPrintOnDebugLine("Serial setup end...");

///INIT DISPLAY
  Debug_SerialPrintOnDebugLine("Init LCD Setup");


  //Init I2C Bus                                  //Init I2C Bus is not needed - will be done on LCD MODULE
  // Wire.begin(PIN_SCL, PIN_SDA);
  //Init LCD Display
  Clcd::getInstance().init();                     //Init LCD
  this->m_DisplayModeFlag = e_DisplayMode_Flag::SINCEMODE;

  String str1;
  String str2;

///INIT TOF
  Debug_SerialPrintOnDebugLine("Init TOF Sensor");

  CSensorAdafruit_VL53L0X sensor;

this->m_SensorAdafruit_Status = ACTIVE;

while(sensor.initLongRange() != 0 )
{
  String str1 = "Failed init ToF";
  String str2 = "Check Wires!";

  Clcd::getInstance().setDisplayText(&str1, &str2);
  delay(500);
  Clcd::getInstance().turnOffBackLight();
  delay(100);
  Clcd::getInstance().turnOnBacklightAndTurnOffLater();

  str1 = "Press ON to";
  str2 = "Skip setup ToF";

  Clcd::getInstance().setDisplayText(&str1, &str2);
  delay(500);
  Clcd::getInstance().turnOffBackLight();
  delay(100);
  Clcd::getInstance().turnOnBacklightAndTurnOffLater();


  Debug_SerialPrintOnDebugLine("Init TOF Sensor - FAILED");


/**
 * @brief if middle button will be pressed while failed to boot tof -> the sensor will be hard disabled!
 * 
 */
  if (digitalRead(PIN_BUTTON_MIDDLE) != HIGH)
  {

    Debug_SerialPrintOnDebugLine("Pressed");

    this->m_SensorAdafruit_Status = DISBALED;

    str1 = "Skipping TOF is";
    str2 = "DISABLED!";
    Clcd::getInstance().setDisplayText(&str1, &str2);
    delay(1000);

    break;
  }
}


if (this->m_SensorAdafruit_Status == ACTIVE)
  Debug_SerialPrintOnDebugLine("Init TOF Sensor - SUCCESS");


////INIT WIFISetup
//
//**Init Wifi, Wifi with LCD for showing IP and settings on LCD
//  Init WebServer Instance
//  Init NTP UDP for getting TimeMessages via NTP
//  Init TimeClient which is also used for Time
str1 = "Configure Wifi";
str2 = "";
Clcd::getInstance().setDisplayText(&str1, &str2);

this->m_pWifi = &CWifiBasic::getInstance(); // Init Wifi

CWaterPumpControlAdditionalWebpages AdditionalWebpages;
CAdditionalWebpages *p = (CAdditionalWebpages *)&AdditionalWebpages;

this->m_pWifi->init(PIN_WIFI_RESET, p);                                              // Init Wifi
this->m_pWebServer = &this->m_pWifi->getWebserver();                                 // Init Webserver
this->m_pNtpUDP = new WiFiUDP();                                                     // Init NTP UDP
this->m_pTimeClient = new NTPClient(*m_pNtpUDP, "europe.pool.ntp.org", 3600, 60000); // Init NTP Client

str1 = "IP_Address:";
str2 = this->m_pWifi->getIpAddress();
Clcd::getInstance().setDisplayText(&str1, &str2);
delay(1000);


  Debug_SerialPrintOnDebugLine("WIFI setup end...");


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


  Debug_SerialPrintOnDebugLine("Memory reserved for Ringbuffer");


  //Init Values for time saving
  for (int i = 0; i < this->S_SIZEOFTIMESSAVED; i++)
  {
    CTimeWaterPump dummy(0, 0, 0);
    this->m_LastPumpRunTimeRingBuffer.addValue(&dummy);

    this->m_LastPumpStopTimeRingBuffer.addValue(&dummy);
  }

  Debug_SerialPrintOnDebugLine("Dummy Values added to ringbuffers");


  this->m_CurrentRunCounter   = 0; //Increase every time when is saved ... < 3 set to 0
  this->m_CurrentStopCounter  = 0;
  //
  //
  ///**** End INIT TIME VALUES ****///

  this->m_restartTimeWithDelay = nullptr;

  //Init RelaysModule
  this->m_pWaterpump = new CWaterPump(new C2RelayModule(PIN_RELAIS_0, PIN_RELAIS_1), 0, WaterPumpModeType::AUTO, false);

  Debug_SerialPrintOnDebugLine("init RelaysModule end...");


//   //Test Relays
   String a("Relais test");                                      
   String b("running");                                        
   String c("end");                         
   Clcd::getInstance().setLine(&a, 0);
   Clcd::getInstance().setLine(&b, 1);
  // this->m_pWaterpump->runTestRelaysModuleWithDelayOf(200);


  Debug_SerialPrintOnDebugLine("init RTurnOffWaterPump()...");

  this->m_pWaterpump->TurnOffWaterPump();


  Debug_SerialPrintOnDebugLine("init print End()...");

  Clcd::getInstance().setLine(&c, 1);


  S_FountainStatus = EMPTY;

  m_IsTimerTemperatureAndWaterLimitAttached = false;

  Debug_SerialPrintOnDebugLine("attachtimer()...");

 this->attachTimerToInputButtons();

 Debug_SerialPrintOnDebugLine("Interrupttimer to Buttons attached ...");


   
   // pinMode(PIN_TEMPERATURE_MESSURE, INPUT);  //Temperature MEssure
   m_TemperatureSensors.init(CWaterPumpControl::S_COUNTOFTEMPERAURESENSORS, PIN_TEMPERATURE_MESSURE); 


 Debug_SerialPrintOnDebugLine("Tempinit Done ...");



// Reading Values from files
//
//

 String WaterLimitMax;
 String WaterLimitMin;
 String StartDelay;
 if (CFileSystem::getInstance().readFile("/watermax.txt", &WaterLimitMax))
 {
   this->S_WaterLimitEmptyBorder = WaterLimitMax.toInt();

   Debug_SerialPrintOnDebug("File watermax.txt readed success: ");
   Debug_SerialPrintOnDebugLine(this->S_WaterLimitEmptyBorder);

 }
 if (CFileSystem::getInstance().readFile("/watermin.txt", &WaterLimitMin))
 {

   this->S_WaterLimitFullBorder = WaterLimitMin.toInt();

   Debug_SerialPrintOnDebug("File watermin.txt readed success: ");
   Debug_SerialPrintOnDebugLine(this->S_WaterLimitFullBorder);

 }
 if (CFileSystem::getInstance().readFile("/startdelay.txt", &StartDelay))
 {
   this->setTurnOnDelay(StartDelay.toInt());

   Debug_SerialPrintOnDebug("File startdelay.txt readed success: ");
   Debug_SerialPrintOnDebugLine(StartDelay.toInt());

 }

//Show all Files to Serial Terminal

  CFileSystem::getInstance().listDir("/", 1);

}

void CWaterPumpControl::InitSerialSetup()
{
  Serial.begin(BAUDRATE);
}

void CWaterPumpControl::saveStopTime(CTimeWaterPump *_ptime)
{

  //DEBUG
  Debug_SerialPrintOnDebug("save STOP Time... to count:");
  Debug_SerialPrintOnDebugLine(this->m_CurrentStopCounter);


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

  //DEBUG
  Debug_SerialPrintOnDebug("save RUN Time... to count:");
  Debug_SerialPrintOnDebugLine(this->m_CurrentRunCounter);


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




  Debug_SerialPrintOnDebugLine("Wifi run()");

  this->m_pWifi->run(); //Handle reset Button
                        //HTTP Requests
                        //DNS for AP
                        //OTA loop
  //update Time....
  this->m_pTimeClient->update();

  //Check is not in AP mode
  if (!this->m_pWifi->isInAPMode())
  {


    Debug_SerialPrintOnDebugLine("update Temperature()");

    this->m_TemperatureSensors.updateTemperature();

    //TODO ADD MULTICAST FOR DATERECORDING....
    //String str("Hello World");
    //
    // this->m_pWifi->sendUDPMultiCast(&str);


    if(!m_IsTimerTemperatureAndWaterLimitAttached)
    {
      attachTimerToReadFountainFilled();
      if(this->m_SensorAdafruit_Status == ACTIVE)
      {
        attachTimerToSensorAdafruit_VL53L0X();
      }
      else
      {
        attachTimerToDummySensor();
      }
      

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


// Debug_SerialPrintOnDebug("Mode is: ");
// Debug_SerialPrintOnDebugLine(this->m_pWaterpump->getWaterPumpMode());


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

          
          Debug_SerialPrintOnDebugLine("Fountain full + Pump running....");
          
          String lineA;
          String lineB;
          int    Messure;
          switch (this->m_DisplayModeFlag)
          {
          case e_DisplayMode_Flag::SINCEMODE:
            Clcd::getInstance().showWaterIsNotEmptySinceTime();
            break;

          case e_DisplayMode_Flag::TOFMESSUREMODE:

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

          
          Debug_SerialPrintOnDebugLine("Fountain full + Pump stopped....");
          

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
            //Debug_SerialPrintOnDebug( this->m_currentTime.getMinute());
            //Debug_SerialPrintOnDebug( this->m_restartTimeWithDelay->getMinute());
            //Debug_SerialPrintOnDebug("-");
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
          //   Debug_SerialPrintOnDebugLine("Fountain full + Pump stopped....");
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

          
          Debug_SerialPrintOnDebugLine("CWaterPumpControl::Fountain empty + Pump running....");
          

          this->m_pWaterpump->TurnOffWaterPump();
          
          Debug_SerialPrintOnDebugLine("TurnOffWaterPump....End");
          

          String msg("Turn Off  Waterpump");
          Clcd::getInstance().showTurnLoadingRoutine(50, "#", false, &msg);

          
          Debug_SerialPrintOnDebugLine("showTurnLoadingRoutine....End");
          


          this->saveStopTime(this->getCurrentCWaterPumpControlTime());
          String time;
          Clcd::getInstance().showWaterIsEmpty(this->getCurrentCWaterPumpControlTime()->getAsString(&time));
          // this->setStartTimeWithDelay();

          
          Debug_SerialPrintOnDebugLine("Save StopTime and Show message on Display ....End");
          

        }
        //Foundtain empty and pump Stopped
        else if (!this->isWaterInFountain() && this->m_pWaterpump->isWaterPumpStopped())
        {
          
          Debug_SerialPrintOnDebugLine("Fountain empty + Pump stopped....");
          
          
          if(this->m_restartTimeWithDelay != nullptr)
          {

            Debug_SerialPrintOnDebugLine("m_restartTimeWithDelay Deleting ....");

            delete this->m_restartTimeWithDelay;
            this->m_restartTimeWithDelay = nullptr ;
          }

          
          Debug_SerialPrintOnDebugLine("showWaterIsEmpty .... with time");
           

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
    // Debug_SerialPrintOnDebugLine("Webserver handle Client....");
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
  // Debug_SerialPrintOnDebug("getCurrentWaterPumpControlTime");
  // String tmp;
  // Debug_SerialPrintOnDebugLine(*this->m_currentTime.getAsString(&tmp));

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

      Debug_SerialPrintOnDebugLine("left pressed in automode");

      Clcd::getInstance().turnOnBacklightAndTurnOffLater();

      if(CWaterPumpControl::getInstance().getWaterPump()->getWaterPumpMode() == WaterPumpModeType::AUTO &&
         CWaterPumpControl::getInstance().getDisplayModeFlag() == e_DisplayMode_Flag::SINCEMODE)
      {
        CWaterPumpControl::getInstance().setDisplayModeFlag(e_DisplayMode_Flag::TOFMESSUREMODE);

        Debug_SerialPrintOnDebugLine("e_DisplayModeFlag::TOFMESSUREMODE");

      }
      else
      {
        CWaterPumpControl::getInstance().setDisplayModeFlag(e_DisplayMode_Flag::SINCEMODE);

        Debug_SerialPrintOnDebugLine("e_DisplayModeFlag::SINCEMODE");

      }
    }
    //Middle Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT)   == HIGH &&
             digitalRead(PIN_BUTTON_MIDDLE) == LOW  && 
             digitalRead(PIN_BUTTON_RIGHT)  == HIGH)
    {

      Debug_SerialPrintOnDebugLine("middle pressed in automode");

      CWaterPumpControl::getInstance().assignWaterPumpMode(WaterPumpModeType::MANUELON);
    }
    //Right Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT)   == HIGH && 
             digitalRead(PIN_BUTTON_MIDDLE) == HIGH && 
             digitalRead(PIN_BUTTON_RIGHT)   == LOW)
    {

      Debug_SerialPrintOnDebugLine("right pressed in automode");

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

      Debug_SerialPrintOnDebugLine("left pressed in MANUELON");

      CWaterPumpControl::getInstance().assignWaterPumpMode(WaterPumpModeType::AUTO);
    }
    //Middle Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT)   == HIGH && 
             digitalRead(PIN_BUTTON_MIDDLE) == LOW  && 
             digitalRead(PIN_BUTTON_RIGHT)  == HIGH)
    {

      Debug_SerialPrintOnDebugLine("middle pressed in MANUELON");

      Clcd::getInstance().turnOnBacklightAndTurnOffLater();
    }
    //Right Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT)   == HIGH && 
             digitalRead(PIN_BUTTON_MIDDLE) == HIGH && 
             digitalRead(PIN_BUTTON_RIGHT)  == LOW)
    {

      Debug_SerialPrintOnDebugLine("right pressed in MANUELON");

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

      Debug_SerialPrintOnDebugLine("left pressed in MANUELOFF");

      CWaterPumpControl::getInstance().assignWaterPumpMode(WaterPumpModeType::AUTO);
    }
    //Middle Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT)   == HIGH && 
             digitalRead(PIN_BUTTON_MIDDLE) == LOW  && 
             digitalRead(PIN_BUTTON_RIGHT)  == HIGH)
    {

      Debug_SerialPrintOnDebugLine("middle pressed in MANUELOFF");

      CWaterPumpControl::getInstance().assignWaterPumpMode(WaterPumpModeType::MANUELON);
    }
    //Right Button pressed
    else if (digitalRead(PIN_BUTTON_LEFT)   == HIGH && 
             digitalRead(PIN_BUTTON_MIDDLE) == HIGH && 
             digitalRead(PIN_BUTTON_RIGHT)  == LOW)
    {

      Debug_SerialPrintOnDebugLine("right pressed in MANUELOFF");

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


  Debug_SerialPrintOnDebugLine("change mode to Auto Method called....");


 
}
void CWaterPumpControl::changeModeToManuelOn()
{

  Debug_SerialPrintOnDebugLine("change mode to Manuel ON Method called....");


  this->getWaterPump()->setWaterPumpMode(MANUELON);
  this->m_pWaterpump->TurnOnWaterPump();
  Clcd::getInstance().showManualON();
}
void CWaterPumpControl::changeModeToManuelOff()
{

  Debug_SerialPrintOnDebugLine("change mode to Manuel OFF Method called....");


  this->getWaterPump()->setWaterPumpMode(MANUELOFF);
  this->m_pWaterpump->TurnOffWaterPump();
  Clcd::getInstance().showManualOFF();
}

void CWaterPumpControl::setTurnOnDelay(int _startDelayInMinutes)
{

  Debug_SerialPrintOnDebugLine("change mode to setTurnOnDelay Method called....");


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

e_FountainStatus_Flag CWaterPumpControl::getFountainStatus()
{
  return S_FountainStatus;
}

void CWaterPumpControl::readIsWaterInFountain()
{

  Debug_SerialPrintOnDebugLine("readIsWaterInFountain");


  // int WaterLimitValue = CWaterPumpControl::getInstance().getUltraSonicSensor()->getDistanceInCM();
  int WaterLimitValue = CWaterPumpControl::getInstance().getSensorAdafruit_VL53L0X()->getMiddleValue();
  // int WaterLimitValue = CWaterPumpControl::getInstance().getSensorAdafruit_VL53L0X()->m_MesureValue;

  // int WaterLimitValue = 10;

  Debug_SerialPrintOnDebug("MessureValue: ");
  Debug_SerialPrintOnDebugLine(WaterLimitValue);

  if (WaterLimitValue > 0)
  {
    CWaterPumpControl::S_WaterLimitMessure = WaterLimitValue;

    Debug_SerialPrintOnDebug(S_WaterLimitMessure);
    Debug_SerialPrintOnDebugLine("cm");

    Debug_SerialPrintOnDebug("Max:");
    Debug_SerialPrintOnDebug(S_WaterLimitEmptyBorder);
    Debug_SerialPrintOnDebug(", Min:");
    Debug_SerialPrintOnDebugLine(S_WaterLimitFullBorder);


    if (CWaterPumpControl::S_WaterLimitMessure > CWaterPumpControl::S_WaterLimitEmptyBorder && CWaterPumpControl::S_WaterLimitMessure <= CWaterPumpControl::S_WaterLimitFullBorder)
    {
      CWaterPumpControl::S_FountainStatus = FILLED;

        Debug_SerialPrintOnDebugLine("FILLED!");

        return;
      }
      else if (CWaterPumpControl::S_WaterLimitMessure > CWaterPumpControl::S_WaterLimitEmptyBorder && CWaterPumpControl::S_WaterLimitMessure > CWaterPumpControl::S_WaterLimitFullBorder)
      {
        CWaterPumpControl::S_FountainStatus = OVEREMPTY;

        Debug_SerialPrintOnDebugLine("OVEREMPTY!");

        return;
      }
      else if (S_WaterLimitMessure == S_WaterLimitEmptyBorder && S_WaterLimitMessure < S_WaterLimitFullBorder)
      {
        CWaterPumpControl::S_FountainStatus = EMPTY;

        Debug_SerialPrintOnDebugLine("EMPTY!");

        return;
      }
      else if (S_WaterLimitMessure < S_WaterLimitEmptyBorder && S_WaterLimitMessure < S_WaterLimitFullBorder)
      {
        CWaterPumpControl::S_FountainStatus = OVERFILLED;

        Debug_SerialPrintOnDebugLine("OVERFILLED!");

        return;
      }
  }

 
  Debug_SerialPrintOnDebugLine("readIsWaterInFountain end");

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

void CWaterPumpControl::attachTimerToDummySensor()
{
  CWaterPumpControl::getInstance().getCallTickerSensorAdafruit_VL53L0X()->attach_ms(100, CSensorAdafruit_VL53L0X::doDummyMeasure);
}


TemperatureSensor* CWaterPumpControl::getTemperatureSensors()
{
  return &this->m_TemperatureSensors;
}

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
  case e_FountainStatus_Flag::FILLED:
    *_pFountainStatus = "FILLED";
    break;
  case e_FountainStatus_Flag::OVERFILLED:
    *_pFountainStatus = "OVERFILLED";
    break;
  case e_FountainStatus_Flag::EMPTY:
    *_pFountainStatus = "EMPTY";
    break;
  case e_FountainStatus_Flag::OVEREMPTY:
    *_pFountainStatus = "OVEREMPTY";
    break;
  case e_FountainStatus_Flag::ERROR:
    *_pFountainStatus = "ERROR";
    break;
  default:
    *_pFountainStatus = "ERROR";
    break;
  };
  return _pFountainStatus;
}

e_DisplayMode_Flag CWaterPumpControl::getDisplayModeFlag()
{
  return this->m_DisplayModeFlag;
}

void CWaterPumpControl::setDisplayModeFlag(e_DisplayMode_Flag _flag)
{
  this->m_DisplayModeFlag = _flag;
}
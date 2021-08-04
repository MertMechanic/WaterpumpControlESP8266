
#include "CWaterPumpControlAdditionalWebpages.h"

#include "ArduinoJson.h"
#include "FS.h"

#include "CFileSystem.h"

#include "ESP8266WebServer.h"
#include "CWifiBasic.h"

#include "DNSServer.h"
#include "CWaterPumpControl.h"
#include "CWaterPump.h"
#include "Clcd.h"
#include "CTemperatureSensor.h"

//#define debug 1

//GLOBAL FUNCTIONS FOR WEBSERVER HTTP REQUESTS
//

String CWaterPumpControlAdditionalWebpages::m_html = "empty";
String CWaterPumpControlAdditionalWebpages::m_JsData = "empty";

CWaterPumpControlAdditionalWebpages::CWaterPumpControlAdditionalWebpages()
{
}
CWaterPumpControlAdditionalWebpages::~CWaterPumpControlAdditionalWebpages()
{
}

/**
 * @brief This method is for collecting all information and send prepare them to send all as json 
 * 
 */
static void handleFetchDataForDashBoardAdditional()
{
    Serial.println("handleFetchDataForDashBoardAdditional()");

    String waterPumpModeStr;



    DynamicJsonBuffer jsonBuffer;
    // create an object
    JsonObject &jsonObject = jsonBuffer.createObject();


    jsonObject["status"]         = CWaterPumpControl::getInstance().isWaterInFountain(); // false or true
    String fountainStatusStr;
    CWaterPumpControl::getInstance().getFountainStatusAsString(&fountainStatusStr);
    jsonObject["fountainstatus"] = fountainStatusStr; // Empty, full, overempty, overful

    //Read Temperature
    String tempStr;
    jsonObject["temperature1"] = *CWaterPumpControl::getInstance().getTemperatureSensors()->getValueByIndexAsString(0, &tempStr);
    jsonObject["temperature2"] = *CWaterPumpControl::getInstance().getTemperatureSensors()->getValueByIndexAsString(1, &tempStr);

    //Read WaterLimits + Current Value
    jsonObject["waterlimitdistance"] = CWaterPumpControl::getInstance().getWaterLimitMessure();
    jsonObject["waterlimitminborder"] = CWaterPumpControl::getInstance().getWaterLimitMinBorder();
    jsonObject["waterlimitmaxborder"] = CWaterPumpControl::getInstance().getWaterLimitMaxBorder();

    jsonObject["restarttimestr"] = "-";
    if (CWaterPumpControl::getInstance().getRestartTimeWithDelay() != nullptr)
    {
        #ifdef debug
        #endif 
        String* ptmp = CWaterPumpControl::getInstance().getRestartTimeWithDelay()->getAsString(&tempStr);
        Serial.print("Pumpenstart bei");
        Serial.println(*ptmp);
        jsonObject["restarttimestr"] = *CWaterPumpControl::getInstance().getRestartTimeWithDelay()->getAsString(&tempStr);
    }

    int StartDelay = CWaterPumpControl::getInstance().getWaterPump()->getTurnOnDelay();
    jsonObject["startdelay"] = String(StartDelay);

    jsonObject["runtime0"] = *CWaterPumpControl::getInstance().getSaveRunTimeReversed()->getData(0)->getAsString(&tempStr);
    jsonObject["runtime1"] = *CWaterPumpControl::getInstance().getSaveRunTimeReversed()->getData(1)->getAsString(&tempStr);
    jsonObject["runtime2"] = *CWaterPumpControl::getInstance().getSaveRunTimeReversed()->getData(2)->getAsString(&tempStr);

    jsonObject["stoptime0"] = *CWaterPumpControl::getInstance().getStopRunTimeReversed()->getData(0)->getAsString(&tempStr);
    jsonObject["stoptime1"] = *CWaterPumpControl::getInstance().getStopRunTimeReversed()->getData(1)->getAsString(&tempStr);
    jsonObject["stoptime2"] = *CWaterPumpControl::getInstance().getStopRunTimeReversed()->getData(2)->getAsString(&tempStr);


    //getting current Pumpmode and generate a String
    CWaterPumpControl::getInstance().getWaterPumpModeAsStr(&waterPumpModeStr);
    jsonObject["mode"] = waterPumpModeStr;
    CWaterPumpControlAdditionalWebpages::m_JsData.clear();                          //Clear Old Data
    jsonObject.printTo(CWaterPumpControlAdditionalWebpages::m_JsData);              //Write Data to Json Send Str
#ifdef Debug
    Serial.println(CWaterPumpControlAdditionalWebpages::m_JsData);
#endif
    CWebServerBasic::getInstance().getESP8266WebServer()->send(200, "text/plain", CWaterPumpControlAdditionalWebpages::m_JsData);
}

static void handleWaterLimitfetchdata()
{
    Serial.println("handleWaterLimitfetchdata()");
    DynamicJsonBuffer jsonBuffer;

    // create an object
    JsonObject &jsonObject = jsonBuffer.createObject();
    // Put Value to json
    jsonObject["waterlimitdistance"] = CWaterPumpControl::getInstance().getWaterLimitMessure();

#ifdef debug
    Serial.print("waterlimitdistance:");
    Serial.println(CWaterPumpControl::getInstance().getWaterLimitMessure());
#endif 
    //Clear Buffer
    CWaterPumpControlAdditionalWebpages::m_JsData.clear();
    
    //Write to Buffer
    jsonObject.printTo( CWaterPumpControlAdditionalWebpages::m_JsData);

    // Send json Object
    CWebServerBasic::getInstance().getESP8266WebServer()->send(200, "text/plain", CWaterPumpControlAdditionalWebpages::m_JsData);
}

static void handleRootAdditional()
{
    //Configure your rootpage here ....
    Serial.println("handleRootAdditional()");
    // CFileSystem::getInstance().listDir("/",1);

    // CWaterPumpControlAdditionalWebpages::m_html.clear();
    CWaterPumpControlAdditionalWebpages::m_html.clear();
    if (CFileSystem::getInstance().readFile("/dashboard.html", &CWaterPumpControlAdditionalWebpages::m_html))
    {
        #ifdef debug
        Serial.print("File /dashboard.html readed success: ");
        //Serial.println(CWaterPumpControlAdditionalWebpages::m_html); // Should never be executed cause the serialprintbuffer will overflow !!
        #endif
        CWebServerBasic::getInstance().getESP8266WebServer()->send(200, "text/html", CWaterPumpControlAdditionalWebpages::m_html);
    }
    else
    {
        #ifdef debug
        Serial.println("Failed reading dashboard.html");
        #endif
    }

}

static void handleChangeWaterPumpModeData()
{
    Serial.println("handleChangeWaterPumpModeData()");

    // Parameters
    int mode = getJsonObjectFromResponse()["mode"]; // 1

    switch (mode)
    {
    case 0:
        CWaterPumpControl::getInstance().changeModeToAuto();
        Serial.println("change mode by http to AUTO");
        break;

    case 1:
        CWaterPumpControl::getInstance().changeModeToManuelOn();
        Serial.println("change mode by http to ManuelON");
        break;

    case 2:
        CWaterPumpControl::getInstance().changeModeToManuelOff();
        Serial.println("change mode by http to ManuelOff");
        break;

    default:
        break;
    }
}

static void handleChangeStartDelay()
{
    int startInMinutes = getJsonObjectFromResponse()["startdelayinminutes"];
    #ifdef debug
    Serial.println("handleChangeStartDelay()");
    Serial.print("StartDelay Set to: ");
    Serial.print(startInMinutes);
    Serial.println("min");
    #endif

    CWaterPumpControl::getInstance().setTurnOnDelay(startInMinutes);

    String str(startInMinutes);
    CFileSystem::getInstance().writeFile("/startdelay.txt", &str);
}

static void handleChangeWaterLimitMax()
{
    Serial.println("handleChangeWaterLimitMax()");
    int max = getJsonObjectFromResponse()["waterlimitmax"];
    Serial.print("Max Set to: ");
    Serial.println(max);

    CWaterPumpControl::getInstance().setWaterLimitMax(max);

    String str(max);
    CFileSystem::getInstance().writeFile("/watermax.txt", &str);
}

static void handleChangeWaterLimitMin()
{
    Serial.println("handleChangeWaterLimitMin()");
    int min = getJsonObjectFromResponse()["waterlimitmin"];
    #ifdef debug
    Serial.print("Min Set to: ");
    Serial.println(min);
    #endif

    CWaterPumpControl::getInstance().setWaterLimitMin(min);

    String str(min);
    CFileSystem::getInstance().writeFile("/watermin.txt", &str);
}

static void getWaterPumpControlMode()
{
    String output;
    CWebServerBasic::getInstance().getESP8266WebServer()->send(200, "text/plain", *CWaterPumpControl::getInstance().getWaterPumpModeAsStr(&output));
}

static void handleWaterLimitBoard()
{
    Serial.println("try to send g_watermessuredashboard...");
    CWaterPumpControlAdditionalWebpages::m_html.clear();
    if (CFileSystem::getInstance().readFile("/watermeasuredashboard.html", &CWaterPumpControlAdditionalWebpages::m_html))
    {
        #ifdef debug
        Serial.print("File watermeasuredashboard.html readed success: ");
        // Serial.println(CWaterPumpControlAdditionalWebpages::m_html); // Care this will maybe overflow the serial buffer!
        #endif

        CWebServerBasic::getInstance().getESP8266WebServer()->send(200, "text/html", CWaterPumpControlAdditionalWebpages::m_html);
    }
    else
    {
        Serial.println("Failed reading html_watermeasuredashboard.html");
    }
}

void CWaterPumpControlAdditionalWebpages::setupAdditionalWebpagesAPMode()
{
    // this->getWebServerBasic()->setupAdditionalAPModeWebPages();

    ///insert new routes  here

    // this->m_pWebServerBasic->start();
}

void CWaterPumpControlAdditionalWebpages::setupAdditionalWebPageNormalMode()
{
#ifdef debug
    Serial.println("Setting up setupAdditionalWebPageNormalMode()");
#endif
    CWebServerBasic &Server = CWebServerBasic::getInstance();
    Server.getESP8266WebServer()->on("/",                    handleRootAdditional);
    Server.getESP8266WebServer()->on("/mode",                getWaterPumpControlMode);
    Server.getESP8266WebServer()->on("/fetchdata",           handleFetchDataForDashBoardAdditional);
    Server.getESP8266WebServer()->on("/waterlimit",          handleWaterLimitBoard);
    Server.getESP8266WebServer()->on("/waterlimitfetchdata", handleWaterLimitfetchdata);
    Server.getESP8266WebServer()->on("/startdelay",          handleChangeStartDelay);
    Server.getESP8266WebServer()->on("/waterlimitmax",       handleChangeWaterLimitMax);
    Server.getESP8266WebServer()->on("/waterlimitmin",       handleChangeWaterLimitMin);
    Server.getESP8266WebServer()->on("/changemode",          handleChangeWaterPumpModeData);

    ///insert new routes  here

    Server.getESP8266WebServer()->begin();
}
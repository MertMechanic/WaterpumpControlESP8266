
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


#define debug 1

//GLOBAL FUNCTIONS FOR WEBSERVER HTTP REQUESTS
//

CWaterPumpControlAdditionalWebpages::CWaterPumpControlAdditionalWebpages()
{
}
CWaterPumpControlAdditionalWebpages::~CWaterPumpControlAdditionalWebpages()
{
}


void handleFetchDataForDashBoardAdditional()
{
    Serial.println("fetchData... Dashboard");

    int StartDelay = CWaterPumpControl::getInstance().getWaterPump()->getTurnOnDelay();
    String waterPumpModeStr;

    //getting current Pumpmode and generate a String
    switch (CWaterPumpControl::getInstance().getWaterPump()->getWaterPumpMode())
    {
    case AUTO:
        waterPumpModeStr = "AUTO";
        break;
    case MANUELON:
        waterPumpModeStr = "MANUELON";
        break;
    case MANUELOFF:
        waterPumpModeStr = "MANUELOFF";
        break;
    default:
        break;
    };
    DynamicJsonBuffer jsonBuffer;

    // create an object
    JsonObject &jsonObject = jsonBuffer.createObject();
    String tmp;

    if (CWaterPumpControl::getInstance().isWaterInFountain())
    {
        jsonObject["status"] = true;
    }
    else
    {
        jsonObject["status"] = false;
    }

    jsonObject["fountainstatus"] = CWaterPumpControl::getInstance().getFountainStatus();

    if (CWaterPumpControl::getInstance().getFountainStatus() == e_FountainStatus::FILLED)
        jsonObject["fountainstatus"] = "FILLED";
    else if (CWaterPumpControl::getInstance().getFountainStatus() == e_FountainStatus::OVERFILLED)
        jsonObject["fountainstatus"] = "OVERFILLED";
    else if (CWaterPumpControl::getInstance().getFountainStatus() == e_FountainStatus::EMPTY)
        jsonObject["fountainstatus"] = "EMPTY";
    else if (CWaterPumpControl::getInstance().getFountainStatus() == e_FountainStatus::OVEREMPTY)
        jsonObject["fountainstatus"] = "OVEREMPTY";
    else if (CWaterPumpControl::getInstance().getFountainStatus() == e_FountainStatus::ERROR)
        jsonObject["fountainstatus"] = "ERROR";

    jsonObject["temperature1"] = *CWaterPumpControl::getInstance().getTemperatureSensors()->getValueByIndexAsString(0, &tmp);
    jsonObject["temperature2"] = *CWaterPumpControl::getInstance().getTemperatureSensors()->getValueByIndexAsString(1, &tmp);

    jsonObject["waterlimitdistance"] = CWaterPumpControl::getInstance().getWaterLimitMessure();
    jsonObject["waterlimitminborder"] = CWaterPumpControl::getInstance().getWaterLimitMinBorder();
    jsonObject["waterlimitmaxborder"] = CWaterPumpControl::getInstance().getWaterLimitMaxBorder();

    CTimeWaterPump *pTime = CWaterPumpControl::getInstance().getRestartTimeWithDelay();
    if (pTime != nullptr)
    {
        jsonObject["restarttimestr"] = *pTime->getAsString(&tmp);
    }
    else
    {
        jsonObject["restarttimestr"] = "-";
    }

    jsonObject["startdelay"] = String(StartDelay);

    jsonObject["runtime0"] = *CWaterPumpControl::getInstance().getSaveRunTimeReversed()->getData(0)->getAsString(&tmp);
    jsonObject["runtime1"] = *CWaterPumpControl::getInstance().getSaveRunTimeReversed()->getData(1)->getAsString(&tmp);
    jsonObject["runtime2"] = *CWaterPumpControl::getInstance().getSaveRunTimeReversed()->getData(2)->getAsString(&tmp);

    jsonObject["stoptime0"] = *CWaterPumpControl::getInstance().getStopRunTimeReversed()->getData(0)->getAsString(&tmp);
    jsonObject["stoptime1"] = *CWaterPumpControl::getInstance().getStopRunTimeReversed()->getData(1)->getAsString(&tmp);
    jsonObject["stoptime2"] = *CWaterPumpControl::getInstance().getStopRunTimeReversed()->getData(2)->getAsString(&tmp);

    jsonObject["mode"] = waterPumpModeStr;

    String output;
    jsonObject.printTo(output);
#ifdef Debug
    Serial.println(output);
#endif
    CWebServerBasic::getInstance().getESP8266WebServer()->send(200, "text/plain", output);
}

void handleWaterLimitfetchdata()
{
    DynamicJsonBuffer jsonBuffer;

    // create an object
    JsonObject &jsonObject = jsonBuffer.createObject();
    String tmp;
    jsonObject["waterlimitdistance"] = CWaterPumpControl::getInstance().getWaterLimitMessure();
    String output;
    jsonObject.printTo(output);
    CWebServerBasic::getInstance().getESP8266WebServer()->send(200, "text/plain", output);
}


void handleRootAdditional()
{
    //Configure your rootpage here ....
    CWebServerBasic::getInstance().getESP8266WebServer()->send_P(200, "text/html", html_dashboard);
}

void handleChangeWaterPumpModeData()
{

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

void handleChangeStartDelay()
{
    int startInMinutes = getJsonObjectFromResponse()["startdelayinminutes"];
    Serial.print("StartDelay Set to: ");
    Serial.print(startInMinutes);
    Serial.println("min");

    CWaterPumpControl::getInstance().setTurnOnDelay(startInMinutes);

    String str(startInMinutes);
    CFileSystem::getInstance().writeFile("/startdelay.txt", &str);
}

void handleChangeWaterLimitMax()
{
    int max = getJsonObjectFromResponse()["waterlimitmax"];
    Serial.print("Max Set to: ");
    Serial.println(max);

    CWaterPumpControl::getInstance().setWaterLimitMax(max);

    String str(max);
    CFileSystem::getInstance().writeFile("/watermax.txt", &str);
}

void handleChangeWaterLimitMin()
{
    int min = getJsonObjectFromResponse()["waterlimitmin"];
    Serial.print("Min Set to: ");
    Serial.println(min);

    CWaterPumpControl::getInstance().setWaterLimitMin(min);

    String str(min);
    CFileSystem::getInstance().writeFile("/watermin.txt", &str);
}

void getWaterPumpControlMode()
{
    String outputStr;

    switch (CWaterPumpControl::getInstance().getWaterPump()->getWaterPumpMode())
    {
    case AUTO:
        outputStr = "AUTO";
        break;
    case MANUELON:
        outputStr = "MANUELON";
        break;
    case MANUELOFF:
        outputStr = "MANUELOFF";
        break;
    default:
        break;
    };

    CWebServerBasic::getInstance().getESP8266WebServer()->send(200, "text/plain", outputStr);
}

void handleWaterLimitBoard()
{
    Serial.println("try to send g_watermessuredashboard...");
    CWebServerBasic &Server = CWebServerBasic::getInstance();
    Server.getESP8266WebServer()->send_P(200, "text/html", html_watermeasuredashboard);
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
    Server.getESP8266WebServer()->on("/"                     , handleRootAdditional);
    Server.getESP8266WebServer()->on("/mode"                 , getWaterPumpControlMode);
    Server.getESP8266WebServer()->on("/fetchdata"            , handleFetchDataForDashBoardAdditional);
    Server.getESP8266WebServer()->on("/waterlimit"           , handleWaterLimitBoard);
    Server.getESP8266WebServer()->on("/waterlimitfetchdata"  , handleWaterLimitfetchdata);
    Server.getESP8266WebServer()->on("/startdelay"           , handleChangeStartDelay);
    Server.getESP8266WebServer()->on("/waterlimitmax"        , handleChangeWaterLimitMax);
    Server.getESP8266WebServer()->on("/waterlimitmin"        , handleChangeWaterLimitMin);
    Server.getESP8266WebServer()->on("/changemode"           , handleChangeWaterPumpModeData);


    ///insert new routes  here


    Server.getESP8266WebServer()->begin();

}
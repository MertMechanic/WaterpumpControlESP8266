
#include "CWebServer.h"
#include "CWifi.h"

#include "ArduinoJson.h"
#include "FS.h"

#include "CWaterPumpControl.h"
#include "CWaterPump.h"
#include "Clcd.h"
//
//GLOBAL FUNCTIONS FOR WEBSERVER HTTP REQUESTS
//



JsonObject &getJsonObjectFromResponse()
{
    String data = CWebServer::getInstance().getESP8266WebServer()->arg("plain");

    // Parsing
    const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonObject &root = jsonBuffer.parseObject(data);

    return root;
}

void handleAPModeRootPage()
{
    Serial.println("try to send wifisetupwebpage...");
    CWebServer &Server = CWebServer::getInstance();
    Server.getESP8266WebServer()->send_P(200, "text/html", wifisetupwebpage);
}

void handleFetchDataForDashBoard()
{
    Serial.println("fetchData... Dashboard");
    
    int StartDelay = CWaterPumpControl::getInstance().getWaterPump()->getTurnOnDelay();

    String waterPumpModeStr;

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
    jsonObject["startdelay"] = String(StartDelay);
    jsonObject["mode"] = waterPumpModeStr;

    // CTimeWaterPump reverseArray[CWaterPumpControl::S_SIZEOFTIMESSAVED];
  


    String tmp;
    jsonObject["runtime0"]    = *CWaterPumpControl::getInstance().getSaveRunTimeReversed()->getData(0)->getAsString(&tmp);
    jsonObject["runtime1"]    = *CWaterPumpControl::getInstance().getSaveRunTimeReversed()->getData(1)->getAsString(&tmp);
    jsonObject["runtime2"]    = *CWaterPumpControl::getInstance().getSaveRunTimeReversed()->getData(2)->getAsString(&tmp);
    

    jsonObject["stoptime0"]    = *CWaterPumpControl::getInstance().getStopRunTimeReversed()->getData(0)->getAsString(&tmp);
    jsonObject["stoptime1"]    = *CWaterPumpControl::getInstance().getStopRunTimeReversed()->getData(1)->getAsString(&tmp);
    jsonObject["stoptime2"]    = *CWaterPumpControl::getInstance().getStopRunTimeReversed()->getData(2)->getAsString(&tmp);




    jsonObject["status"] = CWaterPumpControl::getInstance().isWaterInFountain();

//TODO
    // if (CWaterPumpControl::getInstance().getRestartTimeWithDelay() != nullptr && CWaterPumpControl::getInstance().getWaterPump()->isWaterPumpStopped())
    // {
    //     jsonObject["restarttime"] = *CWaterPumpControl::getInstance().getRestartTimeWithDelay()->getAsString(&tmp);
    // }
    // else if (CWaterPumpControl::getInstance().getRestartTimeWithDelay() != nullptr && CWaterPumpControl::getInstance().getWaterPump()->isWaterPumpRunning())
    // {
    //     jsonObject["restarttime"] = "is running";
    // }
    // else
    // {
    //     jsonObject["restarttime"] = "not defined";
    // }

    String output;
    jsonObject.printTo(output);

    CWebServer::getInstance().getESP8266WebServer()->send(200, "text/plain", output);

}


void handleAPModeSettingsUpdate()
{

    Serial.println("handleSettingsUpdate called...");
    JsonObject &JsonObject = getJsonObjectFromResponse();

    //DEBUG
    // char buffer[1024];
    // JsonObject.printTo(buffer,1024);
    // Serial.println(buffer);

    // String ssid = JsonObject.get("ssid");
    // String password = JsonObject.get("password");

    File configFile = SPIFFS.open("/config.json", "w");
    JsonObject.printTo(configFile);
    configFile.close();

    CWebServer::getInstance().getESP8266WebServer()->send(200, "application/json", "{\"status\":\"ok\"}");
    Serial.println("send status ok");
    delay(500);



    //Restart ESP after getting settings updated!
    ESP.restart();
}

void handleRoot()
{
    //Configure your rootpage here ....
    Serial.println("try to send servus moiiiin...");
    CWebServer &Server = CWebServer::getInstance();
    Server.getESP8266WebServer()->send_P(200, "text/html", g_dashboard);
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

    CWebServer::getInstance().getESP8266WebServer()->send(200, "text/plain", outputStr);
}

CWebServer::CWebServer()
{
}
CWebServer::~CWebServer()
{
}

ESP8266WebServer *CWebServer::getESP8266WebServer()
{
    return &this->m_Webserver;
}

void CWebServer::setupWebPageAPMode()
{

    this->getESP8266WebServer()->on("/", handleAPModeRootPage);
    this->getESP8266WebServer()->on("/settings", HTTP_POST, handleAPModeSettingsUpdate);

    // replay to all requests with same HTML
    this->getESP8266WebServer()->onNotFound(handleAPModeRootPage);
    this->getESP8266WebServer()->begin();
}

void CWebServer::setupWebPageNormalMode()
{
    this->getESP8266WebServer()->on("/", handleRoot);
    this->getESP8266WebServer()->on("/mode",getWaterPumpControlMode );
    this->getESP8266WebServer()->on("/fetchdata",handleFetchDataForDashBoard );

    this->getESP8266WebServer()->on("/startdelay", handleChangeStartDelay);
    this->getESP8266WebServer()->on("/changemode", handleChangeWaterPumpModeData);
     

    this->getESP8266WebServer()->begin();
}


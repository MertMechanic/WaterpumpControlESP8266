#pragma once

#include "CAdditionalWebpages.h"
#include "ESP8266WebServer.h"
// #include "CWebserverData.h"
#include "ArduinoJson.h"

#include "CWebServerBasic.h"
#include "WebServerDataBasic.h"
#include "WaterPumpControlWebServerData.h"

class CWaterPumpControlAdditionalWebpages : public CAdditionalWebpages
{

public:
    CWaterPumpControlAdditionalWebpages();
    ~CWaterPumpControlAdditionalWebpages();
    void setupAdditionalWebPageNormalMode() override;
    void setupAdditionalWebpagesAPMode() override;



    static String m_html;
    static String m_JsData;
};
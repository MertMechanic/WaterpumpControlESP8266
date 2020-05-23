#pragma once

#include "ESP8266WebServer.h"
#include "CWebserverData.h"


class CWebServer 
{
//Stingleton START
public:
    static CWebServer &getInstance()
    {
        static CWebServer S;
        return S;
    }

    CWebServer(const CWebServer &) = delete;
    void operator=(CWebServer const &) = delete;

private:
    CWebServer();
    ~CWebServer();
//Stingleton END
public:
    ESP8266WebServer *getESP8266WebServer();

    virtual void setupWebPageAPMode();
    virtual void setupWebPageNormalMode();

private:
    ESP8266WebServer m_Webserver;
    int static const m_WebserverPort = 80;
};
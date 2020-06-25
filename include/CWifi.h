#pragma once
///
///
#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined ESP32
#include <WiFi.h>
#include "SPIFFS.h"
#endif
class ESP8266WebServer;
class Clcd;

#include "CWebServer.h"
#include <DNSServer.h>

class CWifi
{

//BASIC SINGLETON START
public:
    static CWifi &getInstance()
    {
        static CWifi S(CWebServer::getInstance());
        return S;
    }

    CWifi(const CWifi &) = delete;
    void operator=(CWifi const &) = delete;

private:
    CWifi(CWebServer &_webserver);
    ~CWifi();
//BASIC SINGLETON END

public:
    String getIpAddress();
    void setIpAdress(int, int, int, int);

    void setSSID(String *_pSSID);
    void setPassword(String *_psk);

    String* getSSID();
    String* getPassword();

    bool isInAPMode();
    void wifiConnect();
    // void wifiConnect(Clcd &_lcd);
    void handleSettingsUpdate();

    CWebServer &getWebserver();

    void setResetPin(const uint8_t _pin);
    uint8_t getResetPin();

    //INIT WIFI SSID + PASSWORD
    //
    //
    const char *INIT_SSID = "WIFI ESP8266";
    const char *INIT_APPASSWORD = "12345678";
    bool m_isInAPMode;

    void init(uint8_t _resetPin);
    void initWithLCD(Clcd &_lcd, uint8_t _resetPin);
    
    bool handleResetButton();

    DNSServer* getDNSServer();
    void run();
    



private:
    String* m_pSSID;
    String* m_pPassword;
    bool m_isStaticIP;

    CWebServer& m_WebServer;

    bool m_isWIFIConnected;
    uint8_t m_WifiResetPin;

    IPAddress *m_pIP;
    IPAddress *m_pGateway;
    IPAddress *m_pSubnetmask;

    int m_resetTimeCounter;
    static const int s_limitResetCounter = 5;


    const byte DNS_PORT = 53;
    DNSServer m_dnsServer;

    Clcd *m_pLcd;
};
#include "Defines.h"
#include "CWifi.h"
#include "Arduino.h"
#include <ESP8266WiFi.h>

#include "CWebServer.h"
#include "ArduinoJson.h"

#include "FS.h"
#include <LITTLEFS.h>
#include "CFileSystem.h"
#ifndef CONFIG_LITTLEFS_FOR_IDF_3_2
 #include <time.h>
#endif 


#include "Clcd.h"

CWifi::CWifi(CWebServer &_webserver) : m_WebServer(_webserver)
{
}
CWifi::~CWifi()
{
}

CWebServer &CWifi::getWebserver()
{
    return CWebServer::getInstance();
}

void CWifi::setSSID(String *_pSSID)
{
    this->m_pSSID = _pSSID;
}
void CWifi::setPassword(String *_psk)
{
    this->m_pPassword = _psk;
}

String *CWifi::getSSID()
{
    return this->m_pSSID;
}

String *CWifi::getPassword()
{
    return this->m_pPassword;
}

void CWifi::init(const uint8_t _resetPin)
{
    this->m_pLcd = nullptr;
    this->setResetPin(_resetPin);
    this->wifiConnect();
    this->m_WifiUDP = WiFiUDP();
}

void CWifi::initWithLCD(Clcd &_lcd, uint8_t _resetPin)
{
    this->m_pLcd = &_lcd;
    this->setResetPin(_resetPin);
    this->wifiConnect();
    this->m_WifiUDP = WiFiUDP();
}

bool CWifi::isInAPMode()
{
    return this->m_isInAPMode;
}

void CWifi::setResetPin(const uint8_t _pin)
{
    pinMode(_pin, INPUT_PULLUP); // WIFI RESET PIN
    this->m_WifiResetPin = _pin;
}

uint8_t CWifi::getResetPin()
{
    return this->m_WifiResetPin;
}

String CWifi::getIpAddress()
{
    return this->m_pIP->toString();
}

void CWifi::wifiConnect()
{
    WiFi.softAPdisconnect(true);
    WiFi.disconnect();

    if (this->m_pLcd != nullptr)
    {
        String line1("Wifi Connect");
        String line2(".........");
        this->m_pLcd->setDisplayText(&line1, &line2);
    }

    //delay(1000);
    this->m_isInAPMode = false;

    const char *_ssid = "";
    const char *_pass = "";
    String str;

    if (CFileSystem::getInstance().readFile("/config.json", &str))
    {
        DynamicJsonBuffer JsonBuffer;
        JsonObject &jObject = JsonBuffer.parseObject(str);
        if (jObject.success())
        {
            _ssid = jObject["ssid"];
            _pass = jObject["password"];
            this->m_pSSID = new String(_ssid);
            this->m_pPassword = new String(_pass);

            Serial.print("Try to connect to:");
            Serial.print(_ssid);

            if (this->m_pLcd != nullptr)
            {
                String line1("Try to connect:");
                String line2(_ssid);
                this->m_pLcd->setDisplayText(&line1, &line2);
            }

#ifdef STATICIPADDRESS

            IPAddress StaticIP(STATICIPADDRESS);
            IPAddress GatewayIP(STATICGATEWAY);
            IPAddress Netmask(STATICSUBNETMASK);

            if (WiFi.config(StaticIP, GatewayIP, Netmask, GatewayIP, GatewayIP) == false)
            {
                String cfglcd1("Configuration");
                String cfglcd2("failed");
                Serial.println("Configuration failed.");
                this->m_pLcd->setDisplayText(&cfglcd1, &cfglcd2);
            }
#endif

            Serial.println(*this->m_pSSID);
            Serial.println(*this->m_pPassword);

            WiFi.mode(WIFI_STA);
            WiFi.begin(_ssid, _pass);
            this->m_isInAPMode = false;

            while (WiFi.status() != WL_CONNECTED)
            {
                delay(500);
                Serial.println(".");
                if (CWifi::handleResetButton())
                {
                    if (this->m_pLcd != nullptr)
                    {
                        String line1("Deleting");
                        String line2("Wifi Setup!");
                        this->m_pLcd->setDisplayText(&line1, &line2);
                    }

                    wifiConnect();
                    break;
                }
            }
        }
    }
    else
    {
        Serial.println("Failed to Read File");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        //Wifi is connected to config wifi setup
        this->m_pIP = new IPAddress(WiFi.localIP());
        this->m_pSubnetmask = new IPAddress(WiFi.subnetMask());
        this->m_pGateway = new IPAddress(WiFi.gatewayIP());
        this->m_pMutlicastAddress = new IPAddress(MULTICASTADDRESS);

        Serial.print("IpAddress is:");
        Serial.println(this->m_pIP->toString());
        Serial.print("SubnetMask is:");
        Serial.println(this->m_pSubnetmask->toString());
        Serial.print("Gateway is:");
        Serial.println(this->m_pGateway->toString());
        if (this->m_pLcd != nullptr)
        {

            String line1("IPAddress is:");
            String line2(this->m_pIP->toString());
            this->m_pLcd->setDisplayText(&line1, &line2);
            delay(1000);
        }
        this->m_isInAPMode = false;
    }
    else
    {
        //IMPORTANT - PASSWORD MUST BE MIN LENGHT BY 8 CHARS!
        const char *initssid = INIT_SSID;
        const char *initpassword = INIT_APPASSWORD;
        Serial.print("name:");
        Serial.println(initssid);
        Serial.print("password:");
        Serial.println(initpassword);
        Serial.println("--------");
        Serial.println("AP Mode - no config file found or created...");
        WiFi.mode(WIFI_AP);

        //Set LCD when avalible
        if (this->m_pLcd != nullptr)
        {
            String line1("AP Mode is");
            String line2("Active");
            this->m_pLcd->setDisplayText(&line1, &line2);
        }

        //Check is static IP set 
        if (this->m_pIP == nullptr || this->m_pSubnetmask == nullptr || this->m_pGateway == nullptr)
        {
            this->m_isStaticIP = true;
            WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
        }
        else
        {
            this->m_isStaticIP = false;
            WiFi.softAPConfig(*this->m_pIP, *this->m_pSubnetmask, *this->m_pGateway);
        }

        //AP with passwordphrase
        // WiFi.softAP(initssid, initpassword, 1, 0);

        WiFi.softAP(initssid);

        WiFi.setAutoReconnect(true);

        // if DNSServer is started with "*" for domain name, it will reply with
        // provided IP to all DNS request
        this->m_dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        this->m_dnsServer.start(DNS_PORT, "*", IPAddress(192,168,1,1));

        this->m_isInAPMode = true;
    }

    WiFi.printDiag(Serial);



    if(this->isInAPMode())
    {
        this->getWebserver().setupWebPageAPMode();
    }
    else
    {
        this->getWebserver().setupWebPageNormalMode();
    }
    
}


DNSServer* CWifi::getDNSServer()
{
    return &this->m_dnsServer;
}

//Main Method of WIFI - 
//handle client
//is dns when in AP mode
//handle reset button for resetting WIFI settings....
void CWifi::run()
{
    if (this->m_isInAPMode == true)
    {
        CWifi::getInstance().getDNSServer()->processNextRequest();
    }
    
    CWebServer::getInstance().getESP8266WebServer()->handleClient(); //Method for http request handleing

    CWifi::getInstance().handleResetButton();                        //Method for observing WIFI reset button 
    
}

//Listen to the Reset Button ... Interrupt everything and count 5 seconds 
//if 5 seconds is finished - the wifi settings are resetete by deleteing the config file... 
// Return false if Reset Routine is not finished 
bool CWifi::handleResetButton()
{

    int count = 5;
    bool buttonPressed = false;
    //Count 5 times is button pressed - if yes - reset settings 
    for (int i = 0; i < count; i++)
    {
        if (digitalRead(this->getResetPin()) == HIGH)
        {
            if (this->m_pLcd != nullptr)
            {
                String line1("WifiConfigReset");
                String line2(5-i);
                this->m_pLcd->setDisplayText(&line1, &line2);
            }

            buttonPressed = true;
            Serial.print(5 - i);
            Serial.print("...");
        }
        else
        {
            buttonPressed = false;
            break;
        }
        delay(1000);
    }

    if (buttonPressed)
    {
        Serial.println("Deleting WIFI SETUP");
        if (this->m_pLcd != nullptr)
        {
            String line1("Deleting Wifi");
            String line2("Setup");
            this->m_pLcd->setDisplayText(&line1, &line2);
        }

        if (this->m_isInAPMode == false && this->m_pIP != nullptr && this->m_pSubnetmask != nullptr && this->m_pGateway != nullptr)
        {
            delete this->m_pIP;
            delete this->m_pSubnetmask;
            delete this->m_pGateway;
        }
        

        this->m_isInAPMode = true;
        String str;
        if (CFileSystem::getInstance().readFile("/config.json", &str))
        {
            CFileSystem::getInstance().deleteFile("/config.json");
            Serial.println("Remove Config file...");
            this->wifiConnect();
        }
        return true;
    }

    
    return false;  
}



void CWifi::sendUDPMultiCast(String* pString) 
{

  Serial.print("sendUDP : ");
  Serial.println(*pString);

  // convert string to char array
  char msg[255];
  pString->toCharArray(msg,255);

  this->m_WifiUDP.beginPacketMulticast(*this->m_pMutlicastAddress, MULTICASTPORT, WiFi.localIP());
  this->m_WifiUDP.write(msg);
  this->m_WifiUDP.endPacket();
}
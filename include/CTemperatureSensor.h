#pragma once

#include <DallasTemperature.h>

/**
 * @brief Usage: call init in your setup method once with the count of Sensor
 *        init method needs countofSensors and the readpin of the OneWireBusInput - Care that you need to set the Pinmode Before use !
 *        pinMode(BUSINPUT_PIN, INPUT);
 *
 *        In your while(1) Method you can call updateTemperature() for updating all  sensor Values and check  if there is any error (maybe a wire is disconnected)
 *        After updateing you need to call getValueByIndex to get the value of a specific sensor
 *        The value 999,99 indicates a error !   
 *        Also care you hava a 4,7k resistor between vcc 3,3V and the data wire of the bus!
 * 
 */
class TemperatureSensor
{
public:
    TemperatureSensor();
    ~TemperatureSensor();

    void init(int _countOfSensors, int _BusInputPIN);
    //Return index if failed - if ok == -1
    int updateTemperature();
    
    float getValueByIndex(int _index);
    String* getValueByIndexAsString(int _index, String* _pDestination);

private:
    const float No_Val = 999.99;
    OneWire             m_oneWire;
    DallasTemperature   m_DS18B20;
    DeviceAddress       m_DS18B20_Adressen;
    int                 m_countOfSensors;
    static const int    m_DS18B20_Aufloesung = 12;
    float*              m_pValues;



};

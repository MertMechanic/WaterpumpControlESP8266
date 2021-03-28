#include "cTemperatureSensor.h"


TemperatureSensor::TemperatureSensor()
{
}

TemperatureSensor::~TemperatureSensor()
{
    if(this->m_pValues != nullptr)
    {
         delete[] this->m_pValues;
    }
}

/**
 * @brief Init Method to for _countofSensor on _BusInput Bin via onewire
 * 
 * @param _countOfSensors 
 * @param _BusInputPIN 
 */
void TemperatureSensor::init(int _countOfSensors, int _BusInputPIN)
{
    this->m_countOfSensors = _countOfSensors;

    m_oneWire = OneWire(_BusInputPIN);

    m_DS18B20 = DallasTemperature(&m_oneWire);

    if (this->m_countOfSensors > 0)
    {
        m_DS18B20.begin();
        Serial.print("Count of connected Sensors: ");
        Serial.println(m_DS18B20.getDeviceCount(), DEC);
        Serial.println();

        for (byte i = 0; i < m_DS18B20.getDeviceCount(); i++)
        {
            if (m_DS18B20.getAddress(m_DS18B20_Adressen, i))
            {
                m_DS18B20.setResolution(m_DS18B20_Adressen, m_DS18B20_Aufloesung);
            }
        }

        if (this->m_pValues == nullptr)
        {
            this->m_pValues = new float[this->m_countOfSensors];

            for (size_t i = 0; i < this->m_countOfSensors; i++)
            {
                this->m_pValues[i] = No_Val;
            }
        }
        else
        {
            delete[] this->m_pValues;
            Serial.print("WARNING: YOU CALLED INIT! ACTUALLY REINIT IN RUNTIME IS NOT NESSESARY");
            this->m_pValues = new float[this->m_countOfSensors];

            for (size_t i = 0; i < this->m_countOfSensors; i++)
            {
                this->m_pValues[i] = No_Val;
            }
        }
    }
    Serial.println("Init finished");
}

/**
 * @brief fill m_pValues with messure Values and return false if there is an error reported
 *        each sensor has his own hardware id so you need to care about it in your application 
 * 
 * @return index of Failed Sensor if -1 everything is fine
 */
int TemperatureSensor::updateTemperature()
{

    m_DS18B20.requestTemperatures();
    int indexOfFailedSensor = -1; 
    for (byte i = 0; i < this->m_countOfSensors; i++)
    {
        if (i < m_DS18B20.getDeviceCount())
        {
            // Serial.print("Sensor ");
            // Serial.print(i + 1);
            // Serial.print(": ");

            m_pValues[i] = m_DS18B20.getTempCByIndex(i);
            if (m_pValues[i] == DEVICE_DISCONNECTED_C)
            {
                m_pValues[i] = No_Val;
                // Serial.println("Fehler");
                indexOfFailedSensor = i;
            }
            else
            {
                Serial.print(m_pValues[i]);
                Serial.println(" 'C");
                
            }
        }
    }
    return indexOfFailedSensor;
}

float TemperatureSensor::getValueByIndex(int _index)
{
    if (_index < this->m_countOfSensors)
    {
        return this->m_pValues[_index];
    }
    else
    {
        return this->No_Val;
    }
}

String* TemperatureSensor::getValueByIndexAsString(int _index, String* _pDestination)
{
    *_pDestination = String(this->getValueByIndex(_index));

    *_pDestination += String(" °C");

    return _pDestination;
}
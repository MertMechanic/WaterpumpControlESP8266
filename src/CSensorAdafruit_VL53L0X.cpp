#include "CSensorAdafruit_VL53L0X.h"
#include "Adafruit_VL53L0X.h"

VL53L0X_RangingMeasurementData_t CSensorAdafruit_VL53L0X::m_Measure;
const int   CSensorAdafruit_VL53L0X::s_maxFailedMessures;
bool        CSensorAdafruit_VL53L0X::m_MeasureStatusOK = false;
bool        CSensorAdafruit_VL53L0X::m_DummyFlag       = true;
int         CSensorAdafruit_VL53L0X::m_MesureValue;
int         CSensorAdafruit_VL53L0X::m_counterOfFailedMessures;
int         CSensorAdafruit_VL53L0X::m_MiddleValues[CSensorAdafruit_VL53L0X::s_MiddleValueArraySize];
int         CSensorAdafruit_VL53L0X::m_CurrentBufferPosition;
int         s_MAX_RANGE;
Adafruit_VL53L0X CSensorAdafruit_VL53L0X::m_lox;

CSensorAdafruit_VL53L0X::~CSensorAdafruit_VL53L0X()
{
#ifdef debug
    Serial.println("Adafruit_VL53L0X Destructor Called");
#endif
}

/**
 * @brief Init the Sensor
 * 
 * Return Codes:
 *               1 - Successfull
 *              -1 - Failed to boot VL530LX
 * 
 * @return int 
 */
int CSensorAdafruit_VL53L0X::initLongRange()
{

    m_MeasureStatusOK = true;

#ifdef debug
    Serial.println("Adafruit_VL53L0X Init Long Range Called");
#endif

    int max_restartCounter = 10;
    int counter = 0;
    
    if(!CSensorAdafruit_VL53L0X::m_lox.begin())
    {
        return -1;
    }
    else
    {
        CSensorAdafruit_VL53L0X::m_lox.configSensor(Adafruit_VL53L0X::VL53L0X_SENSE_LONG_RANGE);
        CSensorAdafruit_VL53L0X::initMiddleValueArray();
        return 0;
    }
}

bool CSensorAdafruit_VL53L0X::doMeasure()
{
    int CurrentMeasure = 0;
    CSensorAdafruit_VL53L0X::m_lox.rangingTest(&CSensorAdafruit_VL53L0X::m_Measure, false); // pass in 'true' to get debug data printout!
    if (CSensorAdafruit_VL53L0X::m_Measure.RangeStatus != 4)
    { 

        if (CSensorAdafruit_VL53L0X::m_Measure.RangeStatus <= 1)
        {
            CSensorAdafruit_VL53L0X::m_counterOfFailedMessures++;
        }
        else
        {
            CSensorAdafruit_VL53L0X::m_counterOfFailedMessures = 0;
            CurrentMeasure = (int)CSensorAdafruit_VL53L0X::m_Measure.RangeMilliMeter;
            if (CurrentMeasure < s_MAX_RANGE)
            {
                CSensorAdafruit_VL53L0X::m_MesureValue = CurrentMeasure;
                CSensorAdafruit_VL53L0X::addNewValueToMiddleValueArray();
            }
            else
            {
                CSensorAdafruit_VL53L0X::m_counterOfFailedMessures++;
            }
        }

        if (CSensorAdafruit_VL53L0X::m_counterOfFailedMessures == CSensorAdafruit_VL53L0X::s_maxFailedMessures)
        {
            Serial.println("Messure - FAILED!!!!!");
            CSensorAdafruit_VL53L0X::m_MeasureStatusOK = false;
            CSensorAdafruit_VL53L0X::m_counterOfFailedMessures = 0;
            CSensorAdafruit_VL53L0X::initMiddleValueArray();
        }

        return true;
    }
    else
    {
        // Serial.println(" out of range ");
    }
    return false;
}


bool CSensorAdafruit_VL53L0X::doDummyMeasure()
{

    /**
     * @brief Check Value is on Min or max border
     * 
     */
    if (CSensorAdafruit_VL53L0X::m_MesureValue == -1)
    {
        CSensorAdafruit_VL53L0X::m_DummyFlag = true; //Counting upwards
    }
    else if (CSensorAdafruit_VL53L0X::m_MesureValue == CSensorAdafruit_VL53L0X::s_MAX_RANGE)
    {
        CSensorAdafruit_VL53L0X::m_DummyFlag = false; //Counting downwards
    }
    
    /**
     * @brief Count to direction
     * 
     */
    if (CSensorAdafruit_VL53L0X::m_DummyFlag)
    {
        CSensorAdafruit_VL53L0X::m_MesureValue++;  //UP
    }
    else
    {
        CSensorAdafruit_VL53L0X::m_MesureValue--; //DOWN
    }

    CSensorAdafruit_VL53L0X::addNewValueToMiddleValueArray();
    return true;

}

int CSensorAdafruit_VL53L0X::getMeasureValue()
{
    return CSensorAdafruit_VL53L0X::m_MesureValue;
}
bool CSensorAdafruit_VL53L0X::isMeasureOk()
{
    return CSensorAdafruit_VL53L0X::m_MeasureStatusOK;
}

int CSensorAdafruit_VL53L0X::doMeasureAndGetValue()
{
    if (!CSensorAdafruit_VL53L0X::doMeasure())
    {
        return -1;
    }
    return CSensorAdafruit_VL53L0X::getMeasureValue();
}

void CSensorAdafruit_VL53L0X::initMiddleValueArray()
{
    for (size_t i = 0; i < CSensorAdafruit_VL53L0X::s_MiddleValueArraySize; i++)
    {
        CSensorAdafruit_VL53L0X::m_MiddleValues[i] = 0;
    }
    CSensorAdafruit_VL53L0X::m_CurrentBufferPosition = 0;
}

void CSensorAdafruit_VL53L0X::addNewValueToMiddleValueArray()
{
    if (CSensorAdafruit_VL53L0X::m_CurrentBufferPosition > CSensorAdafruit_VL53L0X::s_MiddleValueArraySize)
    {
        CSensorAdafruit_VL53L0X::m_CurrentBufferPosition = 0;
    }

    CSensorAdafruit_VL53L0X::m_MiddleValues[CSensorAdafruit_VL53L0X::m_CurrentBufferPosition] = CSensorAdafruit_VL53L0X::getMeasureValue();

    CSensorAdafruit_VL53L0X::m_CurrentBufferPosition++;
}

int CSensorAdafruit_VL53L0X::getMiddleValue()
{
    int sum = 0;
    for (size_t i = 0; i < CSensorAdafruit_VL53L0X::s_MiddleValueArraySize; i++)
    {
        sum += CSensorAdafruit_VL53L0X::m_MiddleValues[i];
    }
    int result = sum / CSensorAdafruit_VL53L0X::s_MiddleValueArraySize;
    
    if (result <= 0 || result > s_MAX_RANGE)
    {
        return -1;
    }

    return result;
    
}


void CSensorAdafruit_VL53L0X::printSerial()
{
    Serial.print(CSensorAdafruit_VL53L0X::getMiddleValue());
    Serial.print("-");
    Serial.println(CSensorAdafruit_VL53L0X::m_CurrentBufferPosition);
    //delay(250);
}


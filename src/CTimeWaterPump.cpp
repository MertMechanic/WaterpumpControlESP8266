#include "CTimeWaterPump.h"

#include "Arduino.h"

CTimeWaterPump::CTimeWaterPump(int _hour, int _minute, int _second):
m_hour(_hour),
m_minute(_minute),
m_second(_second)
{
}

CTimeWaterPump::CTimeWaterPump():
m_hour(0),
m_minute(0),
m_second(0)
{
}
CTimeWaterPump::~CTimeWaterPump()
{
}

int CTimeWaterPump::getHour()
{
    return this->m_hour;
}


int CTimeWaterPump::getMinute()
{
    return this->m_minute;
}


int CTimeWaterPump::getSecond()
{
    return this->m_second;
}

 void CTimeWaterPump::addMinutes(int _turnOnDelayInMinutes)
 {
     //Define Time in Minutes....
     unsigned int hoursInMinutes = this->m_hour * 60;
     unsigned int minutesComplete = hoursInMinutes + this->m_minute + _turnOnDelayInMinutes;

     unsigned int restMinutes = minutesComplete % 60;

     unsigned int hours = minutesComplete / 60;

     this->m_hour = hours;
     this->m_minute = restMinutes;
 }

String& CTimeWaterPump::toIntAndAddPreZero(String& _destination, int _value)
{
    String str(_value);
    if (_value < 10)
    {
        str = "0" + str;
    }
    _destination = str;
    return _destination;
}

String* CTimeWaterPump::getAsString(String *_pDestination)
{
    String hour;
    String minute;
    String second;

    hour    = this->toIntAndAddPreZero(hour, this->m_hour);
    minute  = this->toIntAndAddPreZero(minute, this->m_minute);
    second  = this->toIntAndAddPreZero(second, this->m_second);

    String complete = hour + ":" + minute + ":" + second;

    *_pDestination = complete;

    return _pDestination;
}
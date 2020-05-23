
class String;

class CTimeWaterPump
{
public:
    CTimeWaterPump(int _hour, int _minute, int _second);
    CTimeWaterPump();
    ~CTimeWaterPump();

    int getHour();
    int getMinute();
    int getSecond();

    void addMinutes(int _turnOnDelayInMinutes);
    String* getAsString(String *_pDestination);
    
    bool operator<(const CTimeWaterPump& _other)
    {
        if (this->m_hour <= _other.m_hour && this->m_minute <= _other.m_minute && this->m_second <= _other.m_second)
        {
            return true;
        }
        return false;
    }
    bool operator>(const CTimeWaterPump& _other)
    {
        if (this->m_hour >= _other.m_hour && this->m_minute >= _other.m_minute && this->m_second >= _other.m_second)
        {
            return true;
        }
        return false;
    }


private:
    int m_hour;
    int m_minute;
    int m_second;

    String &toIntAndAddPreZero(String &_destination, int _value);
};
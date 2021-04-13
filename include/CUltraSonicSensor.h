#pragma once 

class CUltraSonicSensor
{
    public:
        CUltraSonicSensor();
        ~CUltraSonicSensor();


        static int S_TriggerPin;
        static int S_EchoPin;

    public:
        static int getDistanceInCM();
        static int initUltraSonicSensorPins(int _triggerPin, int _echoPin);

};

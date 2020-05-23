#pragma once 

#include "Arduino.h"

class C2RelayModule{



    private: 
        uint8_t m_ConnectorPinA;
        uint8_t m_ConnectorPinB;
        
    public:
        C2RelayModule(uint8_t _a, uint8_t _b);

        void TurnOnAllRelay();
        void TurnOffAllRelay();
        void TurnOnRelayA();
        void TurnOffRelayA();
        void TurnOnRelayB();
        void TurnOffRelayB();
        void Test(int _intervall);
};
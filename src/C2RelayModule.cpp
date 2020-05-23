
#include "C2RelayModule.h"
#include "Arduino.h"

C2RelayModule::C2RelayModule(uint8_t _a, uint8_t _b)
{
    this->m_ConnectorPinA = _a;
    this->m_ConnectorPinB = _b;

}

void C2RelayModule::TurnOnAllRelay()
{
    this->TurnOnRelayA();
    this->TurnOnRelayB();
}
void C2RelayModule::TurnOffAllRelay()
{
    this->TurnOffRelayA();
    this->TurnOffRelayB();
}
void C2RelayModule::TurnOnRelayA()
{
    digitalWrite(this->m_ConnectorPinA, LOW);
}
void C2RelayModule::TurnOffRelayA()
{
    digitalWrite(this->m_ConnectorPinA, HIGH);
}
void C2RelayModule::TurnOnRelayB()
{
    digitalWrite(this->m_ConnectorPinB, LOW);
}
void C2RelayModule::TurnOffRelayB()
{
    digitalWrite(this->m_ConnectorPinB, HIGH);
}

void C2RelayModule::Test(int _intervall)
{
    this->TurnOffAllRelay();
    //Relais A ON - OFF
    delay(_intervall);
    this->TurnOnRelayA();
    delay(_intervall);
    this->TurnOffRelayA();
    
    delay(2*_intervall);

    //Relais B ON - OFF
    this->TurnOnRelayB();
    delay(_intervall);
    this->TurnOffRelayB();
    delay(2*_intervall);

    //ALL ON - OFF
    this->TurnOnAllRelay();
    delay(_intervall);
    this->TurnOffAllRelay();
}
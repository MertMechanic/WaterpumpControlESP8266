#include "CWaterPump.h"
#include "C2RelayModule.h"


CWaterPump::CWaterPump()
{

}

CWaterPump::CWaterPump(C2RelayModule *_pRelaysModule, int _turnOnDelay, WaterPumpModeType _waterPumpMode, bool _isRunning):
m_turnOnDelay(_turnOnDelay),
m_WaterPumpmode(_waterPumpMode),
m_isRunning(_isRunning)
{
    this->m_pRelayModule = _pRelaysModule;
}

void CWaterPump::TurnOnWaterPump()
{
    this->m_isRunning = true;
    this->m_pRelayModule->TurnOnAllRelay();
    
    
}
void CWaterPump::TurnOffWaterPump()
{
    this->m_isRunning = false;
    this->m_pRelayModule->TurnOffAllRelay();
    
}
void CWaterPump::setTurnONDelay(int _delayInSeconds)
{
    this->m_turnOnDelay = _delayInSeconds;
}
void CWaterPump::setWaterPumpMode(WaterPumpModeType _newMode)
{ 
    this->m_WaterPumpmode = _newMode;
}
WaterPumpModeType CWaterPump::getWaterPumpMode()
{
    return this->m_WaterPumpmode;
}
int CWaterPump::getTurnOnDelay()
{
    return this->m_turnOnDelay;
}
C2RelayModule* CWaterPump::getRelaysModule()
{
    return this->m_pRelayModule;
}
void CWaterPump::runTestRelaysModuleWithDelayOf(int _delay)
{
    this->m_pRelayModule->Test(_delay);
}


bool CWaterPump::isWaterPumpRunning()
{
    return this->m_isRunning;
}
bool CWaterPump::isWaterPumpStopped()
{
    return !this->m_isRunning;
}

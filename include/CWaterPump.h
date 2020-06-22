#pragma once
#include "Defines.h"

class C2RelayModule;

class CWaterPump
{
    public:
        static CWaterPump& getInstance()
        {
            static CWaterPump    instance; // Guaranteed to be destroyed.
                                  // Instantiated on first use.
            return instance;
        }

        CWaterPump();
        CWaterPump(C2RelayModule *_pRelaysModule, int _turnOnDelay, WaterPumpModeType _waterPumpMode, bool _isRunning);
        void TurnOnWaterPump();
        void TurnOffWaterPump();
        
        bool isWaterPumpRunning();
        bool isWaterPumpStopped();

        void setWaterPumpMode(WaterPumpModeType _newMode);

        WaterPumpModeType getWaterPumpMode();
        int getTurnOnDelay();

        C2RelayModule* getRelaysModule();

        void runTestRelaysModuleWithDelayOf(int _delay);
        void setTurnONDelay(int _delayInMinutes);
        

    private:
        bool m_isRunning;
        int m_turnOnDelay;
        
        WaterPumpModeType m_WaterPumpmode;
        C2RelayModule* m_pRelayModule;
};
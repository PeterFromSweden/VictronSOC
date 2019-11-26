#ifndef VICTRON_BMV_H_
#define VICTRON_BMV_H_

#include <Arduino.h>
#include "VictronEnergyDirect.h"

class VictronBMV {
  public:
    bool connected = false;
    uint16_t relaySOCLow      = 0; // 0.1%
    uint16_t relaySOCLowClear = 0; // 0.1%
    uint16_t SOC = 0;              // 0.1%
    
    VictronBMV(VictronEnergyDirect& ved);
    bool ping();
    bool configChange();
    bool getSOC();

  private:
    VictronEnergyDirect* ved;
};

#endif

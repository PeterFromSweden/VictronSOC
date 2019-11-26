#ifndef VICTRON_MPPT_H_
#define VICTRON_MPPT_H_

#include <Arduino.h>
#include "VictronEnergyDirect.h"

class VictronMPPT {
  public:
    bool connected = false;
    bool chargeActivated    = false;
    bool everseen = false;
    
    VictronMPPT(VictronEnergyDirect& ved);
    bool ping();
    bool enableCharge(bool enable);

  private:
    bool activateRemoteControl();
    VictronEnergyDirect* ved;
};

#endif

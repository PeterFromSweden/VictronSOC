#include <stdio.h>
#include <string.h>
#include "VictronBMV.h"

VictronBMV::VictronBMV(VictronEnergyDirect& ved)
{
  this->ved = &ved;
}

bool VictronBMV::ping()
{
  return ved->ping();
}

bool VictronBMV::configChange()
{
  TVEDValue relaySOCLow;
  TVEDValue relaySOCLowClear;
  
  if( !ved->get(VEDID_RELLOWSOC, &relaySOCLow) ) {
    return false;
  }
  
  if( !ved->get(VEDID_RELLOWSOCCLR, &relaySOCLowClear) ) {
    return false;
  }
    
  if( relaySOCLowClear.u16 == 0 ) {
    Serial.println("config read, preserving old.");
    return false;
  }
  
  if( relaySOCLowClear.u16 < 30 ) { // 3.0 %
    // Config > 0 but < 3% => Deactivate function
    if( this->relaySOCLow == 0 ) {
      Serial.println("config read, already deactivated.");
      return false;
    }
    Serial.println("config read, deactivating.");
    relaySOCLow.u16 = 0;
    relaySOCLowClear.u16 = 0;
  }

  if( (this->relaySOCLow == relaySOCLow.u16) &&
      (this->relaySOCLowClear == relaySOCLowClear.u16) )
  {
    Serial.println("config read, identical.");
    return false;
  }
  
  this->relaySOCLow = relaySOCLow.u16;
  this->relaySOCLowClear = relaySOCLowClear.u16;

  return true;
}

bool VictronBMV::getSOC()
{
  TVEDValue SOC;

  if( !ved->get(VEDID_SOC, &SOC) ) {
    Serial.println("soc *not* read.");
    return false;
  }

  this->SOC = SOC.u16;
  
  return true;
}

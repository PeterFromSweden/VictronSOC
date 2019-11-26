#include <stdio.h>
#include <string.h>
#include "VictronMPPT.h"

VictronMPPT::VictronMPPT(VictronEnergyDirect& ved)
{
  this->ved = &ved;
}

bool VictronMPPT::ping()
{
  bool res = ved->ping();
  
  if( res ) {
    everseen = true;
  }
  
  return res;
}

bool VictronMPPT::enableCharge(bool enable)
{
  TVEDValue deviceMode;
  
  if( !activateRemoteControl() ) {
    return false;
  }
  
  if( !ved->get(VEDID_DEVICEMODE, &deviceMode) ) {
    Serial.println("device mode *not* read.");
    return false;
  }

  if( deviceMode.u8 == 1 ) {
    // Charger on, disable?
    if( !enable ) {
      deviceMode.u8 = 0;
      if( !ved->set(VEDID_DEVICEMODE, &deviceMode) ) {
        Serial.println("device mode *not* stopping charge.");
        return false;
      }
      Serial.println("device mode stopped charge.");
    }
    else {
      Serial.println("device mode charge already on");
    }
  }
  else {
    // Charger off, enable?
    if( enable ) {
      deviceMode.u8 = 1;
      if( !ved->set(VEDID_DEVICEMODE, &deviceMode) ) {
        Serial.println("device mode *not* starting charge.");
        return false;
      }
      Serial.println("device mode started charge.");
    }
    else {
      Serial.println("device mode charge already off");
    }
  }

  return true;
}

bool VictronMPPT::activateRemoteControl()
{
  TVEDValue remoteControl;
  
  if( !ved->get(VEDID_REMOTECONTROL, &remoteControl) ) {
    Serial.println("remoteControl *not* read.");
    return false;
  }

  //Serial.print("remoteControl ");
  //Serial.println(ved->tostring(remoteControl));
  
  if( remoteControl.u32 & 0x0002 ) {
    //Serial.println("remoteControl already activated.");
  }
  else
  {
    remoteControl.u32 |= 0x0002;
    if( !ved->set(VEDID_REMOTECONTROL, &remoteControl) ) {
      Serial.println("remoteControl *not* activated.");
      return false;
    }
    
    Serial.println("remoteControl activated.");
  }

  return true;
}

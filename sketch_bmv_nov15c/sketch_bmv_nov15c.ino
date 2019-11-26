#include <string.h>
#include <SoftwareSerial.h>
#include "LowPower.h" // https://github.com/rocketscream/Low-Power
#include "EepromData.h"
#include "SerialMessenger.h" // Debug
#include "VictronEnergyDirect.h"
#include "VictronBMV.h"
#include "VictronMPPT.h"

EepromData eeprom;
SoftwareSerial bmvSerial(12,7);   // rx,tx port
SoftwareSerial mppt1Serial(11,8); // rx,tx port
SoftwareSerial mppt2Serial(10,9); // rx,tx port

SerialMessenger bmvSM(bmvSerial);
SerialMessenger mppt1SM(mppt1Serial);
SerialMessenger mppt2SM(mppt2Serial);
SerialMessenger debugSM(Serial);

VictronEnergyDirect vedBmv(bmvSM);
VictronEnergyDirect vedMppt1(mppt1SM);
VictronEnergyDirect vedMppt2(mppt2SM);

VictronBMV bmv(vedBmv);
VictronMPPT mppt1(vedMppt1);
VictronMPPT mppt2(vedMppt2);

bool setCharge = true;
bool chargeEnable = true;  

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(19200);
  vedBmv.begin();
  vedMppt1.begin();
  vedMppt2.begin();
  vedMppt2.listen(false);
  
  if(!eeprom.read()) {
    blink(5,3); // Eeprom read error, xxxxx xxxxx xxxxx
    eeprom.setRelaySOCLow(bmv.relaySOCLow);
    eeprom.setRelaySOCLowClear(bmv.relaySOCLowClear);
    //eeprom.defaultData();
    //eeprom.write();
  }
  else {
    bmv.relaySOCLow = eeprom.getRelaySOCLow();
    bmv.relaySOCLowClear = eeprom.getRelaySOCLowClear();
  }
}

void loop() {

  /*
    TVEDValue value;
    //vedBmv.scanText();
    if(vedBmv.ping()) {
      value.dataType = VEDDR_STR20;
      strncpy(value.str, "Peter", 20);
      vedBmv.set(VEDID_DESCRIPTION, &value);
      vedBmv.get(VEDID_DESCRIPTION, &value);
      //delay(800);
      //value.u8 = 0;
      //vedBmv.set(VEDID_BACKLIGHT, &value);
    }
    //first = false;
    while(Serial.available() == 0)
      ;
    while(Serial.available() > 0)
      Serial.read();
  */
  
  //vedBmv.ping();
  //bmv.ping();
  //bmvSM.readChars(5000);
  //bmvSM.readLines(5000);
  //mppt1SM.readLines(5000);
  //return;
  // put your main code here, to run repeatedly:
  TVEDValue value;

  if( bmv.ping() ) {
    // Scan for configuration change
    if( bmv.configChange() ) {
      eeprom.setRelaySOCLow(bmv.relaySOCLow);
      eeprom.setRelaySOCLowClear(bmv.relaySOCLowClear);
      eeprom.write();
      if( bmv.relaySOCLowClear != 0 )
      {
        sprintf(value.str, "MPPT OFF%2u%% ON%2u%%", bmv.relaySOCLowClear / 10, bmv.relaySOCLow / 10);
      }
      else
      {
        strcpy(value.str, "MPPT SOC deactivated");
      }
      value.dataType = VEDDR_STR20;
      vedBmv.set(VEDID_DESCRIPTION, &value);
    }
    
    /*    
     * This cannot be used... It syncs battery => 100 %
    TVEDValue value;
    value.u16 = 500; // 5.00 %
    vedBmv.set(VEDID_SOC, &value);
    */
    
    // Get SOC and compare to limits
    if( bmv.getSOC() ) {
      //Serial.println(bmv.SOC);
      if( bmv.SOC == 65535 ) { // Unsynched
        Serial.println("Unsynched");
        bmv.SOC = 0; // Assume charge is needed. Fully charged will synch and then stop
      }
      
      setCharge = true;
      // SOC is in 0.01% and Relay SOC is in 0.1%
      if( bmv.relaySOCLowClear ) {
        if( bmv.SOC >= bmv.relaySOCLowClear * 10 ) {
          chargeEnable = false;
        }
        else if( bmv.SOC < bmv.relaySOCLow * 10) {
          chargeEnable = true;
        }
        else {
          // Hysteresis
        }
      }
      else {
        chargeEnable = true;
      }
    }
    else
    {
      Serial.println("bmv.getSOC fail");
    }

    if( mppt1.ping() ) {
      if( setCharge ) {
        if( !mppt1.enableCharge(chargeEnable) ) {
          Serial.println("mppt1.enableCharge fail");
        }
      }
    }
    else 
    {
      Serial.println("MPPT1 ping fail");
      blink(3,3); // MPPT1 ping fail xxx xxx xxx
    }

    if( mppt2.ping() ) {
      if( setCharge ) {
        if( !mppt2.enableCharge(chargeEnable) ) {
          Serial.println("mppt2.enableCharge fail");
        }
      }
    }
    else 
    {
      if( mppt2.everseen ) {
        Serial.println("MPPT2 ping fail");
        blink(4,3); // MPPT2 ping fail xxxx xxxx xxxx
      }
    }
  }
  else 
  {
    Serial.println("bmv.ping fail");
    blink(2,3); // BMV ping fail xx xx xx
  }

endfun:
# if 0
  while(Serial.available() == 0)
    ;
  while(Serial.available() > 0)
    Serial.read();
# else
  // Enter power down state for 8 s with ADC and BOD module disabled
  Serial.end();
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);  
  Serial.begin(19200);
# endif
}

//void dummy()
//{
//}

void blink(uint8_t reps, uint8_t sets)
{
  const uint16_t repdelay = 200;
  const uint16_t setdelay = 800;

  for( uint8_t set = 0; set < sets; set++ ) {
    for( uint8_t rep = 0; rep < reps; rep++) {
        digitalWrite(LED_BUILTIN, true);
        delay(repdelay);
        digitalWrite(LED_BUILTIN, false);
        delay(repdelay);
    }
    delay(setdelay);
  }
}

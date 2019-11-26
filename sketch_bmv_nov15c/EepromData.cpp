#include <Arduino.h>
#include <EEPROM.h>
#include "EepromData.h"


//
// Constructor
//
EepromData::EepromData()
{
  memset(&content, 0, sizeof(content));
}

bool EepromData::read()
{
  uint8_t* c = (uint8_t *) &content;
  
  //Serial.println("EEPROM read:");
  for(uint8_t ix = 0; ix < sizeof(EepromContentT); ix++) {
    *c = EEPROM[ix];
    //Serial.print(*c, HEX);
    //Serial.print(' ');
    c++;
  }
  //Serial.println();
  
  if( content.version > EEPROM_CONTENT_VERSION ) {
    Serial.println("EEPROM: Future or garbage");
    // Future or garbage
    return false;
  }
  
  if( crc() != content.crc ) {
    Serial.print("EEPROM: Crc ");
    Serial.println(crc(), HEX);
    return false;
  }
  
  return true;
}

bool EepromData::write()
{
  content.version = EEPROM_CONTENT_VERSION;
  content.crc = crc();
  uint8_t* c = (uint8_t *) &content;
  Serial.println("EEPROM write:");
  for(uint8_t ix = 0; ix < sizeof(EepromContentT); ix++) {
    EEPROM[ix] = *c;
    Serial.print(*c, HEX);
    Serial.print(' ');
    c++;
  }
  Serial.println();
}

void EepromData::defaultData()
{
  setRelaySOCLow(0);
  setRelaySOCLowClear(0);
}

uint32_t EepromData::crc()
{
  const uint32_t crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  uint32_t crc = ~0L;
  uint8_t* c = (uint8_t *) &content;
  
  for(uint8_t ix = 0; ix < sizeof(EepromContentT) - 4; ix++) {
    crc = crc_table[(crc ^ c[ix]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (c[ix] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;    
  }

  return crc;  
}

#ifndef EEPROM_DATA_H_
#define EEPROM_DATA_H_

typedef struct {
  uint8_t version;
  uint16_t relaySOCLow;
  uint16_t relaySOCLowClear;
  uint32_t crc;
} EepromContentT_V1;

#define EEPROM_CONTENT_VERSION 1
typedef EepromContentT_V1 EepromContentT;

class EepromData {
  public:
    EepromData();
    bool read();
    bool write();
    void defaultData();
    uint16_t getRelaySOCLow() { return content.relaySOCLow; };
    uint16_t getRelaySOCLowClear() { return content.relaySOCLowClear; };
    void setRelaySOCLow(uint16_t value) { content.relaySOCLow = value; };
    void setRelaySOCLowClear(uint16_t value) { content.relaySOCLowClear = value; };

  private:
    uint32_t crc();
    EepromContentT content;
};

#endif

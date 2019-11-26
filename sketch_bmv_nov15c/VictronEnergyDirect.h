#ifndef VICTRON_ENERGY_DIRECT_H_
#define VICTRON_ENERGY_DIRECT_H_

#include <Arduino.h>
#include "SerialMessenger.h"

#define MAX_CMD_LINE_LEN (1+1+2+2+2+32*2+1+1) // =74
#define MAX_CMD_BYTE_LEN (MAX_CMD_LINE_LEN / 2 + 1)

typedef enum {
  VEDCMD_NONE,     // 0
  VEDCMD_PING,     // 1
  VEDCMD_RESERVED1,// 2
  VEDCMD_APPVER,   // 3
  VEDCMD_PRODID,   // 4
  VEDCMD_RESERVED2,// 5
  VEDCMD_RESTART,  // 6
  VEDCMD_GET,      // 7
  VEDCMD_SET,      // 8
  VEDCMD_RESERVED3,// 9
  VEDCMD_ASYNC,    // A
  VEDCMD_RESERVED4,// B
  VEDCMD_RESERVED5,// C
  VEDCMD_RESERVED6,// E
  VEDCMD_RESERVED7,// E
  VEDCMD_RESERVED8,// F
} TVEDCmd;

typedef enum {
  VEDID_SOC          = 0x0FFF,
  VEDID_BATTCAP      = 0x1000,
  VEDID_RELLOWSOC    = 0x1008,
  VEDID_RELLOWSOCCLR = 0x1009,
  VEDID_BACKLIGHT    = 0x0400, // Cannot be used to turned on over remote
  VEDID_DESCRIPTION  = 0x010C, // VEDDR_STR20
  VEDID_DEVICEMODE   = 0x0200,
  VEDID_DEVICESTATE  = 0x0201,
  VEDID_REMOTECONTROL= 0x0202, // Bit 1 = Enable remote control
} TVEDId;

typedef enum {
  VEDRSP_NONE,        // 0x00
  VEDRSP_RESERVED1,   // 0x01
  VEDRSP_NOTSUPPORTED,// 0x02
  VEDRSP_RESERVED2,   // 0x03
  VEDRSP_PARAMERR,    // 0x04
} TVEDResponse;

typedef enum {
  VEDDT_NONE,
  VEDDT_U32,
  VEDDT_S32,
  VEDDT_U24, // Value 32 bits
  VEDDT_S24,
  VEDDT_U16,
  VEDDT_S16,
  VEDDT_U8,
  VEDDT_S8,
  VEDDR_STR32,
  VEDDR_STR20,
} TVEDDataType;

typedef struct {
  TVEDDataType dataType;
  union {
    uint32_t u32;
    int32_t  s32;
    uint16_t u16;
    int16_t  s16;
    uint8_t  u8;
    char str[32];
  };
} TVEDValue;

typedef enum {
  VEDDS_NOTCHARGING = 0,
  VEDDS_FAULT       = 2,
  VEDDS_BULK        = 3,
  VEDDS_ABSORBTION  = 4,
  VEDDS_FLOAT       = 5,
  VEDDS_MANUALEQ    = 7,
  VEDDS_WAKEUP      = 245,
  VEDDS_AUTOEQ      = 247,
  VEDDS_EXTCTRL     = 252,
  VEDDS_UNAVAIL     = 255,
} TVEDDeviceState;

class VictronEnergyDirect {
  public:
    VictronEnergyDirect(SerialMessenger& serial);
    void begin()  {  serial->begin(19200); };
    void listen(bool enable) { serial->listen(enable); };
    bool scanText();
    bool scanForLabel(const char* label, char* o_value);
    bool ping();
    bool get(TVEDId vedid, TVEDValue* o_value);
    bool set(TVEDId vedid, const TVEDValue* value);
    String tostring(const TVEDValue& value);
    SerialMessenger* serial;

  private:
    void writeAscii(const char* buf, uint16_t buflen);
    void buildCmd(char* io_cmd, uint8_t *o_len, uint8_t *o_cmdByte);
    void sendCmd(const char* cmd, uint8_t len);
    uint8_t readCmd(char* buf, uint8_t len);
    uint8_t validateCmd(const char* buf, uint8_t len, uint8_t cmdByte, uint8_t* o_byteBuf);
    uint8_t cmd(char* io_cmd, uint16_t cmdBufLen, uint8_t* o_byteBuf, uint16_t initialTimeout);
};

#endif

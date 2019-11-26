#include <stdio.h>
#include <string.h>
#include "VictronEnergyDirect.h"


static void hexStrToByteBuf(const char* hexStr, uint8_t len, uint8_t* o_buf);
static uint8_t getCks(const uint8_t* buf, uint8_t len);
static bool verifyCks(const uint8_t* buf, uint8_t len);

VictronEnergyDirect::VictronEnergyDirect(SerialMessenger& serialMessenger)
{
  serial = &serialMessenger;
}

void VictronEnergyDirect::writeAscii(const char* buf, uint16_t buflen)
{
  char c = 0;
  
  for( uint16_t i = 0; i < buflen; i++ )
  {
    char c = buf[i];
    if( isprint(c) || c == '\t' || c == '\r' || c == '\n' ) {
      Serial.print(c);
    }
    else {
      char tmp[5];
      sprintf(tmp, "{%02x}", (uint8_t) c);
      Serial.print(tmp);
    }
  }

  if( c != '\n' ) {
    Serial.println();
  }
}

bool VictronEnergyDirect::scanText()
{
  char buf[240];
  uint16_t len;
  
  len  = serial->readBuf(buf,  sizeof(buf));
  writeAscii(buf,  len);
  Serial.println();
}

bool VictronEnergyDirect::scanForLabel(const char* label, char* o_value)
{
  char line[32];

  if( serial->readLine(line, sizeof(line)) ) {
    if( strncmp(label, line, strlen(label)) == 0 ) {
      char* c = strchr(line, '\t');
      if( c ) {
        strncpy(o_value, c+1, 32);
      }
      else {
        c = strchr(line, ' ');
        char* cprev = NULL;
        while(c)
        {
          cprev = c;
          c = strchr(cprev + 1, ' ');
        }

        if( cprev ) {
          strncpy(o_value, cprev+1, 32);
        }
      }
    }
    return true;
  }

  *o_value = '\0';

  return false;
}

bool VictronEnergyDirect::ping()
{
  char cmdBuf[MAX_CMD_LINE_LEN];
  uint8_t cmdLen;
  uint8_t cmdByte;
  uint8_t byteBuf[MAX_CMD_BYTE_LEN];

  strcpy(cmdBuf, ":1");
  if( !cmd(cmdBuf, sizeof(cmdBuf), byteBuf, 1000) ) {
    return false;
  }

  return true;
  /*
  // Build command
  strcpy(cmdBuf, ":1");
  buildCmd(cmdBuf, &cmdLen, &cmdByte);
    
  // Wait for pause...
  serial->listen(true);
  serial->waitForNoRx(20);
  if( !serial->waitForRx(1000) ) {
    Serial.println("Ping no traffic fail");
    return false;
  }
  serial->waitForNoRx(20);

  // Send command
  sendCmd(cmdBuf, cmdLen);

  // Read command reply
  uint8_t respLen;
  
  respLen = serial->readBufCmd(cmdBuf, sizeof(cmdBuf));

  if( respLen == 0 )
  {
    Serial.println("Ping readCmd fail");
    return false;
  }
  else if( respLen == sizeof(cmdBuf) )
  {
    Serial.println("Ping respLen full");
  }
  else if( respLen > cmdLen + 5 )
  {
    Serial.println("Ping respLen long");
  }
  
  
  uint8_t byteBuf[MAX_CMD_BYTE_LEN];
  if( !validateCmd(cmdBuf, respLen, cmdByte, byteBuf) ) {
    Serial.println("Ping validateCmd fail");
  }

  // Wait for possible asynch messages to pass.
  serial->waitForNoRx(10);
  
  return true; // OK
  */
}

bool VictronEnergyDirect::get(TVEDId vedid, TVEDValue* o_value)
{
  char line[MAX_CMD_LINE_LEN];
  uint8_t buf[MAX_CMD_BYTE_LEN];
  uint8_t len;

  sprintf(line, ":7%02X%02X00", vedid & 0xFF, (vedid & 0xFF00) >> 8);
  len = cmd(line, sizeof(line), buf, 0);
  if( len == 0 )
  {
    o_value->dataType = VEDDT_NONE;
    memset(o_value->str, 0, sizeof(o_value->str));
    return false; // Fail
  }
  
  switch(vedid) {
    case VEDID_DEVICEMODE:
    case VEDID_DEVICESTATE:
    case VEDID_BACKLIGHT:
      o_value->dataType = VEDDT_U8;
      memcpy(&o_value->u8, &buf[4], 1);
      break;
          
    case VEDID_BATTCAP:
    case VEDID_RELLOWSOC:
    case VEDID_RELLOWSOCCLR:
    case VEDID_SOC:
      o_value->dataType = VEDDT_U16;
      memcpy(&o_value->u16, &buf[4], 2);
      break;
      
    case VEDID_REMOTECONTROL:
      o_value->dataType = VEDDT_U32;
      memcpy(&o_value->u32, &buf[4], 4);
      break;

    case VEDID_DESCRIPTION:
      o_value->dataType = VEDDR_STR20;
      hexStrToByteBuf((char*) &buf[4], 20, (uint8_t*) o_value->str);
      break;
      
    default:
      Serial.println("ved: Invalid command byte");
      break;
  }
  
  return true;
}

bool VictronEnergyDirect::set(TVEDId vedid, const TVEDValue* value)
{
  char line[MAX_CMD_LINE_LEN];
  char* tmp;
  uint8_t i = 0;

  sprintf(line, ":8%02X%02X00", vedid & 0xFF, (vedid & 0xFF00) >> 8);
  tmp = &line[8];
  switch(value->dataType) {
    case VEDDT_U8:
      sprintf(tmp, "%02X", value->u8);
      break;

    case VEDDT_U16:
      sprintf(tmp, "%02X%02X", 
        (value->u16 & 0x00FF) >> 0, 
        (value->u16 & 0xFF00) >> 8);
      break;

    case VEDDT_U32:
      sprintf(tmp, "%02X%02X%02X%02X", 
        (value->u32 & 0x000000FF) >> 0, 
        (value->u32 & 0x0000FF00) >> 8,
        (value->u32 & 0x00FF0000) >> 16, 
        (value->u32 & 0xFF000000) >> 24);
      break;

    case VEDDR_STR20:
      while( (i < 20) && 
             (value->str[i]) ) 
      {
        sprintf(&tmp[2 * i], "%02X", value->str[i]);
        i++;
      }
      if( i < 20 ) {
        sprintf(&tmp[2 * i], "%02X", value->str[i]);
      }
      break;
      
    case VEDDR_STR32:
      while( (i < 32) && 
             (value->str[i]) ) 
      {
        sprintf(&tmp[2 * i], "%02X", value->str[i]);
        i++;
      }
      if( i < 32 ) {
        sprintf(&tmp[2 * i], "%02X", value->str[i]);
      }
      break;
      
    default:
      sprintf(tmp, "NOOOOO");
      break;
  }
  
  uint8_t buf[MAX_CMD_BYTE_LEN];
  if( cmd(line, sizeof(line), buf, 0) == 0)
  {
    Serial.println("ved: set failed");
    return false; // Fail
  }
  
  // Async message following
  //serial->readBuf((char*) buf, sizeof(buf));
  
  return true;
}

String VictronEnergyDirect::tostring(const TVEDValue& value)
{
  char tmpStr[12];
  switch( value.dataType ) {
    case VEDDT_U8:
      sprintf(tmpStr, "%i(:U8)", value.u8);
      break;
    
    case VEDDT_U16:
      sprintf(tmpStr, "%i(:U16)", value.u16);
      break;
    
    case VEDDT_U32:
      sprintf(tmpStr, "%i(:U32)", value.u32);
      break;

    default:
      return String("tostring - Unknown datatype");
  }
  return String(tmpStr);
}

//////////// Private /////////////

//
// buildCmd
// Adds checksum and newline to io_comand, sets length and command byte
//
void VictronEnergyDirect::buildCmd(char* io_cmd, uint8_t *o_len, uint8_t *o_cmdByte)
{
  uint8_t byteBuf[MAX_CMD_BYTE_LEN];
  size_t len;
  
  len = strlen(io_cmd);
  hexStrToByteBuf(io_cmd, len, byteBuf);
  *o_cmdByte = byteBuf[0];
  uint8_t tmpCks = getCks(byteBuf, len / 2);
  
  char* tmp = &io_cmd[len];
  sprintf(tmp, "%02X\n", (uint8_t) (0x55u - tmpCks));

  *o_len = len + 3; // cks hex + newline
}

void VictronEnergyDirect::sendCmd(const char* cmd, uint8_t len)
{
  serial->listen(true);
  serial->write(cmd, len);
}

uint8_t VictronEnergyDirect::readCmd(char* buf, uint8_t len)
{
  serial->listen(true);
  len = serial->readBuf(buf, len);
  if( !len ) {
    Serial.println("Timeout");
    return 0; // Timeout
  }
  return len;
}

uint8_t VictronEnergyDirect::validateCmd(const char* buf, uint8_t buflen, uint8_t cmdByte, uint8_t* o_byteBuf)
{
  if( buf[0] != ':' ) {
    Serial.println("ved: Not a command");
    writeAscii(buf, buflen);
    return 0;
  }

  hexStrToByteBuf(buf, buflen, o_byteBuf);

  if( !verifyCks(o_byteBuf, buflen / 2) ) {
    Serial.println("ved: Invalid checksum");
    writeAscii(buf, buflen);
    return 0;
  }

  // Sent command to expected reply translation
  switch( cmdByte ) {
    case 1:
      cmdByte = 5;
      break;
    
    case 3:
      cmdByte = 1;
      break;
    
    case 4:
      cmdByte = 1;
      break;
    
    default:
      break;
  }

  if( o_byteBuf[0] != cmdByte ) {
    Serial.println("ved: Invalid command byte");
    writeAscii(buf, buflen);
    return 0;
  }
  
  return buflen/2;
}

uint8_t VictronEnergyDirect::cmd(char* io_cmd, uint16_t cmdBufLen, uint8_t* o_byteBuf, uint16_t initialTimeout)
{
  uint8_t cmdLen;
  uint8_t cmdByte;
  
  serial->listen(true);

  // Build command
  buildCmd(io_cmd, &cmdLen, &cmdByte);
    
  // Initial scan for data, used for ping message to switch from text barf to hex messaging.
  if( initialTimeout ) {
    // MPPT:
    // After Text barf, few asynch hex messages may occur after 20 ms
    // After ping reply, LOTS of asynch messages may occur back to back.
    serial->waitForNoRx(20);
    if( !serial->waitForRx(initialTimeout) ) {
      Serial.println("Ping no traffic fail");
      return false;
    }
    serial->waitForNoRx(50);
  }
  
  // Send command
  sendCmd(io_cmd, cmdLen);

  // Read command reply
  uint8_t respLen;
  
  respLen = serial->readBufCmd(io_cmd, cmdBufLen);
  if( respLen == 0 )
  {
    Serial.println("cmd respLen 0");
    return 0;
  }
  else if( respLen == cmdBufLen )
  {
    writeAscii(io_cmd, respLen);
    Serial.println("cmd respLen full");
    Serial.println(respLen);
    Serial.println(cmdBufLen);
  }
  else if( respLen > cmdLen + 4 * 2 )
  {
    writeAscii(io_cmd, respLen);
    Serial.println("cmd respLen long");
  }
  
  if( !validateCmd(io_cmd, respLen, cmdByte, o_byteBuf) ) {
    Serial.println("cmd validateCmd fail");
    return 0;
  }

  // Wait for possible asynch messages to pass.
  serial->waitForNoRx(10);
  
  return respLen; // OK
}

/*
uint8_t VictronEnergyDirect::cmd(const char* cmdNoCks, uint8_t* o_byteBuf)
{
  char line[32];
  uint8_t len;
  uint8_t cmdByte;

  // First set checksum in cmd message
  len = strlen(cmdNoCks);
  hexStrToByteBuf(cmdNoCks, len, o_byteBuf);
  cmdByte = o_byteBuf[0];
  sprintf(line, "%s%02X\n", cmdNoCks, (uint8_t) (0x55u - getCks(o_byteBuf, len / 2)));

  serial->listen(true);
  serial->write(line, len + 3);
  len = serial->readBuf(line, sizeof(line));
  if( !len ) {
    Serial.println("Timeout");
    return 0; // Timeout
  }
  
  hexStrToByteBuf(line, len, o_byteBuf);
  
  if( !verifyCks(o_byteBuf, len/2) ) {
    Serial.println("ved: Invalid checksum");
    return 0; // Invalid checksum
  }
  
  // Sent command to expected reply translation
  switch( cmdByte ) {
    case 1:
      cmdByte = 5;
      break;
    
    case 3:
      cmdByte = 1;
      break;
    
    case 4:
      cmdByte = 1;
      break;
    
    default:
      break;
  }

  if( o_byteBuf[0] != cmdByte ) {
    Serial.println("ved: Invalid command byte");
    return 0; // Invalid command byte
  }
  
  return len/2;
}
*/

//////////// Static /////////////
static void hexStrToByteBuf(const char* hexStr, uint8_t len, uint8_t* o_buf)
{
  bool highNibble = true;
  uint8_t val;

  for (uint8_t ix = 0; ix < len; ix++) {
    char c = tolower(hexStr[ix]);
    uint8_t v = 0;

    if (isxdigit(c)) {
      if (c <= '9') {
        v = c - '0';
      }
      else {
        v = c - 'a' + 10;
      }
    }

    if (highNibble) {
      val = v * 16;
      highNibble = false;
    }
    else {
      o_buf[ix / 2] = val + v;
      highNibble = true;
    }
  }
}

static uint8_t getCks(const uint8_t* buf, uint8_t len)
{
  uint8_t cks = 0;

  for (uint8_t ix = 0; ix < len; ix++) {
    cks += buf[ix];
  }

  return cks;
}

static bool verifyCks(const uint8_t* buf, uint8_t len)
{
  if (getCks(buf, len) == 0x55) {
    return true;
  }

  return false;
}

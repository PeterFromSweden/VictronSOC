#include  "SerialMessenger.h"

//
// Constructor
//
SerialMessenger::SerialMessenger( HardwareSerial& device) :
  hwserial(&device),
  stream((Stream*) &device)
{
  listen(false);
}

SerialMessenger::SerialMessenger( SoftwareSerial& device) :
  swserial(&device),
  stream((Stream*) &device)
{
  listen(false);
}

//
// listen
//
void SerialMessenger::listen(bool enable)
{
  if( swserial ) {
    if(enable)
      swserial->listen();
    else
      swserial->stopListening();
  }
}

//
// available
//
int SerialMessenger::available()
{
  if( swserial ) {
    return swserial->available();
  }
  else {
    return hwserial->available();
  }
}

//
// begin
//
void SerialMessenger::begin(uint32_t baudRate)
{
  if( hwserial ) {
    hwserial->begin(baudRate);
  }
  else {
    swserial->begin(baudRate);
  }
}

//
// read
//
/*
int SerialMessenger::read()
{
  // Make sure to:
  //listen();
  
  int i = 0;
  uint32_t start = millis();
  while(!i && (millis() - start < readTimeout)) {
    i = stream->available();
  }

  if( i ) {
    i = stream->read();
  }
  else {
    i = -1; // Timeout
  }
  
  // No dump due to risk of losing chars in input stream
  return i;
}
*/

void SerialMessenger::writeAscii(const char* buf, uint16_t buflen)
{
  for( uint16_t i = 0; i < buflen; i++ )
  {
    char c = buf[i];
    if( isprint(c) || c == '\t' || c == '\r' || c == '\n' ) {
      write(c);
    }
    else {
      char tmp[5];
      sprintf(tmp, "{%02x}", (uint8_t) c);
      write(tmp);
    }
  }
}

//
// waitForRx
//
// waits for no received data. Data is cleared!
// Requires previous call to listen
//
// returns after no received data until timeout
//
void SerialMessenger::waitForNoRx(uint16_t timeout)
{
  uint32_t start = millis();
  int i;
  
  while( (uint16_t) (millis() - start) < timeout ) {
    i = stream->read();
    if( i >= 0 ) {
      start = millis();
    }
  }
}

//
// waitForRx
//
// waits for received data. Data is stored in queue.
// Requires previous call to listen
//
// returns 
//      true  : as soon data is received
//      false : timeout (no data)
//
bool SerialMessenger::waitForRx(uint16_t timeout)
{
  uint32_t start = millis();
  int available = stream->available();
  
  while( !available &&
         ((uint16_t) (millis() - start) < timeout) ) 
  {
    available = stream->available();
  }
  
  if( !available ) {
    return false;
  }
  
  return true;
}

int SerialMessenger::readBuf(char* buf, uint16_t buflen)
{
  listen(true);

  // Wait for no char to arrive
  waitForNoRx(20);
  
  // Wait for any char to arrive
  if( !waitForRx(500) ) {
    listen(false);
    Serial.println("Timeout");
    
    return 0; // Chars read
  }
  
  // Capture chars until timeout
  char* bufp = buf;
  char* bufend = buf + buflen;
  int i = 0;
  uint32_t start = millis();
  while( (bufp < bufend) &&
         (millis() - start < 20) ) 
  {
    i = stream->read();
    if( i >= 0 ) {
      start = millis();
      *bufp++ = (char) i;
    }
  }
  //listen(false);

  if( bufp < bufend) {
  }
  else {
    Serial.println("{FULL}");
  }

  return bufp-buf;
}

int SerialMessenger::readBufCmd(char* buf, uint16_t buflen)
{
  listen(true);

  // Wait for any char to arrive
  if( !waitForRx(200) ) {
    //listen(false);
    Serial.println("SM:readBufCmd Timeout");
    return 0; // Chars read
  }
  
  // Capture chars until '\n'
  char* bufp = buf;
  char* bufend = buf + buflen;
  int i = 0;
  while( (bufp < bufend) &&
         (i != 0x0A) ) 
  {
    i = stream->read();
    if( i >= 0 ) {
      *bufp++ = (char) i;
    }
  }
  //listen(false);

  return bufp-buf;
}

//
// readLine
//
bool SerialMessenger::readLine(char* lineBuf, uint8_t bufLen)
{
  int i = 0;
  uint8_t ix = 0;
  bool done = false;
  uint8_t lineEndingIx = 0;
  
  listen(true);
  while( (i >= 0) && 
         (ix < bufLen) &&
         !done)
  {
    i = stream->read();
    
    if( i >= 0 ) {
      char c = (char) i;
      
      if( c == lineEnding[lineEndingIx] ) {
        lineEndingIx++;
      }

      if( lineEndingIx == 0 ) {
        lineBuf[ix++] = c;
      }
      else if ( lineEndingIx == lineEndingChars ) {
        lineBuf[ix] = '\0';
        done = true;
      }
    }
  }
  //listen(false);

  if( done ) {
  }
  else if( i < 0 ) {
    Serial.println(" TIMEOUT");
  }
  else {
    Serial.println(" FULL");
  }

  return done;
}

/////////////////////////////////////////////////////////////////////
// Write methods
void SerialMessenger::write(char c)
{
  stream->write(c);
}

void SerialMessenger::write(const char* buf, uint8_t bufLen)
{
  uint8_t ix = 0; 
  
  while( ix < bufLen ) {
    write(buf[ix++]);
  }
}

void SerialMessenger::write(const char* lineBuf)
{
  write(lineBuf, strlen(lineBuf));
}


void SerialMessenger::writeLine(const char* lineBuf, uint8_t bufLen)
{
  write(lineBuf, bufLen);
  
  for(uint8_t ix = 0; ix < lineEndingChars; ix++) {
    write(lineEnding[ix]);
  }
}

void SerialMessenger::writeLine(const char* lineBuf)
{
  writeLine(lineBuf, strlen(lineBuf));
}

void SerialMessenger::writeLine(String lineBuf)
{
  writeLine(lineBuf.c_str());
}


/////////////////////////////////////////////////////////////////////
// Dump methods
void SerialMessenger::readLines(uint32_t ms)
{
}

void SerialMessenger::readChars(uint32_t ms)
{
}

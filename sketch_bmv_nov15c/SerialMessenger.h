#ifndef SERIALMESSENGER_H_
#define SERIALMESSENGER_H_

#include <Arduino.h>
#include <SoftwareSerial.h>

#define MESSAGE_LENGTH 64

class SerialMessenger {
  public:
    //SerialMessenger* readDump = NULL;
    //SerialMessenger* writeDump = NULL;
    //char name[7] = ""; // for dump
    
    SerialMessenger( HardwareSerial& device);
    SerialMessenger( SoftwareSerial& device);
    void setTimeout(uint16_t timeout) { readTimeout = timeout; };
    uint16_t getTimeout() { return readTimeout; };
    void begin(uint32_t baudRate);
    void listen(bool enable);
    int available();
    int readBuf(char* buf, uint16_t buflen);
    int readBufCmd(char* buf, uint16_t buflen);
    bool readLine(char* lineBuf, uint8_t bufLen);
    void readLines(uint32_t ms);
    void readChars(uint32_t ms);
    
    void write(char c);
    void write(const char* lineBuf, uint8_t bufLen);
    void write(const char* lineBuf);
    void writeLine(const char* lineBuf);
    void writeLine(String lineBuf);
    void writeLine(const char* lineBuf, uint8_t bufLen);
    void waitForNoRx(uint16_t timeout);
    bool waitForRx(uint16_t timeout);

    //int read();
    Stream* stream;

  private:
    uint16_t readTimeout = UINT16_MAX;
    HardwareSerial* hwserial;
    SoftwareSerial* swserial;
    char lineEnding[3] = "\r\n";
    char lineEndingChars = 2;

    void writeAscii(const char* buf, uint16_t buflen);
};

#endif

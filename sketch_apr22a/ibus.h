#ifndef ibus_H_
#define ibus_H_

#include <Arduino.h>
 
#define noChannels 18
#define messageLength 32
#define startByte  0x20


class IBUS {
  public:             
    IBUS(HardwareSerial* uart, unsigned long baud) {_ibusRX = uart; uart->begin(baud, SERIAL_8N1);buffer[0] = 0x20;};
    int readBuffer();
    int processChannels();
    int update();
    uint16_t channels[14];
    uint16_t getChannel(uint8_t i) { return channels[i]; }

  private:
    HardwareSerial* _ibusRX;
    uint8_t buffer[32];

};
#endif
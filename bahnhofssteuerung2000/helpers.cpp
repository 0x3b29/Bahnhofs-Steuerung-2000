#include "helpers.h"
#include "eeprom.h"
uint8_t getBoardIndexForChannel(uint16_t channel) { return channel / 16; }

uint8_t getBoardAddressForChannel(uint16_t channel) {

  uint8_t boardAddress = 0x40 + getBoardIndexForChannel(channel);

  // In case our calculated address is equal to or bigger to the EEPROMS
  // address, we need to add 1 because we cant use the EEPROM address twice on
  // the bus. Since the boards base address is 0x40, there are 16 available
  // addresses until we reach 0x50 Board 1 .. 16 have regular Board 17 .. 61
  // have adjusted addresses
  if (boardAddress >= EEPROM_ADDRESS) {
    boardAddress++;
  }

  return boardAddress;
}

uint8_t getBoardSubAddressForChannel(uint16_t channel) { return channel % 16; }

float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void floatToBuffer(float value, char *buffer, size_t bufferSize, uint8_t precision) {
  if (bufferSize == 0) {
    return;
  }

  // Handle special cases: NaN and infinity
  if (isnan(value)) {
    snprintf(buffer, bufferSize, "NaN");
    return;
  }

  if (isinf(value)) {
    snprintf(buffer, bufferSize, "Inf");
    return;
  }

  // Handle the integer and fractional parts
  int intPart = (int)value;                    
  float fracPart = fabs(value - intPart);      
  unsigned int fracAsInt = (unsigned int)(fracPart * pow(10, precision));

  // Format into the buffer
  snprintf(buffer, bufferSize, "%d.%0*u", intPart, precision, fracAsInt);
}

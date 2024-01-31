#ifndef helpers_h
#define helpers_h

#include "main.h"
#include <Arduino.h>

uint8_t getBoardIndexForChannel(uint16_t channel);
uint8_t getBoardAddressForChannel(uint16_t channel);
uint8_t getBoardSubAddressForChannel(uint16_t channel);

#endif // helpers_h
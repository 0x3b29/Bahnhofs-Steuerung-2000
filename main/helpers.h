#ifndef helpers_h
#define helpers_h

#include "main.h"
#include <Arduino.h>

uint8_t getBoardIndexForChannel(uint16_t channel);
uint8_t getBoardAddressForChannel(uint16_t channel);
uint8_t getBoardSubAddressForChannel(uint16_t channel);

float mapf(float x, float in_min, float in_max, float out_min, float out_max);

#endif // helpers_h
#ifndef helpers_h
#define helpers_h

#include "main.h"
#include <Arduino.h>

int getBoardIndexForChannel(int channel);
int getBoardAddressForChannel(int channel);
int getBoardSubAddressForChannel(int channel);

#endif // helpers_h
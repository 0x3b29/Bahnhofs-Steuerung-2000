#include "helpers.h"

int getBoardIndexForChannel(int channel) { return channel / 16; }

int getBoardAddressForChannel(int channel) {
  return 0x40 + getBoardIndexForChannel(channel);
}

int getBoardSubAddressForChannel(int channel) { return channel % 16; }

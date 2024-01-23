#ifndef render_h
#define render_h

#include "main.h"
#include <Arduino.h>
#include <WiFiNINA.h>

#define pt client.print
#define pn client.println

void renderWebPage(WiFiClient client, bool foundRecursion,
                   bool renderWithOptionsVisible,
                   bool renderWithEditChannelVisible, bool renderAnchor,
                   uint16_t anchorChannelId, uint16_t numChannels,
                   bool toggleOneBasedAddresses, bool toggleForceAllOff,
                   bool toggleForceAllOn, bool toggleRandom,
                   uint16_t channelIdToEdit,
                   char channelNameBuffer[MAX_CHANNEL_NAME_LENGTH]);

#endif // render_h
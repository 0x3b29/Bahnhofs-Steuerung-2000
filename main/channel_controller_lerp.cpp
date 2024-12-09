#include "channel_controller.h"
#include "eeprom.h"
#include "helpers.h"
#include "server_controller.h"

void ChannelController::addChannelToCurrentlyLerpingList(uint16_t channelId) {
  // Check if the channelId is already in the list
  for (uint16_t i = 0; i < m_currentlyLerpingChannelCount; i++) {
    if (m_currenltyLerpingChannels[i] == channelId)
      return;
  }

  // Add the channelId to the list
  if (m_currentlyLerpingChannelCount < MAX_LERPING_CHANNELS) {
    m_currenltyLerpingChannels[m_currentlyLerpingChannelCount++] = channelId;
  }
}

void ChannelController::removeChannelFromCurrentlyLerpingList(
    uint16_t channelId) {
  for (uint16_t i = 0; i < m_currentlyLerpingChannelCount; i++) {
    if (m_currenltyLerpingChannels[i] == channelId) {
      // Remove the channelId by shifting the remaining elements
      for (uint16_t j = i; j < m_currentlyLerpingChannelCount - 1; j++) {
        m_currenltyLerpingChannels[j] = m_currenltyLerpingChannels[j + 1];
      }
      m_currentlyLerpingChannelCount--;
      return;
    }
  }
}

float ChannelController::updateLerpingChannel(
    uint16_t channelId, uint16_t deltaTimeInMilliseconds) {
  // Read current position and target position
  float currentValue =
      readFloatForChannelFromEepromBuffer(channelId, MEM_SLOT_LERP_CURRENT_POS);
  float targetValue = (float)readUint16tForChannelFromEepromBuffer(
      channelId, MEM_SLOT_LERP_TARGET_VALUE);
  float lerpSpeed = readFloatForChannelFromEepromBuffer(
      channelId, MEM_SLOT_LERP_SPEED); // Time to complete lerp in seconds

  uint16_t value1 = 0;
  uint16_t value2 =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_OUTPUT_VALUE2);

  bool usesValue1 = readBoolForChannelFromEepromBuffer(
      channelId, MEM_SLOT_USES_OUTPUT_VALUE1);

  if (usesValue1) {
    value1 = readUint16tForChannelFromEepromBuffer(channelId,
                                                   MEM_SLOT_OUTPUT_VALUE1);
  }

  // Dynamically determine the range (position1 to position2)
  float position1 = (float)value1;
  float position2 = (float)value2;
  float range = abs(position2 - position1);

  float deltaTimeInSeconds = deltaTimeInMilliseconds / (float)1000;
  float deltaValue = lerpSpeed * deltaTimeInSeconds;

  // Determine the direction of movement
  if (currentValue < targetValue) {
    currentValue =
        min(currentValue + deltaValue, targetValue); // Increment towards target
  } else {
    currentValue =
        max(currentValue - deltaValue, targetValue); // Decrement towards target
  }

  // Write the new value back to memory
  writeFloatForChannelToEepromBuffer(channelId, MEM_SLOT_LERP_CURRENT_POS,
                                     currentValue);

  // Send value to controller
  int boardIndex = getBoardIndexForChannel(channelId);
  int subAddress = getBoardSubAddressForChannel(channelId);
  uint16_t pwmValue = (uint16_t)round(currentValue);
  this->m_pwmBoards[boardIndex].setPWM(subAddress, 0, pwmValue);

  // Return the remaining delat to go
  return abs(currentValue - targetValue);
}

void ChannelController::lerpLoopEvent(uint16_t deltaTimeInMilliseconds) {
  for (uint16_t i = 0; i < m_currentlyLerpingChannelCount; i++) {
    float remainingDelta = updateLerpingChannel(m_currenltyLerpingChannels[i],
                                                deltaTimeInMilliseconds);

    if (remainingDelta < 0.01) {
      removeChannelFromCurrentlyLerpingList(m_currenltyLerpingChannels[i]);
      // Recheck from current index after removing currentl position
      i--;
    }
  }
}
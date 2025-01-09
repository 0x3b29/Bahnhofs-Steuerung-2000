#include "channel_controller.h"
#include "eeprom.h"
#include "helpers.h"
#include "server_controller.h"

void ChannelController::addChannelToCurrentlyLerpingList(uint16_t channelId) {
  // Check if the channelId is already in the list
  for (uint16_t i = 0; i < m_currentlyLerpingChannelCount; i++) {
    if (m_currenltyLerpingChannels[i] == channelId) {
      if (SHOW_DEBUG_INFO) {
        st("Info: Channel ");
        st(channelId);
        sn(" was already in list of lerping channels");
      }

      // Bail out if channel is already in list of lerping channels
      return;
    }
  }

  if (hasChannelArrivedAtTarget(channelId)) {
    // Bail out if channel is already at or close to its target position
    if (SHOW_DEBUG_INFO) {
      st("Fail: Channel ");
      st(channelId);
      sn(" already at target position!");
    }

    return;
  }

  if (m_currentlyLerpingChannelCount >= MAX_LERPING_CHANNELS) {
    // TODO: implement behaviour here!
    sn("Fail: No more free slots for channels to lerp!");
    return;
  }

  // Add the channelId to the list
  if (SHOW_DEBUG_INFO) {
    st("Success: Channel ");
    st(channelId);
    sn(" added to list of lerping channels");
  }

  m_currenltyLerpingChannels[m_currentlyLerpingChannelCount++] = channelId;
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

      if (SHOW_DEBUG_INFO) {
        st("Channel ");
        st(channelId);
        sn(" has finished lerping and was removed from list of lerping "
           "channels");
      }

      return;
    }
  }
}

bool ChannelController::hasChannelArrivedAtTarget(uint16_t channelId) {
  float currentValue =
      readFloatForChannelFromEepromBuffer(channelId, MEM_SLOT_LERP_CURRENT_POS);
  float targetValue = (float)readUint16tForChannelFromEepromBuffer(
      channelId, MEM_SLOT_LERP_TARGET_VALUE);

  float delta = abs(currentValue - targetValue);

  if (delta < MAX_LERP_DELTA) {
    return true;
  } else {
    return false;
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

void ChannelController::lerpLoopEvent(const uint16_t deltaTimeInMilliseconds) {
  uint8_t index = 0;
  while (index < m_currentlyLerpingChannelCount) {
        updateLerpingChannel(m_currenltyLerpingChannels[index],
                         deltaTimeInMilliseconds);

    if (hasChannelArrivedAtTarget(m_currenltyLerpingChannels[index])) {
      removeChannelFromCurrentlyLerpingList(m_currenltyLerpingChannels[index]);
    } else {
      index++;
    }
  }
}
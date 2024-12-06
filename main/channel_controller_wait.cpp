#include "channel_controller.h"
#include "eeprom.h"
#include "helpers.h"
#include "server_controller.h"

void ChannelController::waitLoopEvent(uint16_t deltaTimeInMilliseconds) {

  for (uint16_t i = 0; i < m_currentlyWaitingChannelCount; i++) {
    bool hasChannelWaitedEnough =
        updateCurrenltyWaitingChannel(i, deltaTimeInMilliseconds);

    if (hasChannelWaitedEnough) {
      removeChannelFromWaitingList(m_currenltyWaitingChannels[i].channelId);
      // Recheck the current position
      i--;
    }
  }
}

void ChannelController::addChannelToCurrentlyWaitingList(uint16_t channelId,
                                                         uint16_t timeout,
                                                         uint16_t pwmValue,
                                                         float percentage) {
  // Check if the channelId is already in the list
  for (uint16_t i = 0; i < m_currentlyWaitingChannelCount; i++) {
    if (m_currenltyWaitingChannels[i].channelId == channelId) {
      m_currenltyWaitingChannels[i].remainingDelayInMilliseconds =
          timeout; // Update timeout
      return;
    }
  }

  // Add the channelId if there's space
  if (m_currentlyWaitingChannelCount < MAX_WAITING_CHANNELS) {
    m_currenltyWaitingChannels[m_currentlyWaitingChannelCount++] = {
        channelId, timeout, pwmValue, percentage};
  }
}

void ChannelController::removeChannelFromWaitingList(uint16_t channelId) {
  for (uint16_t i = 0; i < m_currentlyWaitingChannelCount; i++) {
    if (m_currenltyWaitingChannels[i].channelId == channelId) {
      // Remove the channelId by shifting the remaining elements
      for (uint16_t j = i; j < m_currentlyWaitingChannelCount - 1; j++) {
        m_currenltyWaitingChannels[j] = m_currenltyWaitingChannels[j + 1];
      }
      m_currentlyWaitingChannelCount--;
      return;
    }
  }
}

bool ChannelController::updateCurrenltyWaitingChannel(uint16_t channelIndex,
                                                      uint16_t elapsedTime) {

  if (m_currenltyWaitingChannels[channelIndex].remainingDelayInMilliseconds <
      elapsedTime) {
    // Timeout expired, trigger action
    uint16_t channelId = m_currenltyWaitingChannels[channelIndex].channelId;
    uint16_t pwmValue = m_currenltyWaitingChannels[channelIndex].pwmValue;
    float percentage = m_currenltyWaitingChannels[channelIndex].percentage;

    // Channel needs to set itself
    setChannelPwmValue(channelId, pwmValue);
    // Channel needs to command other channels
    commandLinkedChannel(channelId, percentage, 0);

    return true;
  }

  // Channel did not wait enough, substract remaining wait time
  m_currenltyWaitingChannels[channelIndex].remainingDelayInMilliseconds -=
      elapsedTime;
  return false;
}
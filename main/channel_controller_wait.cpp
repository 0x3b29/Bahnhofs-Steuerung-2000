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
                                                         uint16_t delay,
                                                         uint16_t pwmValue,
                                                         float percentage) {
  // Check if the channelId is already in the list
  for (uint16_t i = 0; i < m_currentlyWaitingChannelCount; i++) {
    if (m_currenltyWaitingChannels[i].channelId == channelId) {
      // If it is, set the new values

      if (SHOW_DEBUG_INFO) {
        st("Update channel ");
        st(channelId);
        st(" to wait for ");
        st(delay);
        st(" ms and then set ");
        st(pwmValue);
        st(" PWM which is ");
        st(percentage);
        sn("%");
      }

      m_currenltyWaitingChannels[i].remainingDelayInMilliseconds = delay;
      m_currenltyWaitingChannels[i].pwmValue = pwmValue;
      m_currenltyWaitingChannels[i].percentage = percentage;
      return;
    }
  }

  // Add the channelId if there's space
  if (m_currentlyWaitingChannelCount < MAX_WAITING_CHANNELS) {
    if (SHOW_DEBUG_INFO) {
      st("Set channel ");
      st(channelId);
      st(" to wait for ");
      st(delay);
      st(" ms and then set ");
      st(pwmValue);
      st(" PWM which is ");
      st(percentage);
      sn("%");
    }

    m_currenltyWaitingChannels[m_currentlyWaitingChannelCount++] = {
        channelId, delay, pwmValue, percentage};
  } else {
    // TODO: implement behaviour here!
    sn("No more free slots for channels to wait!");
  }
}

void ChannelController::removeChannelFromWaitingList(uint16_t channelId) {
  st("Remove channel ");
  st(channelId);
  sn(" from list of waiting channels");

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

    if (SHOW_DEBUG_INFO) {
      st("Channel ");
      st(channelId);
      st(" waited enough and set ");
      st(pwmValue);
      st(" PWM which is ");
      st(percentage);
      sn("%");
    }

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
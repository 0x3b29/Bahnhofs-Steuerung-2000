#include "state_manager.h"

void StateManager::loadStateFromEepromBuffer() {
  this->m_numChannels = readUInt16FromEepromBuffer(MEM_SLOT_CHANNELS);

  if (m_numChannels > MAX_TOTAL_CHANNELS) {
    Serial.println("WARNING: READ CHANNEL COUNT BIGGER THAN "
                   "MAX_TOTAL_CHANNELS. RESET TO MAX_TOTAL_CHANNELS");
    m_numChannels = MAX_TOTAL_CHANNELS;
  }

  m_toggleForceAllOff = readBoolFromEepromBuffer(MEM_SLOT_FORCE_ALL_OFF);
  m_toggleForceAllOn = readBoolFromEepromBuffer(MEM_SLOT_FORCE_ALL_ON);
  m_toggleRandomChaos = readBoolFromEepromBuffer(MEM_SLOT_RANDOM_CHAOS);
  m_toggleOneBasedAddresses =
      readBoolFromEepromBuffer(MEM_SLOT_ONE_BASED_ADDRESSES);
  m_toggleCompactDisplay = readBoolFromEepromBuffer(MEM_SLOT_COMPACT_DISPLAY);
  m_toggleRandomEvents = readBoolFromEepromBuffer(MEM_SLOT_RANDOM_EVENTS);
  m_togglePropagateEvents = readBoolFromEepromBuffer(MEM_SLOT_PROPAGATE_EVENTS);
  m_toggleShowOptions = readBoolFromEepromBuffer(MEM_SLOT_SHOW_OPTIONS);
  m_toggleShowActions = readBoolFromEepromBuffer(MEM_SLOT_SHOW_ACTIONS);
  m_toggleRunningLights = readBoolFromEepromBuffer(MEM_SLOT_RUNNING_LIGHTS);

  m_highPwmBoards = readUInt16FromEepromBuffer(MEM_SLOT_HIGH_PWM);
}

void StateManager::setNumChannels(uint16_t numChannels) {
  Serial.println("Setting numchannels");
  m_numChannels = numChannels;
}

uint16_t StateManager::getNumChannels() { return m_numChannels; }

void StateManager::setToggleRandomChaos(bool toggleRandomChaos) {
  m_toggleRandomChaos = toggleRandomChaos;
}

bool StateManager::getToggleRandomChaos() { return m_toggleRandomChaos; }

void StateManager::setToggleForceAllOff(bool toggleForceAllOff) {
  m_toggleForceAllOff = toggleForceAllOff;
}

bool StateManager::getToggleForceAllOff() { return m_toggleForceAllOff; }

void StateManager::setToggleForceAllOn(bool toggleForceAllOn) {
  m_toggleForceAllOn = toggleForceAllOn;
}

bool StateManager::getToggleForceAllOn() { return m_toggleForceAllOn; }

void StateManager::setToggleOneBasedAddresses(bool toggleOneBasedAddresses) {
  m_toggleOneBasedAddresses = toggleOneBasedAddresses;
}

bool StateManager::getToggleOneBasedAddresses() {
  return m_toggleOneBasedAddresses;
}

void StateManager::setToggleCompactDisplay(bool toggleCompactDisplay) {
  m_toggleCompactDisplay = toggleCompactDisplay;
}

bool StateManager::getToggleRunningLights() { return m_toggleRunningLights; }

void StateManager::setToggleRunningLights(bool toggleRunningLights) {
  m_toggleRunningLights = toggleRunningLights;
}

bool StateManager::getToggleCompactDisplay() { return m_toggleCompactDisplay; }

void StateManager::setToggleRandomEvents(bool toggleRandomEvents) {
  m_toggleRandomEvents = toggleRandomEvents;
}

bool StateManager::getToggleRandomEvents() { return m_toggleRandomEvents; }

void StateManager::setTogglePropagateEvents(bool togglePropagateEvents) {
  m_togglePropagateEvents = togglePropagateEvents;
}

bool StateManager::getTogglePropagateEvents() {
  return m_togglePropagateEvents;
}

void StateManager::setToggleShowOptions(bool toggleShowOptions) {
  m_toggleShowOptions = toggleShowOptions;
}

bool StateManager::getToggleShowOptions() { return m_toggleShowOptions; }

void StateManager::setToggleShowActions(bool toggleShowActions) {
  m_toggleShowActions = toggleShowActions;
}

bool StateManager::getToggleShowActions() { return m_toggleShowActions; }

void StateManager::setRenderEditChannel(bool renderEditChannel) {
  m_renderEditChannel = renderEditChannel;
}

bool StateManager::getRenderEditChannel() { return m_renderEditChannel; }

void StateManager::setChannelIdToEdit(uint16_t channelIdToEdit) {
  m_channelIdToEdit = channelIdToEdit;
}

uint16_t StateManager::getChannelIdToEdit() { return m_channelIdToEdit; }

void StateManager::setHighPwmBoards(uint16_t highPwmBoards) {
  m_highPwmBoards = highPwmBoards;
}

uint16_t StateManager::getHighPwmBoards() { return m_highPwmBoards; }

void StateManager::setHighPwmBoard(uint8_t boardIndex, bool isHigh) {
  if (boardIndex > 15) {
    Serial.println("Bad board index, must be smaller than 8");
    return;
  }

  // Calculate the value to change only the specific bit we want
  uint16_t mask = 1 << boardIndex;

  // If we want to set the bit (make it 1)
  if (isHigh) {
    m_highPwmBoards = m_highPwmBoards | mask;
  }
  // If we want to clear the bit (make it 0)
  else {
    m_highPwmBoards = m_highPwmBoards & ~mask;
  }
}

bool StateManager::getHighPwmBoard(uint8_t boardIndex) {
  // Calculate the value to check only the specific bit we want
  uint16_t mask = 1 << boardIndex;

  // Return true if the bit is set (1), or false if it is clear (0)
  return (m_highPwmBoards & mask) != 0;
}
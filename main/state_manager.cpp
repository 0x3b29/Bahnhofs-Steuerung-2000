#include "state_manager.h"

void StateManager::loadStateFromEepromBuffer() {
  this->m_numChannels = readUInt16FromEepromBuffer(MEM_SLOT_CHANNELS);
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

void StateManager::setRenderAnchor(bool renderAnchor) {
  m_renderAnchor = renderAnchor;
}

bool StateManager::getRenderAnchor() { return m_renderAnchor; }

void StateManager::setAnchorChannelId(uint16_t anchorChannelId) {
  anchorChannelId = anchorChannelId;
}

uint16_t StateManager::getAnchorChannelId() { return m_anchorChannelId; }
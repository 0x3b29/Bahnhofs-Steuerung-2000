#include "state_manager.h"

void StateManager::setNumChannels(uint16_t numChannels) {
  Serial.println("Setting numchannels");
    m_numChannels = numChannels;
}

uint16_t StateManager::getNumChannels() {
    return m_numChannels;
}

void StateManager::setToggleRandomChaos(bool toggleRandomChaos) {
    m_toggleRandomChaos = toggleRandomChaos;
}

bool StateManager::getToggleRandomChaos() {
    return m_toggleRandomChaos;
}

void StateManager::setToggleForceAllOff(bool toggleForceAllOff) {
    m_toggleForceAllOff = toggleForceAllOff;
}

bool StateManager::getToggleForceAllOff() {
    return m_toggleForceAllOff;
}

void StateManager::setToggleForceAllOn(bool toggleForceAllOn) {
    m_toggleForceAllOn = toggleForceAllOn;
}

bool StateManager::getToggleForceAllOn() {
    return m_toggleForceAllOn;
}

void StateManager::setToggleOneBasedAddresses(bool toggleOneBasedAddresses) {
    m_toggleOneBasedAddresses = toggleOneBasedAddresses;
}

bool StateManager::getToggleOneBasedAddresses() {
    return m_toggleOneBasedAddresses;
}

void StateManager::setToggleCompactDisplay(bool toggleCompactDisplay) {
    m_toggleCompactDisplay = toggleCompactDisplay;
}

bool StateManager::getToggleCompactDisplay() {
    return m_toggleCompactDisplay;
}

void StateManager::setToggleRandomEvents(bool toggleRandomEvents) {
    m_toggleRandomEvents = toggleRandomEvents;
}

bool StateManager::getToggleRandomEvents() {
    return m_toggleRandomEvents;
}

void StateManager::setTogglePropagateEvents(bool togglePropagateEvents) {
    m_togglePropagateEvents = togglePropagateEvents;
}

bool StateManager::getTogglePropagateEvents() {
    return m_togglePropagateEvents;
}

void StateManager::setToggleShowOptions(bool toggleShowOptions) {
    m_toggleShowOptions = toggleShowOptions;
}

bool StateManager::getToggleShowOptions() {
    return m_toggleShowOptions;
}

void StateManager::setToggleShowActions(bool toggleShowActions) {
    m_toggleShowActions = toggleShowActions;
}

bool StateManager::getToggleShowActions() {
    return m_toggleShowActions;
}

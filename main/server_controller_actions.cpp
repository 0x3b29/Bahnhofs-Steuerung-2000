#include "channel_controller.h"
#include "eeprom.h"
#include "render.h"
#include "server_controller.h"
#include <WiFiServer.h>

void ServerController::toggleOneBasedAddresses() {
  char toggleOneBasedAddressesBuffer[2] = "0";
  getValueFromData(m_requestBuffer,
                   "toggleOneBasedAddresses=", toggleOneBasedAddressesBuffer,
                   2);
  bool toggleOneBasedAddresses = atoi(toggleOneBasedAddressesBuffer);
  writeBoolToEepromBuffer(MEM_SLOT_ONE_BASED_ADDRESSES,
                          toggleOneBasedAddresses);
  m_stateManager->setToggleOneBasedAddresses(toggleOneBasedAddresses);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
}

void ServerController::setCustomValue() {
  // User changed value via a slider
  getValueFromData(m_requestBuffer, "channelId=", m_channelIdBuffer, 5);
  uint16_t channelIdAsNumber = atoi(m_channelIdBuffer);

  char channelOutputCustomValueBuffer[5] = "0";
  getValueFromData(m_requestBuffer,
                   "customValue=", channelOutputCustomValueBuffer, 5);
  uint16_t customValue = atoi(channelOutputCustomValueBuffer);

  char propagateValueBuffer[2] = "0";
  getValueFromData(m_requestBuffer, "propagateValue=", propagateValueBuffer, 2);
  bool propagateValue = atoi(propagateValueBuffer);

  if (propagateValue) {
    uint16_t value2 = readUint16tForChannelFromEepromBuffer(
        channelIdAsNumber, MEM_SLOT_OUTPUT_VALUE2);

    uint16_t value1 = readUint16tForChannelFromEepromBuffer(
        channelIdAsNumber, MEM_SLOT_OUTPUT_VALUE1);

    // Avoid division by zero by checking if value2 and value1 are the same
    if (value2 == value1) {
      sn("Warning: Value2 should never be value1");
      return;
    }

    // Calculate percentage of customValue in reference to value2 and value1
    float percentage = mapf(customValue, value2, value1, 100, 0);

    m_channelController->applyAndPropagateValue(channelIdAsNumber, customValue,
                                                percentage);
  } else {
    m_channelController->setChannelPwmValue(channelIdAsNumber, customValue);
  }
}

void ServerController::toggleCompactDisplay() {
  char toggleCompactDisplayBuffer[2] = "0";
  getValueFromData(m_requestBuffer,
                   "toggleCompactDisplay=", toggleCompactDisplayBuffer, 2);
  bool toggleCompactDisplay = atoi(toggleCompactDisplayBuffer);
  writeBoolToEepromBuffer(MEM_SLOT_COMPACT_DISPLAY, toggleCompactDisplay);
  m_stateManager->setToggleCompactDisplay(toggleCompactDisplay);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
}

void ServerController::setChannelToValue1() {
  char setChannelToValue1IdBuffer[4];
  uint16_t setChannelToValue1Id;

  getValueFromData(m_requestBuffer,
                   "setChannelToValue1=", setChannelToValue1IdBuffer, 4);

  setChannelToValue1Id = atoi(setChannelToValue1IdBuffer);

  bool useOutputValue1 = readBoolForChannelFromEepromBuffer(
      setChannelToValue1Id, MEM_SLOT_USES_OUTPUT_VALUE1);

  uint16_t value1 = 0;

  if (useOutputValue1) {
    value1 = readUint16tForChannelFromEepromBuffer(setChannelToValue1Id,
                                                   MEM_SLOT_OUTPUT_VALUE1);
  }

  m_channelController->applyAndPropagateValue(setChannelToValue1Id, value1, 0);
}

void ServerController::setChannelToValue2() {
  char setChannelToValue2IdBuffer[4];
  uint16_t setChannelToValue2Id;
  getValueFromData(m_requestBuffer,
                   "setChannelToValue2=", setChannelToValue2IdBuffer, 4);
  setChannelToValue2Id = atoi(setChannelToValue2IdBuffer);

  uint16_t pwmValue = readUint16tForChannelFromEepromBuffer(
      setChannelToValue2Id, MEM_SLOT_OUTPUT_VALUE2);

  m_channelController->applyAndPropagateValue(setChannelToValue2Id, pwmValue,
                                              100);
}

void ServerController::toggleForceAllOff() {
  char toggleForceAllOffBuffer[2] = "0";
  getValueFromData(m_requestBuffer,
                   "toggleForceAllOff=", toggleForceAllOffBuffer, 2);
  bool toggleForceAllOff = atoi(toggleForceAllOffBuffer);
  writeBoolToEepromBuffer(MEM_SLOT_FORCE_ALL_OFF, toggleForceAllOff);
  m_stateManager->setToggleForceAllOff(toggleForceAllOff);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
  m_channelController->applyInitialState();
}

void ServerController::toggleForceAllOn() {
  char toggleForceAllOnBuffer[2] = "0";
  getValueFromData(m_requestBuffer, "toggleForceAllOn=", toggleForceAllOnBuffer,
                   2);
  bool toggleForceAllOn = atoi(toggleForceAllOnBuffer);
  writeBoolToEepromBuffer(MEM_SLOT_FORCE_ALL_ON, toggleForceAllOn);
  m_stateManager->setToggleForceAllOn(toggleForceAllOn);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
  m_channelController->applyInitialState();
}

void ServerController::toggleRandomChaos() {
  char toggleRandomChaosBuffer[2] = "0";
  getValueFromData(m_requestBuffer,
                   "toggleRandomChaos=", toggleRandomChaosBuffer, 2);
  bool toggleRandomChaos = atoi(toggleRandomChaosBuffer);
  writeBoolToEepromBuffer(MEM_SLOT_RANDOM_CHAOS, toggleRandomChaos);
  m_stateManager->setToggleRandomChaos(toggleRandomChaos);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
}

void ServerController::toggleRunningLights() {
  char toggleRunningLightsBuffer[2] = "0";
  getValueFromData(m_requestBuffer,
                   "toggleRunningLights=", toggleRunningLightsBuffer, 2);
  bool toggleRunningLights = atoi(toggleRunningLightsBuffer);
  writeBoolToEepromBuffer(MEM_SLOT_RUNNING_LIGHTS, toggleRunningLights);
  m_stateManager->setToggleRunningLights(toggleRunningLights);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
}

void ServerController::toggleRandomEvents() {
  char toggleRandomEventsBuffer[2] = "0";
  getValueFromData(m_requestBuffer,
                   "toggleRandomEvents=", toggleRandomEventsBuffer, 2);
  bool toggleRandomEvents = atoi(toggleRandomEventsBuffer);
  writeBoolToEepromBuffer(MEM_SLOT_RANDOM_EVENTS, toggleRandomEvents);
  m_stateManager->setToggleRandomEvents(toggleRandomEvents);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
}

void ServerController::togglePropagateEvents() {
  char togglePropagateEventsBuffer[2] = "0";
  getValueFromData(m_requestBuffer,
                   "togglePropagateEvents=", togglePropagateEventsBuffer, 2);
  bool togglePropagateEvents = atoi(togglePropagateEventsBuffer);
  writeBoolToEepromBuffer(MEM_SLOT_PROPAGATE_EVENTS, togglePropagateEvents);
  m_stateManager->setTogglePropagateEvents(togglePropagateEvents);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
}

void ServerController::updateNumberOfChannels() {
  uint16_t oldNumChannels = m_stateManager->getNumChannels();

  char numChannelsBuffer[4] = "0";
  getValueFromData(m_requestBuffer, "numChannels=", numChannelsBuffer, 4);
  uint16_t numChannels = atoi(numChannelsBuffer);
  writeUInt16ToEepromBuffer(MEM_SLOT_CHANNELS, numChannels);
  m_stateManager->setNumChannels(numChannels);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);

  // If we have new channels, we load them, check them and wipe them if
  // nessesary
  if (oldNumChannels < numChannels) {
    for (int i = oldNumChannels; i < numChannels; i++) {
      Serial.print("Checking page for channel: ");
      Serial.println(i);
      loadPageFromEepromToEepromBufferAndCheckIntegrity(i + 1);
    }
  }
}

void ServerController::toggleShowOptions() {
  char toggleShowOptionsBuffer[2] = "0";
  getValueFromData(m_requestBuffer,
                   "toggleShowOptions=", toggleShowOptionsBuffer, 2);
  uint8_t toggleShowOptionsInt = atoi(toggleShowOptionsBuffer);
  bool toggleShowOptions = false;

  if (toggleShowOptionsInt == 1) {
    toggleShowOptions = true;
  }

  writeBoolToEepromBuffer(MEM_SLOT_SHOW_OPTIONS, toggleShowOptions);
  m_stateManager->setToggleShowOptions(toggleShowOptions);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
}

void ServerController::toggleShowActions() {
  char toggleShowActionsBuffer[2] = "0";
  getValueFromData(m_requestBuffer,
                   "toggleShowActions=", toggleShowActionsBuffer, 2);
  uint8_t toggleShowActionsInt = atoi(toggleShowActionsBuffer);
  bool toggleShowActions = false;

  if (toggleShowActionsInt == 1) {
    toggleShowActions = true;
  }

  writeBoolToEepromBuffer(MEM_SLOT_SHOW_ACTIONS, toggleShowActions);
  m_stateManager->setToggleShowActions(toggleShowActions);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
}

void ServerController::toggleHighPwmBoard() {
  char toggleHighPwmBoardIdCharArray[3] = "0";
  getValueFromData(m_requestBuffer,
                   "additionalData=", toggleHighPwmBoardIdCharArray, 3);
  uint8_t toggleHighPwmBoardId = atoi(toggleHighPwmBoardIdCharArray);
  bool isBoardHigh = m_stateManager->getHighPwmBoard(toggleHighPwmBoardId);
  m_stateManager->setHighPwmBoard(toggleHighPwmBoardId, !isBoardHigh);

  uint16_t allHighPwmBoards = m_stateManager->getHighPwmBoards();
  writeUInt16ToEepromBuffer(MEM_SLOT_HIGH_PWM, allHighPwmBoards);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);

  m_channelController->updatePwmBoard(toggleHighPwmBoardId);
}

void ServerController::setAllChannels() {
  char channelOutputValue2Buffer[5] = "0";
  getValueFromData(m_requestBuffer,
                   "setAllChannels=", channelOutputValue2Buffer, 5);
  uint16_t outputValue2 = atoi(channelOutputValue2Buffer);
  m_channelController->setAllChannels(outputValue2);
}
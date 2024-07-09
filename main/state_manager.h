#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "eeprom.h"
#include "main.h"
#include <Arduino.h>

class StateManager {
private:
  uint16_t m_numChannels = 0;
  bool m_toggleRandomChaos = false;
  bool m_toggleForceAllOff = false;
  bool m_toggleForceAllOn = false;
  bool m_toggleOneBasedAddresses = false;
  bool m_toggleCompactDisplay = false;
  bool m_toggleRandomEvents = false;
  bool m_togglePropagateEvents = false;
  bool m_toggleShowOptions = false;
  bool m_toggleShowActions = false;
  bool m_toggleRunningLights = false;

  bool m_renderEditChannel = false;
  uint16_t m_channelIdToEdit = 0;

public:
  void loadStateFromEepromBuffer();

  void setNumChannels(uint16_t numChannels);
  uint16_t getNumChannels();

  void setToggleRandomChaos(bool toggleRandomChaos);
  bool getToggleRandomChaos();

  void setToggleForceAllOff(bool toggleForceAllOff);
  bool getToggleForceAllOff();

  void setToggleForceAllOn(bool toggleForceAllOn);
  bool getToggleForceAllOn();

  void setToggleOneBasedAddresses(bool toggleOneBasedAddresses);
  bool getToggleOneBasedAddresses();

  void setToggleCompactDisplay(bool toggleCompactDisplay);
  bool getToggleCompactDisplay();

  void setToggleRunningLights(bool toggleRunningLights);
  bool getToggleRunningLights();

  void setToggleRandomEvents(bool toggleRandomEvents);
  bool getToggleRandomEvents();

  void setTogglePropagateEvents(bool togglePropagateEvents);
  bool getTogglePropagateEvents();

  void setToggleShowOptions(bool toggleShowOptions);
  bool getToggleShowOptions();

  void setToggleShowActions(bool toggleShowActions);
  bool getToggleShowActions();

  void setRenderEditChannel(bool renderEditChannel);
  bool getRenderEditChannel();

  void setChannelIdToEdit(uint16_t channelIdToEdit);
  uint16_t getChannelIdToEdit();
};

#endif // STATE_MANAGER_H

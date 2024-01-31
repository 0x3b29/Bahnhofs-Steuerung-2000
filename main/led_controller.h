#ifndef led_controller
#define led_controller

#include "main.h"
#include <Adafruit_PWMServoDriver.h>
#include <Arduino.h>

class LedController {
private:
  uint16_t m_numChannels;
  bool m_toggleRandomChaos;
  bool m_toggleForceAllOff;
  bool m_toggleForceAllOn;
  bool m_toggleOneBasedAddresses;
  bool m_togglePropagateEvents;
  bool m_foundRecursion;
  
  uint16_t m_binaryCount;

  Adafruit_PWMServoDriver m_pwmBoards[PWM_BOARDS];

public:
  LedController();
  void initializePwmBoards();

  void setNumChannels(uint16_t numChannels);
  void setToggleRandomChaos(bool toggleRandomChaos);
  void setToggleForceAllOff(bool toggleForceAllOff);
  void setToggleForceAllOn(bool toggleForceAllOn);
  void setToggleOneBasedAddresses(bool toggleOneBasedAddresses);
  void setTogglePropagateEvents(bool togglePropagateEvents);

  bool getFoundRecursion();
  void resetRecursionFlag();

  void setChannelBrightness(int channel, uint16_t brightness);
  void commandLinkedChannel(uint16_t commandingChannelId, bool turnOn,
                            int depth, int maxDepth);
  void applyAndPropagateValue(int channel, uint16_t brightness);
  void applyInitialState();
  void turnAllChannelsOff();
  void turnAllChannels25();
  void turnAllChannels50();
  void turnAllChannels100();
  void turnEvenChannelsOn();
  void turnOddChannelsOn();
  void countBinary();
  bool shouldInvokeEvent(uint8_t freq);

  void calculateRandomEvents();
  void setEveryChannelToRandomValue();
};

#endif // led_controller
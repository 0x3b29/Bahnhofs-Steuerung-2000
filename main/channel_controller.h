#ifndef led_controller
#define led_controller

#include "main.h"
#include "state_manager.h"
#include <Adafruit_PWMServoDriver.h>
#include <Arduino.h>

class ChannelController {
private:
  StateManager *m_stateManager;

  bool m_foundRecursion;
  uint16_t m_binaryCount;
  uint16_t m_nextRunningLight;

  Adafruit_PWMServoDriver m_pwmBoards[PWM_BOARDS];

  uint16_t currenltyLerpingChannels[MAX_TOTAL_CHANNELS];
  uint16_t currentlyLerpingChannelCount = 0;

  float previousTime = 0;

public:
  ChannelController(StateManager *stateManager);
  void initializePwmBoards();
  void updatePwmBoard(int boardIndex);
  void initializeLerpTimer();

  bool getFoundRecursion();
  void resetRecursionFlag();

  void setChannelPwmValue(int channel, uint16_t pwmValue);
  void setPWM(int channel, int boardIndex, int subAddress, uint16_t pwmValue);

  void addChannelToCurrentlyLerpingList(uint16_t channel);
  void removeChannelFromCurrentlyLerpingList(uint16_t channel);
  void updateLerpingChannel(uint16_t channel);

  void commandLinkedChannel(uint16_t commandingChannelId, float percentage,
                            int depth, int maxDepth);
  void applyAndPropagateValue(int channel, uint16_t pwmValue, float percentage);
  void applyInitialState();
  void setAllChannels(uint8_t percentage);
  void turnEvenChannelsOn();
  void turnOddChannelsOn();
  void countBinary();
  bool shouldInvokeEvent(uint8_t freq);

  void calculateRandomEvents();
  void setEveryChannelToRandomValue();
  void setNextRunningLight();

  void loopEvent();
};

#endif // led_controller
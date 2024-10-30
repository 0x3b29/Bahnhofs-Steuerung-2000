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

public:
  ChannelController(StateManager *stateManager);
  void initializePwmBoards();
  void updatePwmBoard(int boardIndex);

  bool getFoundRecursion();
  void resetRecursionFlag();

  void setChannelPwmValue(int channel, uint16_t pwmValue);
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
};

#endif // led_controller
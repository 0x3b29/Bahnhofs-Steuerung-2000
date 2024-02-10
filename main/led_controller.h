#ifndef led_controller
#define led_controller

#include "main.h"
#include "state_manager.h"
#include <Adafruit_PWMServoDriver.h>
#include <Arduino.h>

class LedController {
private:
  StateManager *m_stateManager;

  bool m_foundRecursion;
  uint16_t m_binaryCount;
  uint16_t m_nextRunningLight;

  Adafruit_PWMServoDriver m_pwmBoards[PWM_BOARDS];

public:
  LedController(StateManager *stateManager);
  void initializePwmBoards();

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
  void setNextRunningLight();
};

#endif // led_controller
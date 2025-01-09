#ifndef led_controller
#define led_controller

#include "bahnhofssteuerung2000.h"
#include "state_manager.h"
#include <Adafruit_PWMServoDriver.h>
#include <Arduino.h>

#define MAX_LERP_DELTA 0.01

class ChannelController {
private:
  struct ChannelDelay {
    uint16_t channelId;
    long remainingDelayInMilliseconds;
    uint16_t pwmValue;
    float percentage;
  };

  StateManager *m_stateManager;

  bool m_foundRecursion;
  uint16_t m_binaryCount;
  uint16_t m_nextRunningLight;

  Adafruit_PWMServoDriver m_pwmBoards[PWM_BOARDS];

  uint16_t m_currenltyLerpingChannels[MAX_LERPING_CHANNELS];
  uint16_t m_currentlyLerpingChannelCount = 0;

  ChannelDelay m_currenltyWaitingChannels[MAX_WAITING_CHANNELS];
  uint16_t m_currentlyWaitingChannelCount = 0;

  unsigned long m_previousMillis = 0;

  void lerpLoopEvent(const uint16_t deltaTimeInMilliseconds);
  void waitLoopEvent(const uint16_t deltaTimeInMilliseconds);

  void addChannelToCurrentlyLerpingList(uint16_t channelId);
  void removeChannelFromCurrentlyLerpingList(uint16_t channelId);
  float updateLerpingChannel(uint16_t channelId,
                             uint16_t deltaTimeInMilliseconds);

  void addChannelToCurrentlyWaitingList(uint16_t channelId, uint16_t delay,
                                        uint16_t pwmValue, float percentage);
  void removeChannelFromWaitingList(uint16_t channelId);
  bool updateCurrenltyWaitingChannel(uint16_t channelIndex,
                                     uint16_t elapsedTime);

  void commandLinkedChannel(uint16_t commandingChannelId, float percentage,
                            int depth);

public:
  ChannelController(StateManager *stateManager);
  void initializePwmBoards();
  void updatePwmBoard(int boardIndex);
  void initializePreviousMillis();

  bool getFoundRecursion();
  void resetRecursionFlag();

  void setChannelPwmValue(int channel, uint16_t pwmValue);
  bool hasChannelArrivedAtTarget(uint16_t channelId);
  void setPWM(int channel, int boardIndex, int subAddress, uint16_t pwmValue);

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
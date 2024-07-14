#include "channel_controller.h"
#include "eeprom.h"
#include "helpers.h"
#include "server_controller.h"

ChannelController::ChannelController(StateManager *stateManager)
    : m_binaryCount(0), m_foundRecursion(false) {
  this->m_stateManager = stateManager;
}

Adafruit_PWMServoDriver m_pwmBoards[PWM_BOARDS];

void ChannelController ::initializePwmBoards() {
  for (int i = 0; i < PWM_BOARDS; i++) {
    int pwmAddress = 0x40 + i;

    m_pwmBoards[i] = Adafruit_PWMServoDriver(pwmAddress);
    m_pwmBoards[i].begin();
    m_pwmBoards[i].setOscillatorFrequency(27000000);
    m_pwmBoards[i].setPWMFreq(PWM_REFRESH_RATE);

    Serial.print("PWM Board ");
    Serial.print(i);
    Serial.print(" initialized with address ");
    Serial.println(pwmAddress);
  }
}

bool ChannelController::getFoundRecursion() { return this->m_foundRecursion; }

void ChannelController::resetRecursionFlag() { this->m_foundRecursion = false; }

void ChannelController::setChannelPwmValue(int channel, uint16_t pwmValue) {
  int boardIndex = getBoardIndexForChannel(channel);
  int subAddress = getBoardSubAddressForChannel(channel);

  bool useOutputValue2 =
      readBoolForChannelFromEepromBuffer(channel, MEM_SLOT_USES_OUTPUT_VALUE2);

  if (m_stateManager->getToggleForceAllOff() == true) {
    uint16_t value2;

    if (useOutputValue2) {
      value2 = readUint16tForChannelFromEepromBuffer(channel,
                                                     MEM_SLOT_OUTPUT_VALUE2);
    } else {
      value2 = 0;
    }

    this->m_pwmBoards[boardIndex].setPWM(subAddress, 0, value2);
    return;
  }

  if (m_stateManager->getToggleForceAllOn() == true) {
    uint16_t value1;

    if (useOutputValue2) {
      value1 = readUint16tForChannelFromEepromBuffer(channel,
                                                     MEM_SLOT_OUTPUT_VALUE1);
    } else {
      value1 = 4095;
    }

    this->m_pwmBoards[boardIndex].setPWM(subAddress, 0, value1);
    return;
  }

  this->m_pwmBoards[boardIndex].setPWM(subAddress, 0, pwmValue);
}

void ChannelController::applyAndPropagateValue(int channel, uint16_t pwmValue,
                                               bool wasValue1) {
  setChannelPwmValue(channel, pwmValue);

  if (m_stateManager->getTogglePropagateEvents() &&
      !m_stateManager->getToggleForceAllOff() &&
      !m_stateManager->getToggleForceAllOn() &&
      !m_stateManager->getToggleRandomChaos()) {

    commandLinkedChannel(channel, wasValue1, 0, 5);
  }
}

void ChannelController::commandLinkedChannel(uint16_t commandingChannelId,
                                             bool wasValue1, int depth,
                                             int maxDepth) {

  if (depth > maxDepth) {
    st("Detected and broke recusrion for commandingChannelId ");
    sn(commandingChannelId);

    m_foundRecursion = true;
    return;
  }

  for (uint16_t i = 0; i < m_stateManager->getNumChannels(); i++) {
    if (i == commandingChannelId) {
      continue;
    }

    bool isChannelLinked =
        readBoolForChannelFromEepromBuffer(i, MEM_SLOT_IS_LINKED);

    if (isChannelLinked == true) {
      uint16_t linkedChannelId =
          readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_LINKED_CHANNEL);

      if (linkedChannelId == commandingChannelId) {
        uint16_t pwmValue = 0;

        if (wasValue1) {
          pwmValue =
              readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE1);
        } else {
          bool useOutputValue2 = readBoolForChannelFromEepromBuffer(
              i, MEM_SLOT_USES_OUTPUT_VALUE2);

          if (useOutputValue2) {
            pwmValue = readUint16tForChannelFromEepromBuffer(
                i, MEM_SLOT_OUTPUT_VALUE2);
          }
        }

        setChannelPwmValue(i, pwmValue);
        commandLinkedChannel(i, wasValue1, depth + 1, maxDepth);
      }
    }
  }
}

void ChannelController::applyInitialState() {
  this->m_binaryCount = 0;

  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {

    bool isInitialStateValue1 =
        readBoolForChannelFromEepromBuffer(i, MEM_SLOT_START_OUTPUT_VALUE1);

    uint16_t pwmValue = 0;

    if (isInitialStateValue1 == true) {
      pwmValue =
          readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE1);
    } else {
      pwmValue =
          readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE2);
    }

    applyAndPropagateValue(i, pwmValue, isInitialStateValue1);
  }
}

void ChannelController::setAllChannels(uint8_t percentage) {
  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {
    bool usesValue2 =
        readBoolForChannelFromEepromBuffer(i, MEM_SLOT_USES_OUTPUT_VALUE2);
    uint16_t pwmValue = 0;

    if (usesValue2) {
      uint16_t value1 =
          readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE1);
      uint16_t value2 =
          readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE2);

      // Ensure correct interpolation regardless of whether value2 is greater or
      // smaller than value1
      if (value1 < value2) {
        pwmValue =
            value1 + (uint16_t)((value2 - value1) * (percentage / 100.0));
      } else {
        pwmValue =
            value1 - (uint16_t)((value1 - value2) * (percentage / 100.0));
      }
    } else {
      pwmValue = (uint16_t)(((float)4095 / 100) * percentage);
    }

    setChannelPwmValue(i, pwmValue);
  }
}

void ChannelController::turnEvenChannelsOn() {
  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {

    int userFacingAddress = i;

    if (m_stateManager->getToggleOneBasedAddresses()) {
      userFacingAddress++;
    }

    bool useOutputValue2 =
        readBoolForChannelFromEepromBuffer(i, MEM_SLOT_USES_OUTPUT_VALUE2);
    uint16_t value1;
    uint16_t value2;

    if (useOutputValue2) {
      value1 = readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE1);
      value2 = readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE2);
    } else {
      value1 = 4095;
      value2 = 0;
    }

    if (userFacingAddress % 2 == 0) {
      setChannelPwmValue(i, 4095);
    } else {
      setChannelPwmValue(i, 0);
    }
  }
}

void ChannelController::turnOddChannelsOn() {
  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {

    int userFacingAddress = i;

    if (m_stateManager->getToggleOneBasedAddresses()) {
      userFacingAddress++;
    }

    if (userFacingAddress % 2 == 1) {
      setChannelPwmValue(i, 4095);
    } else {
      setChannelPwmValue(i, 0);
    }
  }
}

void ChannelController::countBinary() {
  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {
    if ((this->m_binaryCount & (1 << i)) == 0) {
      setChannelPwmValue(i, 0);
    } else {
      setChannelPwmValue(i, 4095);
    }
  }

  this->m_binaryCount++;
}

bool ChannelController::shouldInvokeEvent(uint8_t freq) {
  // This function is called 10x per secod, so the probability
  // of events per hour (called freq) is seconds * minutes * 10 =
  // 60 * 60 * 10 = 36000

  // Generate a random number between 0 and 35999
  uint16_t randNumber = random(36000);

  // Check if the random number is less than the threshold
  return randNumber < freq;
}

void ChannelController::calculateRandomEvents() {
  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {
    bool randomOn = readBoolForChannelFromEepromBuffer(i, MEM_SLOT_RANDOM_ON);
    bool randomOff = readBoolForChannelFromEepromBuffer(i, MEM_SLOT_RANDOM_OFF);

    /*
    bool isLinked = readBoolForChannelFromEepromBuffer(i, MEM_SLOT_IS_LINKED);

    uint16_t linkedChannel =
        readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_LINKED_CHANNEL);
    */

    // No need for further checks if channel has no random events
    if (!randomOn && !randomOff) {
      continue;
    }

    uint8_t randomOnFreq =
        readUint8tForChannelFromEepromBuffer(i, MEM_SLOT_RANDOM_ON_FREQ);

    uint8_t randomOffFreq =
        readUint8tForChannelFromEepromBuffer(i, MEM_SLOT_RANDOM_OFF_FREQ);

    bool turnOn = shouldInvokeEvent(randomOnFreq);
    bool turnOff = shouldInvokeEvent(randomOffFreq);

    if (randomOn & turnOn) {
      st("Got random on event for channel ");
      sn(i);

      uint16_t brightness =
          readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE1);

      applyAndPropagateValue(i, brightness, true);

      /*
      if (isLinked) {
        uint16_t pwmValue = readUint16tForChannelFromEepromBuffer(
            linkedChannel, MEM_SLOT_OUTPUT_VALUE1);

        applyAndPropagateValue(linkedChannel, pwmValue, true);
      }*/
    } else if (randomOff & turnOff) {
      st("Got random off event for channel ");
      sn(i);

      uint16_t brightness = 0;

      bool useOutputValue2 =
          readBoolForChannelFromEepromBuffer(i, MEM_SLOT_USES_OUTPUT_VALUE2);

      if (useOutputValue2) {
        brightness =
            readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE1);
      }

      applyAndPropagateValue(i, useOutputValue2, false);

      /*
      if (isLinked) {
        applyAndPropagateValue(linkedChannel, 0);
      }
      */
    }
  }
}

void ChannelController::setEveryChannelToRandomValue() {
  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {
    uint16_t randomBrightness = random(0, 4095);
    setChannelPwmValue(i, randomBrightness);
  }
}

void ChannelController::setNextRunningLight() {
  setChannelPwmValue(m_nextRunningLight, 0);
  m_nextRunningLight++;

  if (m_nextRunningLight >= m_stateManager->getNumChannels()) {
    m_nextRunningLight = 0;
  }

  setChannelPwmValue(m_nextRunningLight, 4095);
}
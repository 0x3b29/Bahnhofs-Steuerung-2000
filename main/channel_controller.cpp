#include "channel_controller.h"
#include "eeprom.h"
#include "helpers.h"
#include "server_controller.h"

ChannelController::ChannelController(StateManager *stateManager)
    : m_binaryCount(0), m_foundRecursion(false) {
  this->m_stateManager = stateManager;
}

Adafruit_PWMServoDriver m_pwmBoards[PWM_BOARDS];

void ChannelController::initializePwmBoards() {
  for (int i = 0; i < PWM_BOARDS; i++) {
    int pwmAddress = 0x40 + i;

    m_pwmBoards[i] = Adafruit_PWMServoDriver(pwmAddress);
    m_pwmBoards[i].begin();
    m_pwmBoards[i].setOscillatorFrequency(27000000);

    bool isBoardHighPwm = m_stateManager->getHighPwmBoard(i);

    if (isBoardHighPwm) {
      m_pwmBoards[i].setPWMFreq(PWM_HIGH_REFRESH_RATE);
    } else {
      m_pwmBoards[i].setPWMFreq(PWM_LOW_REFRESH_RATE);
    }

    Serial.print("PWM Board ");
    Serial.print(i);
    Serial.print(" initialized with address ");
    Serial.print(pwmAddress);
    Serial.print(" and ");

    if (isBoardHighPwm) {
      Serial.print("high ");
    } else {
      Serial.print("low ");
    }

    Serial.println("pwm frequency");
  }
}

void ChannelController::initializeLerpTimer() {
  previousTime = millis() / 1000.0;
}

void ChannelController::updatePwmBoard(int boardIndex) {
  int pwmAddress = 0x40 + boardIndex;
  bool isBoardHighPwm = m_stateManager->getHighPwmBoard(boardIndex);

  if (isBoardHighPwm) {
    m_pwmBoards[boardIndex].setPWMFreq(PWM_HIGH_REFRESH_RATE);
  } else {
    m_pwmBoards[boardIndex].setPWMFreq(PWM_LOW_REFRESH_RATE);
  }

  Serial.print("PWM Board ");
  Serial.print(boardIndex);
  Serial.print(" with address ");
  Serial.print(pwmAddress);
  Serial.print(" updated using ");

  if (isBoardHighPwm) {
    Serial.print("high ");
  } else {
    Serial.print("low ");
  }

  Serial.println("pwm frequency");
}

bool ChannelController::getFoundRecursion() { return this->m_foundRecursion; }

void ChannelController::resetRecursionFlag() { this->m_foundRecursion = false; }

void ChannelController::setChannelPwmValue(int channel, uint16_t pwmValue) {
  int boardIndex = getBoardIndexForChannel(channel);
  int subAddress = getBoardSubAddressForChannel(channel);

  bool useOutputValue1 =
      readBoolForChannelFromEepromBuffer(channel, MEM_SLOT_USES_OUTPUT_VALUE1);

  if (m_stateManager->getToggleForceAllOff() == true) {
    uint16_t value1 = 0;

    if (useOutputValue1) {
      value1 = readUint16tForChannelFromEepromBuffer(channel,
                                                     MEM_SLOT_OUTPUT_VALUE1);
    }

    // Immediate respnse
    // this->m_pwmBoards[boardIndex].setPWM(subAddress, 0, value1);

    // Lerped response
    setPWM(channel, boardIndex, subAddress, value1);
    return;
  }

  if (m_stateManager->getToggleForceAllOn() == true) {
    uint16_t value2;

    if (useOutputValue1) {
      value2 = readUint16tForChannelFromEepromBuffer(channel,
                                                     MEM_SLOT_OUTPUT_VALUE2);
    } else {
      value2 = 4095;
    }

    // Immediate respnse
    // this->m_pwmBoards[boardIndex].setPWM(subAddress, 0, value2);

    // Lerped response
    setPWM(channel, boardIndex, subAddress, value2);
    return;
  }

  Serial.print("The pwmValue: ");
  Serial.println(pwmValue);
  setPWM(channel, boardIndex, subAddress, pwmValue);
}

void ChannelController::setPWM(int channel, int boardIndex, int subAddress,
                               uint16_t pwmValue) {
  bool isChannelLerped =
      readBoolForChannelFromEepromBuffer(channel, MEM_SLOT_IS_LERPED);

  if (isChannelLerped) {
    writeUint16tForChannelToEepromBuffer(channel, MEM_SLOT_LERP_TARGET_VALUE,
                                         pwmValue);

    addChannelToCurrentlyLerpingList(channel);
  } else {
    this->m_pwmBoards[boardIndex].setPWM(subAddress, 0, pwmValue);
  }
}

void ChannelController::addChannelToCurrentlyLerpingList(uint16_t channel) {
  // Check if the channel is already in the list
  for (uint16_t i = 0; i < currentlyLerpingChannelCount; i++) {
    if (currenltyLerpingChannels[i] == channel)
      return;
  }

  // Add the channel to the list
  if (currentlyLerpingChannelCount < MAX_TOTAL_CHANNELS) {
    currenltyLerpingChannels[currentlyLerpingChannelCount++] = channel;
  }
}

void ChannelController::removeChannelFromCurrentlyLerpingList(
    uint16_t channel) {
  for (uint16_t i = 0; i < currentlyLerpingChannelCount; i++) {
    if (currenltyLerpingChannels[i] == channel) {
      // Remove the channel by shifting the remaining elements
      for (uint16_t j = i; j < currentlyLerpingChannelCount - 1; j++) {
        currenltyLerpingChannels[j] = currenltyLerpingChannels[j + 1];
      }
      currentlyLerpingChannelCount--;
      return;
    }
  }
}

void ChannelController::updateLerpingChannel(uint16_t channel) {
  // Read current position and target position
  float currentValue =
      readFloatForChannelFromEepromBuffer(channel, MEM_SLOT_LERP_CURRENT_POS);
  float targetValue = (float)readUint16tForChannelFromEepromBuffer(
      channel, MEM_SLOT_LERP_TARGET_VALUE);
  float lerpSpeed = readFloatForChannelFromEepromBuffer(
      channel, MEM_SLOT_LERP_SPEED); // Time to complete lerp in seconds

  uint16_t value1 = 0;
  uint16_t value2 =
      readUint16tForChannelFromEepromBuffer(channel, MEM_SLOT_OUTPUT_VALUE2);

  bool usesValue1 =
      readBoolForChannelFromEepromBuffer(channel, MEM_SLOT_USES_OUTPUT_VALUE1);

  if (usesValue1) {
    value1 =
        readUint16tForChannelFromEepromBuffer(channel, MEM_SLOT_OUTPUT_VALUE1);
  }

  // Dynamically determine the range (position1 to position2)
  float position1 = (float)value1;
  float position2 = (float)value2;
  float range = abs(position2 - position1);

  // Calculate step size (delta) based on range, speed, and update rate
  const float updatesPerSecond = 100.0; // Adjust based on your loop frequency
  float delta = range / (lerpSpeed * updatesPerSecond);

  // Determine the direction of movement
  if (currentValue < targetValue) {
    currentValue =
        min(currentValue + delta, targetValue); // Increment towards target
  } else {
    currentValue =
        max(currentValue - delta, targetValue); // Decrement towards target
  }

  /*
    Serial.print("channel: ");
    Serial.print(channel);
    Serial.print(" currentValue: ");
    Serial.print(currentValue, 2);
    Serial.print(" targetValue: ");
    Serial.print(targetValue, 2);
    Serial.print(" delta: ");
    Serial.println(delta, 4);
  */

  // Write the new value back to memory
  writeFloatForChannelToEepromBuffer(channel, MEM_SLOT_LERP_CURRENT_POS,
                                     currentValue);

  // Send value to controller
  int boardIndex = getBoardIndexForChannel(channel);
  int subAddress = getBoardSubAddressForChannel(channel);
  uint16_t pwmValue = (uint16_t)round(currentValue);
  this->m_pwmBoards[boardIndex].setPWM(subAddress, 0, pwmValue);

  // If the current value is close to the target, stop lerping
  float diff = abs(currentValue - targetValue);
  if (diff < 0.01) {
    removeChannelFromCurrentlyLerpingList(channel);
    Serial.println("Remove");
  }
}

void ChannelController::applyAndPropagateValue(int channel, uint16_t pwmValue,
                                               float percentage) {
  setChannelPwmValue(channel, pwmValue);

  if (m_stateManager->getTogglePropagateEvents() &&
      !m_stateManager->getToggleForceAllOff() &&
      !m_stateManager->getToggleForceAllOn() &&
      !m_stateManager->getToggleRandomChaos()) {

    commandLinkedChannel(channel, percentage, 0, 5);
  }
}

void ChannelController::commandLinkedChannel(uint16_t commandingChannelId,
                                             float percentage, int depth,
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

    if (isChannelLinked == false) {
      continue;
    }

    uint16_t linkedChannelId =
        readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_LINKED_CHANNEL);

    if (linkedChannelId != commandingChannelId) {
      continue;
    }

    uint16_t value2 =
        readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE2);

    uint16_t value1 = 0;

    bool useOutputValue1 =
        readBoolForChannelFromEepromBuffer(i, MEM_SLOT_USES_OUTPUT_VALUE1);

    if (useOutputValue1) {
      value1 = readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE1);
    }

    uint16_t pwmValue = (int)mapf(percentage, 0, 100, value1, value2);

    setChannelPwmValue(i, pwmValue);
    commandLinkedChannel(i, percentage, depth + 1, maxDepth);
  }
}

void ChannelController::applyInitialState() {
  this->m_binaryCount = 0;

  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {

    bool isInitialStateValue2 = readBoolForChannelFromEepromBuffer(
        i, MEM_SLOT_IS_START_VALUE_OUTPUT_VALUE2);

    uint16_t pwmValue = 0;

    if (isInitialStateValue2 == true) {
      pwmValue =
          readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE2);
    } else {
      pwmValue =
          readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE1);
    }

    applyAndPropagateValue(i, pwmValue, isInitialStateValue2 ? 100 : 0);
  }
}

void ChannelController::setAllChannels(uint8_t percentage) {
  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {
    bool usesValue1 =
        readBoolForChannelFromEepromBuffer(i, MEM_SLOT_USES_OUTPUT_VALUE1);
    uint16_t pwmValue = 0;

    if (usesValue1) {
      uint16_t value2 =
          readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE2);
      uint16_t value1 =
          readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE1);

      // Here we map across the defined range. This is done to avoid damaging
      // the servos or the driven object, which is usually not an issue for an
      // LED
      pwmValue = (int)mapf(percentage, 0, 100, value1, value2);
    } else {
      // In case we use only one value, we do not map over the selected range
      // (e.g. 0 to 60%) but over the full range (e.g. 0% to 100%)
      // If this is not whats needed, the following lines can be exchanged

      pwmValue = (uint16_t)(((float)4095 / 100) * percentage);
      // pwmValue = (int)mapf(percentage, 0, 100, 0, value2);
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

    bool useOutputValue1 =
        readBoolForChannelFromEepromBuffer(i, MEM_SLOT_USES_OUTPUT_VALUE1);
    uint16_t value2;
    uint16_t value1;

    if (useOutputValue1) {
      value2 = readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE2);
      value1 = readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE1);
    } else {
      value2 = 4095;
      value1 = 0;
    }

    if (userFacingAddress % 2 == 0) {
      setChannelPwmValue(i, value2);
    } else {
      setChannelPwmValue(i, value1);
    }
  }
}

void ChannelController::turnOddChannelsOn() {
  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {

    int userFacingAddress = i;

    if (m_stateManager->getToggleOneBasedAddresses()) {
      userFacingAddress++;
    }

    bool useOutputValue1 =
        readBoolForChannelFromEepromBuffer(i, MEM_SLOT_USES_OUTPUT_VALUE1);
    uint16_t value2;
    uint16_t value1;

    if (useOutputValue1) {
      value2 = readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE2);
      value1 = readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE1);
    } else {
      value2 = 4095;
      value1 = 0;
    }

    if (userFacingAddress % 2 == 1) {
      setChannelPwmValue(i, value2);
    } else {
      setChannelPwmValue(i, value1);
    }
  }
}

void ChannelController::countBinary() {
  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {

    bool useOutputValue1 =
        readBoolForChannelFromEepromBuffer(i, MEM_SLOT_USES_OUTPUT_VALUE1);
    uint16_t value2;
    uint16_t value1;

    if (useOutputValue1) {
      value2 = readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE2);
      value1 = readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE1);
    } else {
      value2 = 4095;
      value1 = 0;
    }

    if ((this->m_binaryCount & (1 << i)) == 0) {
      setChannelPwmValue(i, value1);
    } else {
      setChannelPwmValue(i, value2);
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
      st("Got random on/value2 event for channel ");
      st(i);
      st(" at ");
      sn(millis());

      uint16_t outputValue2 =
          readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE2);

      applyAndPropagateValue(i, outputValue2, 100);
    } else if (randomOff & turnOff) {
      st("Got random off/value1 event for channel ");
      st(i);
      st(" at ");
      sn(millis());

      uint16_t outputValue1 = 0;

      bool useOutputValue1 =
          readBoolForChannelFromEepromBuffer(i, MEM_SLOT_USES_OUTPUT_VALUE1);

      if (useOutputValue1) {
        outputValue1 =
            readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE1);
      }

      applyAndPropagateValue(i, outputValue1, 0);
    }
  }
}

void ChannelController::setEveryChannelToRandomValue() {
  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {

    bool useOutputValue1 =
        readBoolForChannelFromEepromBuffer(i, MEM_SLOT_USES_OUTPUT_VALUE1);
    uint16_t value2;
    uint16_t value1;

    if (useOutputValue1) {
      value2 = readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE2);
      value1 = readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_OUTPUT_VALUE1);
    } else {
      value2 = 4095;
      value1 = 0;
    }

    uint16_t randomValue = 0;
    if (value2 > value1) {
      randomValue = random(value1, value2);
    } else {
      randomValue = random(value2, value1);
    }

    setChannelPwmValue(i, randomValue);
  }
}

void ChannelController::setNextRunningLight() {
  // First we set previous output to value1
  bool useOutputValue1 = readBoolForChannelFromEepromBuffer(
      m_nextRunningLight, MEM_SLOT_USES_OUTPUT_VALUE1);
  uint16_t value2;
  uint16_t value1;

  if (useOutputValue1) {
    value2 = readUint16tForChannelFromEepromBuffer(m_nextRunningLight,
                                                   MEM_SLOT_OUTPUT_VALUE2);
    value1 = readUint16tForChannelFromEepromBuffer(m_nextRunningLight,
                                                   MEM_SLOT_OUTPUT_VALUE1);
  } else {
    value2 = 4095;
    value1 = 0;
  }

  setChannelPwmValue(m_nextRunningLight, value1);

  // Then we select next output
  m_nextRunningLight++;

  if (m_nextRunningLight >= m_stateManager->getNumChannels()) {
    m_nextRunningLight = 0;
  }

  // And set it to value 2
  useOutputValue1 = readBoolForChannelFromEepromBuffer(
      m_nextRunningLight, MEM_SLOT_USES_OUTPUT_VALUE1);

  if (useOutputValue1) {
    value2 = readUint16tForChannelFromEepromBuffer(m_nextRunningLight,
                                                   MEM_SLOT_OUTPUT_VALUE2);
    value1 = readUint16tForChannelFromEepromBuffer(m_nextRunningLight,
                                                   MEM_SLOT_OUTPUT_VALUE1);
  } else {
    value2 = 4095;
    value1 = 0;
  }

  setChannelPwmValue(m_nextRunningLight, value2);
}

void ChannelController::loopEvent() {
  for (uint16_t i = 0; i < currentlyLerpingChannelCount; i++) {
    updateLerpingChannel(currenltyLerpingChannels[i]);
  }
}
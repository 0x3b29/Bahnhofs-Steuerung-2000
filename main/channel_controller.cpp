#include "channel_controller.h"
#include "eeprom.h"
#include "helpers.h"
#include "server_controller.h"

ChannelController::ChannelController(StateManager *stateManager)
    : m_binaryCount(0), m_foundRecursion(false) {
  this->m_stateManager = stateManager;
}

Adafruit_PWMServoDriver m_pwmBoards[PWM_BOARDS];

void ChannelController::initializePreviousMillis() {
  m_previousMillis = millis();
}

void ChannelController::initializePwmBoards() {
  for (int pwmBoardIndex = 0; pwmBoardIndex < PWM_BOARDS; pwmBoardIndex++) {
    int pwmAddress = 0x40 + pwmBoardIndex;

    m_pwmBoards[pwmBoardIndex] = Adafruit_PWMServoDriver(pwmAddress);
    m_pwmBoards[pwmBoardIndex].begin();
    m_pwmBoards[pwmBoardIndex].setOscillatorFrequency(27000000);

    bool isBoardHighPwm = m_stateManager->getHighPwmBoard(pwmBoardIndex);

    if (isBoardHighPwm) {
      m_pwmBoards[pwmBoardIndex].setPWMFreq(PWM_HIGH_REFRESH_RATE);
    } else {
      m_pwmBoards[pwmBoardIndex].setPWMFreq(PWM_LOW_REFRESH_RATE);
    }

    Serial.print("PWM Board ");
    Serial.print(pwmBoardIndex);
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

void ChannelController::setChannelPwmValue(int channelId, uint16_t pwmValue) {
  int boardIndex = getBoardIndexForChannel(channelId);
  int subAddress = getBoardSubAddressForChannel(channelId);

  bool useOutputValue1 = readBoolForChannelFromEepromBuffer(
      channelId, MEM_SLOT_USES_OUTPUT_VALUE1);

  if (m_stateManager->getToggleForceAllOff() == true) {
    uint16_t value1 = 0;

    if (useOutputValue1) {
      value1 = readUint16tForChannelFromEepromBuffer(channelId,
                                                     MEM_SLOT_OUTPUT_VALUE1);
    }

    // Immediate respnse
    // this->m_pwmBoards[boardIndex].setPWM(subAddress, 0, value1);

    // Lerped response
    setPWM(channelId, boardIndex, subAddress, value1);
    return;
  }

  if (m_stateManager->getToggleForceAllOn() == true) {
    uint16_t value2;

    if (useOutputValue1) {
      value2 = readUint16tForChannelFromEepromBuffer(channelId,
                                                     MEM_SLOT_OUTPUT_VALUE2);
    } else {
      value2 = 4095;
    }

    // Immediate respnse
    // this->m_pwmBoards[boardIndex].setPWM(subAddress, 0, value2);

    // Lerped response
    setPWM(channelId, boardIndex, subAddress, value2);
    return;
  }

  Serial.print("The pwmValue: ");
  Serial.println(pwmValue);
  setPWM(channelId, boardIndex, subAddress, pwmValue);
}

void ChannelController::setPWM(int channelId, int boardIndex, int subAddress,
                               uint16_t pwmValue) {
  bool isChannelLerped =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_IS_LERPED);

  if (isChannelLerped) {
    writeUint16tForChannelToEepromBuffer(channelId, MEM_SLOT_LERP_TARGET_VALUE,
                                         pwmValue);

    addChannelToCurrentlyLerpingList(channelId);
  } else {
    this->m_pwmBoards[boardIndex].setPWM(subAddress, 0, pwmValue);
  }
}

void ChannelController::applyAndPropagateValue(int channelId, uint16_t pwmValue,
                                               float percentage) {
  setChannelPwmValue(channelId, pwmValue);

  if (m_stateManager->getTogglePropagateEvents() &&
      !m_stateManager->getToggleForceAllOff() &&
      !m_stateManager->getToggleForceAllOn() &&
      !m_stateManager->getToggleRandomChaos()) {

    commandLinkedChannel(channelId, percentage, 0);
  }
}

void ChannelController::commandLinkedChannel(uint16_t commandingChannelId,
                                             float percentage, int depth) {

  if (depth > MAX_RECURSION) {
    st("Detected and broke recusrion for commandingChannelId ");
    sn(commandingChannelId);

    m_foundRecursion = true;
    return;
  }

  for (uint16_t channelId = 0; channelId < m_stateManager->getNumChannels();
       channelId++) {
    if (channelId == commandingChannelId) {
      continue;
    }

    bool isChannelLinked =
        readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_IS_LINKED);

    if (isChannelLinked == false) {
      continue;
    }

    uint16_t linkedChannelId = readUint16tForChannelFromEepromBuffer(
        channelId, MEM_SLOT_LINKED_CHANNEL);

    if (linkedChannelId != commandingChannelId) {
      continue;
    }

    uint16_t value2 = readUint16tForChannelFromEepromBuffer(
        channelId, MEM_SLOT_OUTPUT_VALUE2);

    uint16_t value1 = 0;

    bool useOutputValue1 = readBoolForChannelFromEepromBuffer(
        channelId, MEM_SLOT_USES_OUTPUT_VALUE1);

    if (useOutputValue1) {
      value1 = readUint16tForChannelFromEepromBuffer(channelId,
                                                     MEM_SLOT_OUTPUT_VALUE1);
    }

    uint16_t pwmValue = (int)mapf(percentage, 0, 100, value1, value2);

    uint16_t linkTimeout =
        readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_LINK_TIMEOUT);

    if (linkTimeout > 0) {
      // Add to list of waiting channels
      addChannelToCurrentlyWaitingList(channelId, linkTimeout, pwmValue,
                                       percentage);
    } else {
      // Set channelId and search for more linked channels
      setChannelPwmValue(channelId, pwmValue);
      commandLinkedChannel(channelId, percentage, depth + 1);
    }
  }
}

void ChannelController::applyInitialState() {
  this->m_binaryCount = 0;

  for (int channelId = 0; channelId < m_stateManager->getNumChannels();
       channelId++) {

    bool isInitialStateValue2 = readBoolForChannelFromEepromBuffer(
        channelId, MEM_SLOT_IS_START_VALUE_OUTPUT_VALUE2);

    uint16_t pwmValue = 0;

    if (isInitialStateValue2 == true) {
      pwmValue = readUint16tForChannelFromEepromBuffer(channelId,
                                                       MEM_SLOT_OUTPUT_VALUE2);
    } else {
      pwmValue = readUint16tForChannelFromEepromBuffer(channelId,
                                                       MEM_SLOT_OUTPUT_VALUE1);
    }

    applyAndPropagateValue(channelId, pwmValue, isInitialStateValue2 ? 100 : 0);
  }
}

void ChannelController::setAllChannels(uint8_t percentage) {
  for (int channelId = 0; channelId < m_stateManager->getNumChannels();
       channelId++) {
    bool usesValue1 = readBoolForChannelFromEepromBuffer(
        channelId, MEM_SLOT_USES_OUTPUT_VALUE1);
    uint16_t pwmValue = 0;

    if (usesValue1) {
      uint16_t value2 = readUint16tForChannelFromEepromBuffer(
          channelId, MEM_SLOT_OUTPUT_VALUE2);
      uint16_t value1 = readUint16tForChannelFromEepromBuffer(
          channelId, MEM_SLOT_OUTPUT_VALUE1);

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

    setChannelPwmValue(channelId, pwmValue);
  }
}

void ChannelController::turnEvenChannelsOn() {
  for (int channelId = 0; channelId < m_stateManager->getNumChannels();
       channelId++) {

    int userFacingAddress = channelId;

    if (m_stateManager->getToggleOneBasedAddresses()) {
      userFacingAddress++;
    }

    bool useOutputValue1 = readBoolForChannelFromEepromBuffer(
        channelId, MEM_SLOT_USES_OUTPUT_VALUE1);
    uint16_t value2;
    uint16_t value1;

    if (useOutputValue1) {
      value2 = readUint16tForChannelFromEepromBuffer(channelId,
                                                     MEM_SLOT_OUTPUT_VALUE2);
      value1 = readUint16tForChannelFromEepromBuffer(channelId,
                                                     MEM_SLOT_OUTPUT_VALUE1);
    } else {
      value2 = 4095;
      value1 = 0;
    }

    if (userFacingAddress % 2 == 0) {
      setChannelPwmValue(channelId, value2);
    } else {
      setChannelPwmValue(channelId, value1);
    }
  }
}

void ChannelController::turnOddChannelsOn() {
  for (int channelId = 0; channelId < m_stateManager->getNumChannels();
       channelId++) {

    int userFacingAddress = channelId;

    if (m_stateManager->getToggleOneBasedAddresses()) {
      userFacingAddress++;
    }

    bool useOutputValue1 = readBoolForChannelFromEepromBuffer(
        channelId, MEM_SLOT_USES_OUTPUT_VALUE1);
    uint16_t value2;
    uint16_t value1;

    if (useOutputValue1) {
      value2 = readUint16tForChannelFromEepromBuffer(channelId,
                                                     MEM_SLOT_OUTPUT_VALUE2);
      value1 = readUint16tForChannelFromEepromBuffer(channelId,
                                                     MEM_SLOT_OUTPUT_VALUE1);
    } else {
      value2 = 4095;
      value1 = 0;
    }

    if (userFacingAddress % 2 == 1) {
      setChannelPwmValue(channelId, value2);
    } else {
      setChannelPwmValue(channelId, value1);
    }
  }
}

void ChannelController::countBinary() {
  for (int channelId = 0; channelId < m_stateManager->getNumChannels();
       channelId++) {

    bool useOutputValue1 = readBoolForChannelFromEepromBuffer(
        channelId, MEM_SLOT_USES_OUTPUT_VALUE1);
    uint16_t value2;
    uint16_t value1;

    if (useOutputValue1) {
      value2 = readUint16tForChannelFromEepromBuffer(channelId,
                                                     MEM_SLOT_OUTPUT_VALUE2);
      value1 = readUint16tForChannelFromEepromBuffer(channelId,
                                                     MEM_SLOT_OUTPUT_VALUE1);
    } else {
      value2 = 4095;
      value1 = 0;
    }

    if ((this->m_binaryCount & (1 << channelId)) == 0) {
      setChannelPwmValue(channelId, value1);
    } else {
      setChannelPwmValue(channelId, value2);
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
  for (int channelId = 0; channelId < m_stateManager->getNumChannels();
       channelId++) {
    bool randomOn =
        readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_ON);
    bool randomOff =
        readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_OFF);

    // No need for further checks if channelId has no random events
    if (!randomOn && !randomOff) {
      continue;
    }

    uint8_t randomOnFreq = readUint8tForChannelFromEepromBuffer(
        channelId, MEM_SLOT_RANDOM_ON_FREQ);

    uint8_t randomOffFreq = readUint8tForChannelFromEepromBuffer(
        channelId, MEM_SLOT_RANDOM_OFF_FREQ);

    bool turnOn = shouldInvokeEvent(randomOnFreq);
    bool turnOff = shouldInvokeEvent(randomOffFreq);

    if (randomOn & turnOn) {
      st("Got random on/value2 event for channelId ");
      st(channelId);
      st(" at ");
      sn(millis());

      uint16_t outputValue2 = readUint16tForChannelFromEepromBuffer(
          channelId, MEM_SLOT_OUTPUT_VALUE2);

      applyAndPropagateValue(channelId, outputValue2, 100);
    } else if (randomOff & turnOff) {
      st("Got random off/value1 event for channelId ");
      st(channelId);
      st(" at ");
      sn(millis());

      uint16_t outputValue1 = 0;

      bool useOutputValue1 = readBoolForChannelFromEepromBuffer(
          channelId, MEM_SLOT_USES_OUTPUT_VALUE1);

      if (useOutputValue1) {
        outputValue1 = readUint16tForChannelFromEepromBuffer(
            channelId, MEM_SLOT_OUTPUT_VALUE1);
      }

      applyAndPropagateValue(channelId, outputValue1, 0);
    }
  }
}

void ChannelController::setEveryChannelToRandomValue() {
  for (int channelId = 0; channelId < m_stateManager->getNumChannels();
       channelId++) {

    bool useOutputValue1 = readBoolForChannelFromEepromBuffer(
        channelId, MEM_SLOT_USES_OUTPUT_VALUE1);
    uint16_t value2;
    uint16_t value1;

    if (useOutputValue1) {
      value2 = readUint16tForChannelFromEepromBuffer(channelId,
                                                     MEM_SLOT_OUTPUT_VALUE2);
      value1 = readUint16tForChannelFromEepromBuffer(channelId,
                                                     MEM_SLOT_OUTPUT_VALUE1);
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

    setChannelPwmValue(channelId, randomValue);
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
  unsigned long currentMillis = millis();
  uint16_t deltaTimeInMilliseconds = currentMillis - m_previousMillis;
  m_previousMillis = currentMillis;

  ChannelController::waitLoopEvent(deltaTimeInMilliseconds);
  ChannelController::lerpLoopEvent(deltaTimeInMilliseconds);
}
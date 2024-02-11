#include "led_controller.h"
#include "eeprom.h"
#include "helpers.h"
#include "server_controller.h"

LedController::LedController(StateManager *stateManager)
    : m_binaryCount(0), m_foundRecursion(false) {
  this->m_stateManager = stateManager;
}

Adafruit_PWMServoDriver m_pwmBoards[PWM_BOARDS];

void LedController ::initializePwmBoards() {
  for (int i = 0; i < PWM_BOARDS; i++) {
    int pwmAddress = 0x40 + i;

    m_pwmBoards[i] = Adafruit_PWMServoDriver(pwmAddress);
    m_pwmBoards[i].begin();
    m_pwmBoards[i].setOscillatorFrequency(27000000);
    m_pwmBoards[i].setPWMFreq(PWM_REFRESH_RATE);

    Serial.print("Board added: ");
    Serial.println(pwmAddress);
  }
}

bool LedController::getFoundRecursion() { return this->m_foundRecursion; }

void LedController::resetRecursionFlag() { this->m_foundRecursion = false; }

void LedController::setChannelBrightness(int channel, uint16_t brightness) {
  int boardIndex = getBoardIndexForChannel(channel);
  int subAddress = getBoardSubAddressForChannel(channel);

  if (m_stateManager->getToggleForceAllOff() == true) {
    this->m_pwmBoards[boardIndex].setPWM(subAddress, 0, 0);
    return;
  }

  if (m_stateManager->getToggleForceAllOn() == true) {
    this->m_pwmBoards[boardIndex].setPWM(subAddress, 0, 4095);
    return;
  }

  this->m_pwmBoards[boardIndex].setPWM(subAddress, 0, brightness);
}

void LedController::applyAndPropagateValue(int channel, uint16_t brightness) {
  setChannelBrightness(channel, brightness);

  if (m_stateManager->getTogglePropagateEvents() &&
      !m_stateManager->getToggleForceAllOff() &&
      !m_stateManager->getToggleForceAllOn() &&
      !m_stateManager->getToggleRandomChaos()) {

    bool turnOn = false;

    if (brightness > 0) {
      turnOn = true;
    }

    commandLinkedChannel(channel, turnOn, 0, 5);
  }
}

void LedController::commandLinkedChannel(uint16_t commandingChannelId,
                                         bool turnOn, int depth, int maxDepth) {

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
        uint16_t brightness = 0;

        if (turnOn) {
          brightness =
              readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_BRIGHTNESS);
        }

        setChannelBrightness(i, brightness);
        commandLinkedChannel(i, turnOn, depth + 1, maxDepth);
      }
    }
  }
}

void LedController::applyInitialState() {
  this->m_binaryCount = 0;

  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {

    bool initialState =
        readBoolForChannelFromEepromBuffer(i, MEM_SLOT_INITIAL_STATE);

    uint16_t brightness = 0;

    if (initialState == true) {
      brightness =
          readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_BRIGHTNESS);
    }

    applyAndPropagateValue(i, brightness);
  }
}

void LedController::setAllChannels(uint16_t brightness) {
  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {
    setChannelBrightness(i, brightness);
  }
}

void LedController::turnEvenChannelsOn() {
  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {

    int userFacingAddress = i;

    if (m_stateManager->getToggleOneBasedAddresses()) {
      userFacingAddress++;
    }

    if (userFacingAddress % 2 == 0) {
      setChannelBrightness(i, 4095);
    } else {
      setChannelBrightness(i, 0);
    }
  }
}

void LedController::turnOddChannelsOn() {
  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {

    int userFacingAddress = i;

    if (m_stateManager->getToggleOneBasedAddresses()) {
      userFacingAddress++;
    }

    if (userFacingAddress % 2 == 1) {
      setChannelBrightness(i, 4095);
    } else {
      setChannelBrightness(i, 0);
    }
  }
}

void LedController::countBinary() {
  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {
    if ((this->m_binaryCount & (1 << i)) == 0) {
      setChannelBrightness(i, 0);
    } else {
      setChannelBrightness(i, 4095);
    }
  }

  this->m_binaryCount++;
}

bool LedController::shouldInvokeEvent(uint8_t freq) {
  // This function is called 10x per secod, so the probability
  // of events per hour (called freq) is seconds * minutes * 10 =
  // 60 * 60 * 10 = 36000

  // Generate a random number between 0 and 35999
  uint16_t randNumber = random(36000);

  // Check if the random number is less than the threshold
  return randNumber < freq;
}

void LedController::calculateRandomEvents() {
  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {
    bool randomOn = readBoolForChannelFromEepromBuffer(i, MEM_SLOT_RANDOM_ON);
    bool randomOff = readBoolForChannelFromEepromBuffer(i, MEM_SLOT_RANDOM_OFF);
    bool isLinked = readBoolForChannelFromEepromBuffer(i, MEM_SLOT_IS_LINKED);
    uint16_t linkedChannel =
        readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_LINKED_CHANNEL);

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
          readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_BRIGHTNESS);

      applyAndPropagateValue(i, brightness);

      if (isLinked) {
        uint16_t linkedBrightness = readUint16tForChannelFromEepromBuffer(
            linkedChannel, MEM_SLOT_BRIGHTNESS);

        applyAndPropagateValue(linkedChannel, linkedBrightness);
      }
    } else if (randomOff & turnOff) {
      st("Got random off event for channel ");
      sn(i);

      applyAndPropagateValue(i, 0);

      if (isLinked) {
        applyAndPropagateValue(linkedChannel, 0);
      }
    }
  }
}

void LedController::setEveryChannelToRandomValue() {
  for (int i = 0; i < m_stateManager->getNumChannels(); i++) {
    uint16_t randomBrightness = random(0, 4095);
    setChannelBrightness(i, randomBrightness);
  }
}

void LedController::setNextRunningLight() {
  setChannelBrightness(m_nextRunningLight, 0);
  m_nextRunningLight++;

  if (m_nextRunningLight >= m_stateManager->getNumChannels()) {
    m_nextRunningLight = 0;
  }

  setChannelBrightness(m_nextRunningLight, 4095);
}
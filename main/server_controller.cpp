
#include "server_controller.h"
#include "eeprom.h"
#include "led_controller.h"
#include "render.h"
#include <WiFiServer.h>

WiFiServer m_wifiServer(80);
char m_pageBuffer[PAGE_BUFFER_SIZE];

ServerController::ServerController(StateManager *stateManager,
                                   LedController *ledController,
                                   Renderer *renderer) {
  this->clearPageBuffer();
  this->m_stateManager = stateManager;
  this->m_ledController = ledController;
  this->m_renderer = renderer;
}

void ServerController::begin() { m_wifiServer.begin(); }

void ServerController::replyToClientWithSuccess(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/plain");
  client.println();
  client.println("Settings updated successfully.");
}

void ServerController::replyToClientWithFail(WiFiClient client) {
  client.println("HTTP/1.1 400 Bad Request");
  client.println("Content-type:text/plain");
  client.println();
  client.println("Failed to update settings.");
}

void ServerController::cancelChannelUpdate() {
  // Reload old brigthness
  getValueFromData(m_pageBuffer, "channelId=", m_channelIdBuffer, 5);
  uint16_t channelIdAsNumber = atoi(m_channelIdBuffer);

  uint16_t originalBrightness = readUint16tForChannelFromEepromBuffer(
      channelIdAsNumber, MEM_SLOT_BRIGHTNESS);

  m_ledController->setChannelBrightness(channelIdAsNumber, originalBrightness);

  m_stateManager->setRenderAnchor(true);
  m_stateManager->setAnchorChannelId(channelIdAsNumber);
}

void ServerController::toggleOneBasedAddresses() {
  char toggleOneBasedAddressesBuffer[2] = "0";
  getValueFromData(m_pageBuffer,
                   "toggleOneBasedAddresses=", toggleOneBasedAddressesBuffer,
                   2);
  bool toggleOneBasedAddresses = atoi(toggleOneBasedAddressesBuffer);
  writeBoolToEepromBuffer(MEM_SLOT_ONE_BASED_ADDRESSES,
                          toggleOneBasedAddresses);
  m_stateManager->setToggleOneBasedAddresses(toggleOneBasedAddresses);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
}

void ServerController::testBrightness() {
  // User changed brightness on edit channel form
  getValueFromData(m_pageBuffer, "channelId=", m_channelIdBuffer, 5);
  uint16_t channelIdAsNumber = atoi(m_channelIdBuffer);

  char channelBrightnessBuffer[5] = "0";
  getValueFromData(m_pageBuffer, "channelBrightness=", channelBrightnessBuffer,
                   5);
  uint16_t channelBrightness = atoi(channelBrightnessBuffer);

  m_ledController->setChannelBrightness(channelIdAsNumber, channelBrightness);
}

void ServerController::updateChannel() {
  getValueFromData(m_pageBuffer, "channelId=", m_channelIdBuffer, 5);
  uint16_t channelIdAsNumber = atoi(m_channelIdBuffer);

  // Worst case, each UTF8 character is 4 bytes long as url encoded
  char urlEncodedNameBuffer[MAX_CHANNEL_NAME_LENGTH * 4];
  getValueFromData(m_pageBuffer, "channelName=", urlEncodedNameBuffer,
                   MAX_CHANNEL_NAME_LENGTH * 4);
  urlDecode(urlEncodedNameBuffer, m_channelNameBuffer, MAX_CHANNEL_NAME_LENGTH);

  char channelBrightnessBuffer[5] = "0";
  getValueFromData(m_pageBuffer, "channelBrightness=", channelBrightnessBuffer,
                   5);

  char initialStateBuffer[2] = "0";
  getValueFromData(m_pageBuffer, "initialState=", initialStateBuffer, 2);
  uint8_t initialState = atoi(initialStateBuffer);
  writeUint8tToEepromBuffer(channelIdAsNumber, MEM_SLOT_INITIAL_STATE,
                            initialState);

  char randomOnBuffer[2] = "0";
  char randomOnFreqBuffer[4] = "0";

  char randomOffBuffer[2] = "0";
  char randomOffFreqBuffer[4] = "0";

  char isLinkedBuffer[2] = "0";
  char linkedChannelIdBuffer[4] = "0";

  getValueFromData(m_pageBuffer, "randomOn=", randomOnBuffer, 2);
  getValueFromData(m_pageBuffer, "frequencyOn=", randomOnFreqBuffer, 4);
  getValueFromData(m_pageBuffer, "randomOff=", randomOffBuffer, 2);
  getValueFromData(m_pageBuffer, "frequencyOff=", randomOffFreqBuffer, 4);
  getValueFromData(m_pageBuffer, "channelLinked=", isLinkedBuffer, 2);
  getValueFromData(m_pageBuffer, "linkedChannelId=", linkedChannelIdBuffer, 4);

  // Todo convert to bool
  uint8_t randomOn = atoi(randomOnBuffer);
  uint8_t randomOnFreq = atoi(randomOnFreqBuffer);

  uint8_t randomOff = atoi(randomOffBuffer);
  uint8_t randomOffFreq = atoi(randomOffFreqBuffer);

  uint8_t isLinked = atoi(isLinkedBuffer);
  uint16_t linkedChannelId = atoi(linkedChannelIdBuffer);

  if (m_stateManager->getToggleOneBasedAddresses()) {
    linkedChannelId--;
  }

  Serial.print("Channel ");
  Serial.print(channelIdAsNumber);

  uint16_t startAddress = 64 + channelIdAsNumber * 64;
  Serial.print(" got startAddress ");
  Serial.println(startAddress);

  uint16_t channelBrightness = atoi(channelBrightnessBuffer);

  writeChannelNameFromChannelNameBufferToEepromBuffer(channelIdAsNumber);

  writeUint16tForChannelToEepromBuffer(channelIdAsNumber, MEM_SLOT_BRIGHTNESS,
                                       channelBrightness);

  writeUint8tToEepromBuffer(channelIdAsNumber, MEM_SLOT_RANDOM_ON, randomOn);
  if (randomOn == 1) {
    writeUint8tToEepromBuffer(channelIdAsNumber, MEM_SLOT_RANDOM_ON_FREQ,
                              randomOnFreq);
  }

  writeUint8tToEepromBuffer(channelIdAsNumber, MEM_SLOT_RANDOM_OFF, randomOff);
  if (randomOff == 1) {
    writeUint8tToEepromBuffer(channelIdAsNumber, MEM_SLOT_RANDOM_OFF_FREQ,
                              randomOffFreq);
  }

  writeUint8tToEepromBuffer(channelIdAsNumber, MEM_SLOT_IS_LINKED, isLinked);
  if (isLinked == 1) {
    writeUint16tForChannelToEepromBuffer(
        channelIdAsNumber, MEM_SLOT_LINKED_CHANNEL, linkedChannelId);
  }

  m_ledController->applyAndPropagateValue(channelIdAsNumber, channelBrightness);

  writePageIntegrity(channelIdAsNumber + 1);
  writePageFromBufferToEeprom(channelIdAsNumber + 1);

  m_stateManager->setRenderAnchor(true);
  m_stateManager->setAnchorChannelId(channelIdAsNumber);
  m_ledController->resetRecursionFlag();
}

void ServerController::toggleCompactDisplay() {
  char toggleCompactDisplayBuffer[2] = "0";
  getValueFromData(m_pageBuffer,
                   "toggleCompactDisplay=", toggleCompactDisplayBuffer, 2);
  bool toggleCompactDisplay = atoi(toggleCompactDisplayBuffer);
  writeBoolToEepromBuffer(MEM_SLOT_COMPACT_DISPLAY, toggleCompactDisplay);
  m_stateManager->setToggleCompactDisplay(toggleCompactDisplay);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
}

void ServerController::turnChannelOff() {
  char turnChannelOffIdBuffer[4];
  uint16_t turnChannelOffId;
  getValueFromData(m_pageBuffer, "turnChannelOff=", turnChannelOffIdBuffer, 4);
  turnChannelOffId = atoi(turnChannelOffIdBuffer);
  m_ledController->applyAndPropagateValue(turnChannelOffId, 0);
}

void ServerController::turnChannelOn() {
  char turnChannelOnIdBuffer[4];
  uint16_t turnChannelOnId;
  getValueFromData(m_pageBuffer, "turnChannelOn=", turnChannelOnIdBuffer, 4);
  turnChannelOnId = atoi(turnChannelOnIdBuffer);

  uint16_t turnOnBrightness = readUint16tForChannelFromEepromBuffer(
      turnChannelOnId, MEM_SLOT_BRIGHTNESS);

  m_ledController->applyAndPropagateValue(turnChannelOnId, turnOnBrightness);
}

void ServerController::editChannel() {
  getValueFromData(m_pageBuffer, "editChannel=", m_channelIdToEditBuffer, 4);

  uint16_t channelIdToEdit = atoi(m_channelIdToEditBuffer);
  readChannelNameFromEepromBufferToChannelNameBuffer(channelIdToEdit);
  m_stateManager->setChannelIdToEdit(channelIdToEdit);
  m_stateManager->setRenderEditChannel(true);
}

void ServerController::toggleForceAllOff() {
  char toggleForceAllOffBuffer[2] = "0";
  getValueFromData(m_pageBuffer, "toggleForceAllOff=", toggleForceAllOffBuffer,
                   2);
  bool toggleForceAllOff = atoi(toggleForceAllOffBuffer);
  writeBoolToEepromBuffer(MEM_SLOT_FORCE_ALL_OFF, toggleForceAllOff);
  m_stateManager->setToggleForceAllOff(toggleForceAllOff);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
  m_ledController->applyInitialState();
}

void ServerController::toggleForceAllOn() {
  char toggleForceAllOnBuffer[2] = "0";
  getValueFromData(m_pageBuffer, "toggleForceAllOn=", toggleForceAllOnBuffer,
                   2);
  bool toggleForceAllOn = atoi(toggleForceAllOnBuffer);
  writeBoolToEepromBuffer(MEM_SLOT_FORCE_ALL_ON, toggleForceAllOn);
  m_stateManager->setToggleForceAllOn(toggleForceAllOn);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
  m_ledController->applyInitialState();
}

void ServerController::toggleRandomChaos() {
  char toggleRandomChaosBuffer[2] = "0";
  getValueFromData(m_pageBuffer, "toggleRandomChaos=", toggleRandomChaosBuffer,
                   2);
  bool toggleRandomChaos = atoi(toggleRandomChaosBuffer);
  writeBoolToEepromBuffer(MEM_SLOT_RANDOM_CHAOS, toggleRandomChaos);
  m_stateManager->setToggleRandomChaos(toggleRandomChaos);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
}

void ServerController::toggleRunningLights() {
  char toggleRunningLightsBuffer[2] = "0";
  getValueFromData(m_pageBuffer,
                   "toggleRunningLights=", toggleRunningLightsBuffer, 2);
  bool toggleRunningLights = atoi(toggleRunningLightsBuffer);
  writeBoolToEepromBuffer(MEM_SLOT_RUNNING_LIGHTS, toggleRunningLights);
  m_stateManager->setToggleRunningLights(toggleRunningLights);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
}

void ServerController::toggleRandomEvents() {
  char toggleRandomEventsBuffer[2] = "0";
  getValueFromData(m_pageBuffer,
                   "toggleRandomEvents=", toggleRandomEventsBuffer, 2);
  bool toggleRandomEvents = atoi(toggleRandomEventsBuffer);
  writeBoolToEepromBuffer(MEM_SLOT_RANDOM_EVENTS, toggleRandomEvents);
  m_stateManager->setToggleRandomEvents(toggleRandomEvents);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
}

void ServerController::togglePropagateEvents() {
  char togglePropagateEventsBuffer[2] = "0";
  getValueFromData(m_pageBuffer,
                   "togglePropagateEvents=", togglePropagateEventsBuffer, 2);
  bool togglePropagateEvents = atoi(togglePropagateEventsBuffer);
  writeBoolToEepromBuffer(MEM_SLOT_PROPAGATE_EVENTS, togglePropagateEvents);
  m_stateManager->setTogglePropagateEvents(togglePropagateEvents);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
}

void ServerController::updateNumberOfChannels() {
  char clearEepromBuffer[10] = "";
  getValueFromData(m_pageBuffer, "clearEeprom=", clearEepromBuffer, 10);

  // TODO: move reset2024 to secrets file
  if (strcmp(clearEepromBuffer, "reset2024") == 0) {
    Serial.println("Clearing Eeprom!!!");
    clearEeprom();
  }

  uint16_t oldNumChannels = m_stateManager->getNumChannels();

  char numChannelsBuffer[4] = "0";
  getValueFromData(m_pageBuffer, "numChannels=", numChannelsBuffer, 4);
  uint16_t numChannels = atoi(numChannelsBuffer);
  writeUInt16ToEepromBuffer(MEM_SLOT_CHANNELS, numChannels);
  m_stateManager->setNumChannels(numChannels);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);

  // If we have new channels, we load them, check them and wipe them if
  // nessesary
  if (oldNumChannels < numChannels) {
    for (int i = oldNumChannels; i < numChannels; i++) {
      Serial.print("Checking page for channel: ");
      Serial.println(i);
      loadPageFromEepromToEepromBufferAndCheckIntegrity(i + 1);
    }
  }
}

void ServerController::toggleShowOptions() {
  char toggleShowOptionsBuffer[2] = "0";
  getValueFromData(m_pageBuffer, "toggleShowOptions=", toggleShowOptionsBuffer,
                   2);
  uint8_t toggleShowOptionsInt = atoi(toggleShowOptionsBuffer);
  bool toggleShowOptions = false;

  if (toggleShowOptionsInt == 1) {
    toggleShowOptions = true;
  }

  writeBoolToEepromBuffer(MEM_SLOT_SHOW_OPTIONS, toggleShowOptions);
  m_stateManager->setToggleShowOptions(toggleShowOptions);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
}

void ServerController::toggleShowActions() {
  char toggleShowActionsBuffer[2] = "0";
  getValueFromData(m_pageBuffer, "toggleShowActions=", toggleShowActionsBuffer,
                   2);
  uint8_t toggleShowActionsInt = atoi(toggleShowActionsBuffer);
  bool toggleShowActions = false;

  if (toggleShowActionsInt == 1) {
    toggleShowActions = true;
  }

  writeBoolToEepromBuffer(MEM_SLOT_SHOW_ACTIONS, toggleShowActions);
  m_stateManager->setToggleShowActions(toggleShowActions);

  writePageIntegrity(0);
  writePageFromBufferToEeprom(0);
}

void ServerController::setAllChannels() {
  char channelBrightnessBuffer[5] = "0";
  getValueFromData(m_pageBuffer, "setAllChannels=", channelBrightnessBuffer, 5);
  uint16_t channelBrightness = atoi(channelBrightnessBuffer);
  m_ledController->setAllChannels(channelBrightness);
}

void ServerController::processRequest(WiFiClient client) {
  bool shouldRerender = false;
  m_stateManager->setRenderAnchor(false);
  m_stateManager->setRenderEditChannel(false);

  if (strstr(m_pageBuffer, "POST") != NULL) {

    if (isKeyInData(m_pageBuffer, "cancelChannelUpdate")) {
      cancelChannelUpdate();
      shouldRerender = true;
    }

    if (isKeyInData(m_pageBuffer, "testBrightness")) {
      testBrightness();
    }

    if (isKeyInData(m_pageBuffer, "toggleOneBasedAddresses")) {
      toggleOneBasedAddresses();
    }

    if (isKeyInData(m_pageBuffer, "toggleCompactDisplay")) {
      toggleCompactDisplay();
    }

    if (isKeyInData(m_pageBuffer, "turnChannelOff")) {
      turnChannelOff();
    }

    if (isKeyInData(m_pageBuffer, "turnChannelOn")) {
      turnChannelOn();
    }

    if (isKeyInData(m_pageBuffer, "editChannel")) {
      editChannel();
      shouldRerender = true;
    }

    if (isKeyInData(m_pageBuffer, "toggleForceAllOff")) {
      toggleForceAllOff();
    }

    if (isKeyInData(m_pageBuffer, "toggleForceAllOn")) {
      toggleForceAllOn();
    }

    if (isKeyInData(m_pageBuffer, "toggleRandomChaos")) {
      toggleRandomChaos();
    }

    if (isKeyInData(m_pageBuffer, "toggleRunningLights")) {
      toggleRunningLights();
    }

    if (isKeyInData(m_pageBuffer, "toggleRandomEvents")) {
      toggleRandomEvents();
    }

    if (isKeyInData(m_pageBuffer, "togglePropagateEvents")) {
      togglePropagateEvents();
    }

    if (isKeyInData(m_pageBuffer, "updateNumberOfChannels")) {
      updateNumberOfChannels();
      shouldRerender = true;
    }

    if (isKeyInData(m_pageBuffer, "updateChannel")) {
      updateChannel();
      shouldRerender = true;
    }

    if (isKeyInData(m_pageBuffer, "resetAllChannels")) {
      m_ledController->applyInitialState();
    }

    if (isKeyInData(m_pageBuffer, "setAllChannels")) {
      setAllChannels();
    }

    if (isKeyInData(m_pageBuffer, "turnEvenChannelsOn")) {
      m_ledController->turnEvenChannelsOn();
    }

    if (isKeyInData(m_pageBuffer, "turnOddChannelsOn")) {
      m_ledController->turnOddChannelsOn();
    }

    if (isKeyInData(m_pageBuffer, "countBinary")) {
      m_ledController->countBinary();
    }

    if (isKeyInData(m_pageBuffer, "toggleShowOptions")) {
      toggleShowOptions();
    }

    if (isKeyInData(m_pageBuffer, "toggleShowActions")) {
      toggleShowActions();
    }

    if (shouldRerender) {
      m_renderer->renderWebPage(client, m_ledController->getFoundRecursion());
    } else {
      if (m_ledController->getFoundRecursion()) {
        replyToClientWithFail(client);
      } else {
        replyToClientWithSuccess(client);
      }
    }

    // Include line to see whats going on in memory
    // dumpEepromData(0, MAX_EEPROM_RANGE - 1);
  } else {
    // For get requests, we always want to render the page to the client
    m_renderer->renderWebPage(client, m_ledController->getFoundRecursion());
  }
}

void ServerController::loopEvent() {
  WiFiClient client = m_wifiServer.available();

  if (client) {
    boolean currentLineIsBlank = true;

    this->clearPageBuffer();
    int pageIndex = 0;

    if (client.connected()) {
      while (client.available()) {
        char c = client.read();
        // Serial.write(c);

        m_pageBuffer[pageIndex] = c;
        pageIndex++;

        if (pageIndex >= PAGE_BUFFER_SIZE) {
          Serial.println("WARNING: PAGE EXCEEDED BUFFER SIZE! DANGER!!!");
        }
      }

      processRequest(client);

      client.stop();
    }
  }
}

// Function to extract value from form data using char arrays
// formData: The form data as a char array
// key: The key as a char array
// value: A char array buffer where the extracted value will be stored
// valueLen: The length of the value buffer
void ServerController::getValueFromData(const char *formData, const char *key,
                                        char *value, int valueLen) {
  const char *startPtr = strstr(formData, key);
  if (startPtr == NULL) {
    Serial.print("Unable to find key: ");
    Serial.println(key);
    return;
  }

  startPtr += strlen(key); // Move pointer past the key

  const char *endPtr = strchr(startPtr, '&');
  if (endPtr == NULL) {
    endPtr =
        formData +
        strlen(formData); // Set endPtr to the end of formData if '&' not found
  }

  int numCharsToCopy = endPtr - startPtr;
  if (numCharsToCopy >= valueLen) {
    numCharsToCopy = valueLen - 1; // Ensure we don't exceed buffer size
  }

  strncpy(value, startPtr, numCharsToCopy);
  value[numCharsToCopy] = '\0'; // Null-terminate the string

  return;
}

bool ServerController::isKeyInData(const char *formData, const char *key) {
  const char *startPtr = formData;
  while ((startPtr = strstr(startPtr, key)) != NULL) {
    // Check if the key is at the start of formData or preceded by '&' or '\n'
    bool atStart = startPtr == formData || *(startPtr - 1) == '&' ||
                   *(startPtr - 1) == '\n';

    // Check if the key is followed by '=', '&' or is at the end of formData
    const char *endPtr = startPtr + strlen(key);
    bool atEnd = *endPtr == '\0' || *endPtr == '&' || *endPtr == '=';

    if (atStart && atEnd) {
      return true;
    }

    // Move past this occurrence to check for another
    startPtr = endPtr;
  }

  return false; // Key not found with proper boundaries
}

void ServerController::clearPageBuffer() {
  for (int i = 0; i < PAGE_BUFFER_SIZE; i++) {
    m_pageBuffer[i] = '\0';
  }
}

void ServerController::urlDecode(const char *urlEncoded, char *decoded,
                                 int maxLen) {
  int len = strlen(urlEncoded);
  int decodedIndex = 0;

  for (int i = 0; i < len && decodedIndex < maxLen - 1; ++i) {
    if (urlEncoded[i] == '+') {
      decoded[decodedIndex++] = ' ';
    } else if (urlEncoded[i] == '%' && i + 2 < len) {
      // Get the next two characters after '%'
      char hex[3];
      hex[0] = urlEncoded[++i];
      hex[1] = urlEncoded[++i];
      hex[2] = '\0';

      // Convert it into a character
      decoded[decodedIndex++] = (char)strtol(hex, NULL, 16);
    } else {
      decoded[decodedIndex++] = urlEncoded[i];
    }
  }

  // Null-terminate the decoded string
  decoded[decodedIndex] = '\0';
}
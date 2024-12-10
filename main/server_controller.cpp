#include "server_controller.h"
#include "channel_controller.h"
#include "eeprom.h"
#include "render.h"
#include <WiFiServer.h>

WiFiServer m_wifiServer(80);

ServerController::ServerController(StateManager *stateManager,
                                   ChannelController *channelController,
                                   Renderer *renderer) {
  this->clearRequestBuffer();
  this->m_stateManager = stateManager;
  this->m_channelController = channelController;
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
  getValueFromData(m_requestBuffer, "channelId=", m_channelIdBuffer, 5);
  uint16_t channelIdAsNumber = atoi(m_channelIdBuffer);

  // TODO: do not always read value2, but previous set value instead
  uint16_t originalBrightness = readUint16tForChannelFromEepromBuffer(
      channelIdAsNumber, MEM_SLOT_OUTPUT_VALUE2);

  m_channelController->setChannelPwmValue(channelIdAsNumber,
                                          originalBrightness);
}

void ServerController::updateBoolIfFound(uint16_t channelId, const char *buffer,
                                         const char *key, int memorySlot) {
  char boolBuffer[2] = "0";
  bool foundKey = getValueFromData(buffer, key, boolBuffer, 2);

  if (foundKey) {
    uint8_t boolAsUint8t = atoi(boolBuffer);
    writeUint8tForChannelToEepromBuffer(channelId, memorySlot, boolAsUint8t);
  }
}

void ServerController::updateUint8tIfFound(uint16_t channelId,
                                           const char *buffer, const char *key,
                                           int memorySlot) {
  char uint8tBuffer[4] = "0";
  bool foundKey = getValueFromData(buffer, key, uint8tBuffer, 4);

  if (foundKey) {
    uint8_t uint8tValue = atoi(uint8tBuffer);
    writeUint8tForChannelToEepromBuffer(channelId, memorySlot, uint8tValue);
  }
}

void ServerController::updateUint16tIfFound(uint16_t channelId,
                                            const char *buffer, const char *key,
                                            int memorySlot) {
  char uint16tBuffer[7] = "0";
  bool foundKey = getValueFromData(buffer, key, uint16tBuffer, 7);

  if (foundKey) {
    uint16_t uint16tValue = atoi(uint16tBuffer);
    writeUint16tForChannelToEepromBuffer(channelId, memorySlot, uint16tValue);
  }
}

void ServerController::updateFloatIfFound(uint16_t channelId,
                                          const char *buffer, const char *key,
                                          int memorySlot) {
  char floatBuffer[10] = "0";
  bool foundKey = getValueFromData(buffer, key, floatBuffer, 10);

  if (foundKey) {
    float floatValue = atof(floatBuffer);
    writeFloatForChannelToEepromBuffer(channelId, memorySlot, floatValue);
  }
}

void ServerController::updateChannel() {
  bool foundChannelId =
      getValueFromData(m_requestBuffer, "channelId=", m_channelIdBuffer, 5);

  if (!foundChannelId) {
    Serial.println(
        "Error: unable to find channelId, aborting updateChannel();");
    return;
  }

  uint16_t channelIdAsNumber = atoi(m_channelIdBuffer);

  // Worst case, each UTF8 character is 4 bytes long as url encoded
  char urlEncodedNameBuffer[MAX_CHANNEL_NAME_LENGTH * 4];
  getValueFromData(m_requestBuffer, "channelName=", urlEncodedNameBuffer,
                   MAX_CHANNEL_NAME_LENGTH * 4);
  urlDecode(urlEncodedNameBuffer, m_channelNameBuffer, MAX_CHANNEL_NAME_LENGTH);
  writeChannelNameFromChannelNameBufferToEepromBuffer(channelIdAsNumber);

  updateUint16tIfFound(channelIdAsNumber, m_requestBuffer,
                       "outputValue2=", MEM_SLOT_OUTPUT_VALUE2);

  updateBoolIfFound(channelIdAsNumber, m_requestBuffer,
                    "initialState=", MEM_SLOT_IS_START_VALUE_OUTPUT_VALUE2);

  updateBoolIfFound(channelIdAsNumber, m_requestBuffer,
                    "doRandomlySetValue2=", MEM_SLOT_DO_RANDOMLY_SET_VALUE2);

  updateUint8tIfFound(channelIdAsNumber, m_requestBuffer,
                      "frequencyValue2=", MEM_SLOT_RANDOMLY_SET_VALUE2_FREQ);

  updateBoolIfFound(channelIdAsNumber, m_requestBuffer,
                    "doRandomlySetValue1=", MEM_SLOT_DO_RANDOMLY_SET_VALUE1);

  updateUint8tIfFound(channelIdAsNumber, m_requestBuffer,
                      "frequencyValue1=", MEM_SLOT_RANDOMLY_SET_VALUE1_FREQ);

  updateBoolIfFound(channelIdAsNumber, m_requestBuffer,
                    "channelLinked=", MEM_SLOT_IS_LINKED);

  updateUint16tIfFound(channelIdAsNumber, m_requestBuffer,
                       "channelLinkDelay=", MEM_SLOT_LINK_DELAY);

  updateBoolIfFound(
      channelIdAsNumber, m_requestBuffer,
      "channelHiddenInCompactView=", MEM_SLOT_HIDE_IN_COMPACT_VIEW);

  updateBoolIfFound(channelIdAsNumber, m_requestBuffer,
                    "showSlider=", MEM_SLOT_SHOW_SLIDER);

  updateBoolIfFound(channelIdAsNumber, m_requestBuffer,
                    "useOutputValue1=", MEM_SLOT_USES_OUTPUT_VALUE1);

  updateUint16tIfFound(channelIdAsNumber, m_requestBuffer,
                       "outputValue1=", MEM_SLOT_OUTPUT_VALUE1);

  updateBoolIfFound(channelIdAsNumber, m_requestBuffer,
                    "channelLerped=", MEM_SLOT_IS_LERPED);

  updateFloatIfFound(channelIdAsNumber, m_requestBuffer,
                     "lerpSpeed=", MEM_SLOT_LERP_SPEED);

  char linkedChannelIdBuffer[4] = "0";
  getValueFromData(m_requestBuffer, "linkedChannelId=", linkedChannelIdBuffer,
                   4);

  uint16_t linkedChannelId = atoi(linkedChannelIdBuffer);

  if (m_stateManager->getToggleOneBasedAddresses()) {
    linkedChannelId--;
  }

  writeUint16tForChannelToEepromBuffer(
      channelIdAsNumber, MEM_SLOT_LINKED_CHANNEL, linkedChannelId);

  char isInitialStateOutputValue2Buffer[2] = {0};
  bool isInitialStateOutputValue2 = getValueFromData(
      m_requestBuffer, "outputValue2=", isInitialStateOutputValue2Buffer, 2);

  if (isInitialStateOutputValue2) {
    char outputValue2Buffer[7] = {0};
    getValueFromData(m_requestBuffer, "outputValue2=", outputValue2Buffer, 7);
    uint16_t outputValue2 = atoi(outputValue2Buffer);

    m_channelController->applyAndPropagateValue(channelIdAsNumber, outputValue2,
                                                100);
  } else {
    char outputValue1Buffer[7] = {0};
    bool foundOutputValue1 = getValueFromData(
        m_requestBuffer, "outputValue2=", outputValue1Buffer, 7);
    uint16_t outputValue1 = 0;

    if (foundOutputValue1) {
      outputValue1 = atoi(outputValue1Buffer);
    }

    m_channelController->applyAndPropagateValue(channelIdAsNumber, outputValue1,
                                                100);
  }

  writePageIntegrity(channelIdAsNumber + 1);
  writePageFromBufferToEeprom(channelIdAsNumber + 1);

  m_channelController->resetRecursionFlag();
}

void ServerController::processPostRequest(WiFiClient client) {
  if (isKeyInData(m_requestBuffer, "cancelChannelUpdate")) {
    cancelChannelUpdate();
  }

  if (isKeyInData(m_requestBuffer, "setCustomValue")) {
    setCustomValue();
  }

  if (isKeyInData(m_requestBuffer, "toggleOneBasedAddresses")) {
    toggleOneBasedAddresses();
  }

  if (isKeyInData(m_requestBuffer, "toggleCompactDisplay")) {
    toggleCompactDisplay();
  }

  if (isKeyInData(m_requestBuffer, "setChannelToValue1")) {
    setChannelToValue1();
  }

  if (isKeyInData(m_requestBuffer, "setChannelToValue2")) {
    setChannelToValue2();
  }

  if (isKeyInData(m_requestBuffer, "toggleForceAllOff")) {
    toggleForceAllOff();
  }

  if (isKeyInData(m_requestBuffer, "toggleForceAllOn")) {
    toggleForceAllOn();
  }

  if (isKeyInData(m_requestBuffer, "toggleRandomChaos")) {
    toggleRandomChaos();
  }

  if (isKeyInData(m_requestBuffer, "toggleRunningLights")) {
    toggleRunningLights();
  }

  if (isKeyInData(m_requestBuffer, "toggleRandomEvents")) {
    toggleRandomEvents();
  }

  if (isKeyInData(m_requestBuffer, "togglePropagateEvents")) {
    togglePropagateEvents();
  }

  if (isKeyInData(m_requestBuffer, "updateNumberOfChannels")) {
    updateNumberOfChannels();
  }

  if (isKeyInData(m_requestBuffer, "updateChannel")) {
    updateChannel();
  }

  if (isKeyInData(m_requestBuffer, "resetAllChannels")) {
    m_channelController->applyInitialState();
  }

  if (isKeyInData(m_requestBuffer, "setAllChannels")) {
    setAllChannels();
  }

  if (isKeyInData(m_requestBuffer, "turnEvenChannelsOn")) {
    m_channelController->turnEvenChannelsOn();
  }

  if (isKeyInData(m_requestBuffer, "turnOddChannelsOn")) {
    m_channelController->turnOddChannelsOn();
  }

  if (isKeyInData(m_requestBuffer, "countBinary")) {
    m_channelController->countBinary();
  }

  if (isKeyInData(m_requestBuffer, "toggleShowOptions")) {
    toggleShowOptions();
  }

  if (isKeyInData(m_requestBuffer, "toggleShowActions")) {
    toggleShowActions();
  }

  if (isKeyInData(m_requestBuffer, "toggleHighPwmBoard")) {
    toggleHighPwmBoard();
  }

  if (m_channelController->getFoundRecursion()) {
    replyToClientWithFail(client);
  } else {
    replyToClientWithSuccess(client);
  }
}

void ServerController::prepareRenderEditChannel(WiFiClient client,
                                                uint16_t channelId) {
  m_stateManager->setRenderEditChannel(true);
  m_stateManager->setChannelIdToEdit(channelId);
  readChannelNameFromEepromBufferToChannelNameBuffer(channelId);
  m_renderer->renderWebPage(client, m_channelController->getFoundRecursion());
}

void ServerController::prepareRenderChannels(WiFiClient client) {
  m_stateManager->setRenderEditChannel(false);
  m_renderer->renderWebPage(client, m_channelController->getFoundRecursion());
}

void ServerController::processGetRequest(WiFiClient client) {

  if (isArgInRequest(m_requestBuffer, " / ")) {
    // A get request for the main page
    prepareRenderChannels(client);
    return;
  } else if (isArgInRequest(m_requestBuffer, "/update")) {
    // A get request for the update page
    if (!isArgInRequest(m_requestBuffer, "channel")) {
      m_renderer->renderHttp400ErrorPage(client);
      return;
    }

    uint16_t channelId = getUint16tFromRequest(m_requestBuffer, "channel");

    if (m_stateManager->getToggleOneBasedAddresses()) {
      channelId--;
    }

    if (channelId >= MAX_TOTAL_CHANNELS) {
      m_renderer->renderHttp400ErrorPage(client);
      return;
    }

    prepareRenderEditChannel(client, channelId);
    return;
  }

  // An unknown get request
  m_renderer->renderHttp404ErrorPage(client);
  return;
}

void ServerController::processRequest(WiFiClient client) {
  m_stateManager->setRenderEditChannel(false);

  if (isArgInRequest(m_requestBuffer, "POST /")) {
    processPostRequest(client);
    return;
  }

  if (isArgInRequest(m_requestBuffer, "GET /")) {
    processGetRequest(client);
    return;
  }
}

void ServerController::loopEvent() {
  WiFiClient client = m_wifiServer.available();

  if (!client) {
    return;
  }

  this->clearRequestBuffer();
  int requestBufferIndex = 0;

  if (client.connected() == 0) {
    return;
  }

  while (client.available()) {
    char c = client.read();
    // Serial.write(c);

    m_requestBuffer[requestBufferIndex] = c;
    requestBufferIndex++;

    if (requestBufferIndex >= REQUEST_BUFFER_SIZE) {
      Serial.println("WARNING: REQUEST EXCEEDED BUFFER SIZE -> IGNORE!");
      m_renderer->renderHttp413ErrorPage(client);
      client.stop();
      return;
    }
  }

  processRequest(client);

  client.flush();
  client.stop();
}

// Function to extract value from form data using char arrays
// data: The form data as a char array
// key: The key as a char array
// value: A char array buffer where the extracted value will be stored
// valueLen: The length of the value buffer
bool ServerController::getValueFromData(const char *data, const char *key,
                                        char *value, int valueLen) {
  const char *startPtr = strstr(data, key);
  if (startPtr == NULL) {
    Serial.print("Unable to find key: ");
    Serial.println(key);
    return false;
  }

  startPtr += strlen(key); // Move pointer past the key

  const char *endPtr = strchr(startPtr, '&');
  if (endPtr == NULL) {
    endPtr =
        data + strlen(data); // Set endPtr to the end of data if '&' not found
  }

  int numCharsToCopy = endPtr - startPtr;
  if (numCharsToCopy >= valueLen) {
    numCharsToCopy = valueLen - 1; // Ensure we don't exceed buffer size
  }

  strncpy(value, startPtr, numCharsToCopy);
  value[numCharsToCopy] = '\0'; // Null-terminate the string

  return true;
}

bool ServerController::isKeyInData(const char *data, const char *key) {
  const char *startPtr = data;
  while ((startPtr = strstr(startPtr, key)) != NULL) {
    // Check if the key is at the start of data or preceded by '&' or '\n'
    bool atStart =
        startPtr == data || *(startPtr - 1) == '&' || *(startPtr - 1) == '\n';

    // Check if the key is followed by '=', '&' or is at the end of data
    const char *endPtr = startPtr + strlen(key);
    bool atEnd = *endPtr == '\0' || *endPtr == '&' || *endPtr == '=';

    if (atStart && atEnd) {
      return true;
    }

    // Move past this occurrence to check for another
    startPtr = endPtr;
  }

  // Key not found with proper boundaries
  return false;
}

void ServerController::clearRequestBuffer() {
  for (int i = 0; i < REQUEST_BUFFER_SIZE; i++) {
    m_requestBuffer[i] = '\0';
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

bool ServerController::isArgInRequest(char *request, char *arg) {
  char *pointerInRequest = strstr(request, arg);

  // We need to check if the parameter was on the first line to avoid triggers
  // from the referer header argument etc
  char *positionOfFirstNewline = strstr(request, "\r");
  if (positionOfFirstNewline == NULL || pointerInRequest == NULL ||
      positionOfFirstNewline < pointerInRequest) {
    return false;
  }

  return true;
}

uint16_t ServerController::getUint16tFromRequest(char *request, char *arg) {
  char *pointerInRequest = strstr(request, arg);

  // Move pointer to the start of the value
  pointerInRequest += strlen(arg) + 1;

  // Convert the value to an integer
  return atoi(pointerInRequest);
}
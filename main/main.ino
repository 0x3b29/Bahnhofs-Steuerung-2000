#include "eeprom.h"
#include "helpers.h"
#include "main.h"
#include "render.h"
#include <Adafruit_PWMServoDriver.h>
#include <ArduinoOTA.h>
#include <WiFiNINA.h>
#include <WiFiServer.h>

// If you check out this project, this file does not exist.
// You need to create a copy of example_arduino_secrets.h and rename it to
// arduino_secrets.h and fill in your WiFI name and password
#include "arduino_secrets.h"

#define RANDOM_INTERVAL 200
#define RANDOM_EVENT_INTERVAL 1000

WiFiServer server(80);
Adafruit_PWMServoDriver m_pwmBoards[PWM_BOARDS];

char m_pageBuffer[PAGE_BUFFER_SIZE];

uint16_t m_numChannels = 0;
uint8_t m_toggleRandomChaos = false;
uint8_t m_toggleForceAllOff = false;
uint8_t m_toggleForceAllOn = false;
uint8_t m_toggleOneBasedAddresses = false;
uint8_t m_toggleCompactDisplay = false;
uint8_t m_toggleRandomEvents = false;
uint8_t m_togglePropagateEvents = false;

char m_channelIdBuffer[4] = "0";

char m_channelIdToEditBuffer[4] = "";
uint16_t m_channelIdToEdit = 0;
bool m_renderNextPageWithOptionsVisible = true;
bool m_renderNextPageWithChannelEditVisible = false;

bool m_renderAnchor = false;
uint16_t m_anchorChannelId;

long m_lastRandom = 0;
long m_lastRandomEvent = 0;

bool m_foundRecursion = false;
uint8_t m_binaryCount = 0;

// Function to extract value from form data using char arrays
// formData: The form data as a char array
// key: The key as a char array
// value: A char array buffer where the extracted value will be stored
// valueLen: The length of the value buffer
void getValueFromData(const char *formData, const char *key, char *value,
                      int valueLen) {
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

bool isKeyInData(const char *formData, const char *key) {
  const char *startPtr = strstr(formData, key);
  if (startPtr == NULL) {
    return false;
  }

  return true;
}

void replyToClientWithSuccess(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/plain");
  client.println();
  client.println("Settings updated successfully.");
}

void applyAndPropagateValue(int channel, uint16_t brightness) {
  setChannelBrightness(channel, brightness);

  if (m_togglePropagateEvents && !m_toggleForceAllOff && !m_toggleForceAllOn &&
      !m_toggleRandomChaos) {
    commandLinkedChannel(channel, brightness, 0, 5);
  }
}

void setChannelBrightness(int channel, uint16_t brightness) {
  int boardIndex = getBoardIndexForChannel(channel);
  int subAddress = getBoardSubAddressForChannel(channel);

  if (m_toggleForceAllOff == true) {
    m_pwmBoards[boardIndex].setPWM(subAddress, 0, 0);
    return;
  }

  if (m_toggleForceAllOn) {
    m_pwmBoards[boardIndex].setPWM(subAddress, 0, 4095);
    return;
  }

  if (m_toggleRandomChaos == true) {
    m_pwmBoards[boardIndex].setPWM(subAddress, 0, random(0, 4095));
    return;
  }

  m_pwmBoards[boardIndex].setPWM(subAddress, 0, brightness);
}

void applyInitialState() {
  m_binaryCount = 0;
  
  for (int i = 0; i < m_numChannels; i++) {

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

void turnAllChannelsOff() {
  for (int i = 0; i < m_numChannels; i++) {
    setChannelBrightness(i, 0);
  }
}

void turnAllChannels50() {
  for (int i = 0; i < m_numChannels; i++) {
    setChannelBrightness(i, 2048);
  }
}

void turnAllChannels100() {
  for (int i = 0; i < m_numChannels; i++) {
    setChannelBrightness(i, 4095);
  }
}

void turnEvenChannelsOn() {
  for (int i = 0; i < m_numChannels; i++) {

    int userFacingAddress = i;

    if (m_toggleOneBasedAddresses) {
      userFacingAddress++;
    }

    if (userFacingAddress % 2 == 0) {
      setChannelBrightness(i, 4095);
    } else {
      setChannelBrightness(i, 0);
    }
  }
}

void turnOddChannelsOn() {
  for (int i = 0; i < m_numChannels; i++) {

    int userFacingAddress = i;

    if (m_toggleOneBasedAddresses) {
      userFacingAddress++;
    }

    if (userFacingAddress % 2 == 1) {
      setChannelBrightness(i, 4095);
    } else {
      setChannelBrightness(i, 0);
    }
  }
}

void countBinary() {
  for (int i = 0; i < m_numChannels; i++) {
    if ((m_binaryCount & (1 << i)) == 0) {
      setChannelBrightness(i, 0);
    } else {
      setChannelBrightness(i, 4095);
    }
  }
  m_binaryCount++;
}

void clearPageBuffer() {
  for (int i = 0; i < PAGE_BUFFER_SIZE; i++) {
    m_pageBuffer[i] = '\0';
  }
}

void urlDecode(const char *urlEncoded, char *decoded, int maxLen) {
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

void processRequest(WiFiClient client) {
  bool shouldRerender = false;
  m_renderAnchor = false;

  if (strstr(m_pageBuffer, "POST") != NULL) {
    if (isKeyInData(m_pageBuffer, "ignoreChannel")) {
      // User clicked on "Verwerfen" button
      m_renderNextPageWithOptionsVisible = true;
      m_renderNextPageWithChannelEditVisible = false;

      // Reload old brigthness
      getValueFromData(m_pageBuffer, "channelId=", m_channelIdBuffer, 5);
      uint16_t channelIdAsNumber = atoi(m_channelIdBuffer);

      uint16_t originalBrightness = readUint16tForChannelFromEepromBuffer(
          channelIdAsNumber, MEM_SLOT_BRIGHTNESS);

      setChannelBrightness(channelIdAsNumber, originalBrightness);

      shouldRerender = true;
    }

    if (isKeyInData(m_pageBuffer, "testBrightness")) {
      // User changed brightness on edit channel form
      getValueFromData(m_pageBuffer, "channelId=", m_channelIdBuffer, 5);
      uint16_t channelIdAsNumber = atoi(m_channelIdBuffer);

      char channelBrightnessBuffer[5] = "0";
      getValueFromData(m_pageBuffer,
                       "channelBrightness=", channelBrightnessBuffer, 5);
      uint16_t channelBrightness = atoi(channelBrightnessBuffer);

      setChannelBrightness(channelIdAsNumber, channelBrightness);
    }

    if (isKeyInData(m_pageBuffer, "toggleOneBasedAddresses")) {
      char toggleOneBasedAddressesBuffer[2] = "0";
      getValueFromData(m_pageBuffer, "toggleOneBasedAddresses=",
                       toggleOneBasedAddressesBuffer, 2);
      m_toggleOneBasedAddresses = atoi(toggleOneBasedAddressesBuffer);
      writeToEepromBuffer(MEM_SLOT_ONE_BASED_ADDRESSES,
                          &m_toggleOneBasedAddresses, 1);

      writePageIntegrity(0);
      writePageFromBufferToEeprom(0);
    }

    if (isKeyInData(m_pageBuffer, "toggleCompactDisplay")) {
      char toggleCompactDisplayBuffer[2] = "0";
      getValueFromData(m_pageBuffer,
                       "toggleCompactDisplay=", toggleCompactDisplayBuffer, 2);
      m_toggleCompactDisplay = atoi(toggleCompactDisplayBuffer);
      writeToEepromBuffer(MEM_SLOT_COMPACT_DISPLAY, &m_toggleCompactDisplay, 1);

      writePageIntegrity(0);
      writePageFromBufferToEeprom(0);
    }

    if (isKeyInData(m_pageBuffer, "turnChannelOff")) {
      char turnChannelOffIdBuffer[4];
      uint16_t turnChannelOffId;
      getValueFromData(m_pageBuffer, "turnChannelOff=", turnChannelOffIdBuffer,
                       4);
      turnChannelOffId = atoi(turnChannelOffIdBuffer);
      applyAndPropagateValue(turnChannelOffId, 0);
    }

    if (isKeyInData(m_pageBuffer, "turnChannelOn")) {
      char turnChannelOnIdBuffer[4];
      uint16_t turnChannelOnId;
      getValueFromData(m_pageBuffer, "turnChannelOn=", turnChannelOnIdBuffer,
                       4);
      turnChannelOnId = atoi(turnChannelOnIdBuffer);

      uint16_t turnOnBrightness = readUint16tForChannelFromEepromBuffer(
          turnChannelOnId, MEM_SLOT_BRIGHTNESS);

      applyAndPropagateValue(turnChannelOnId, turnOnBrightness);
    }

    if (isKeyInData(m_pageBuffer, "editChannel")) {
      Serial.println("PREPARE CHANNEL FORM");

      getValueFromData(m_pageBuffer, "editChannel=", m_channelIdToEditBuffer,
                       4);

      m_renderNextPageWithOptionsVisible = false;
      m_renderNextPageWithChannelEditVisible = true;

      m_channelIdToEdit = atoi(m_channelIdToEditBuffer);
      readChannelNameFromEepromBufferToChannelNameBuffer(m_channelIdToEdit);

      shouldRerender = true;
    }

    if (isKeyInData(m_pageBuffer, "toggleForceAllOff")) {
      char toggleForceAllOffBuffer[2] = "0";
      getValueFromData(m_pageBuffer,
                       "toggleForceAllOff=", toggleForceAllOffBuffer, 2);
      m_toggleForceAllOff = atoi(toggleForceAllOffBuffer);
      writeToEepromBuffer(MEM_SLOT_FORCE_ALL_OFF, &m_toggleForceAllOff, 1);

      writePageIntegrity(0);
      writePageFromBufferToEeprom(0);
      applyInitialState();
    }

    if (isKeyInData(m_pageBuffer, "toggleForceAllOn")) {
      char toggleForceAllOnBuffer[2] = "0";
      getValueFromData(m_pageBuffer,
                       "toggleForceAllOn=", toggleForceAllOnBuffer, 2);
      m_toggleForceAllOn = atoi(toggleForceAllOnBuffer);
      writeToEepromBuffer(MEM_SLOT_FORCE_ALL_ON, &m_toggleForceAllOn, 1);

      writePageIntegrity(0);
      writePageFromBufferToEeprom(0);
      applyInitialState();
    }

    if (isKeyInData(m_pageBuffer, "toggleRandomChaos")) {
      char toggleRandomChaosBuffer[2] = "0";
      getValueFromData(m_pageBuffer,
                       "toggleRandomChaos=", toggleRandomChaosBuffer, 2);
      m_toggleRandomChaos = atoi(toggleRandomChaosBuffer);
      writeToEepromBuffer(MEM_SLOT_RANDOM_CHAOS, &m_toggleRandomChaos, 1);

      writePageIntegrity(0);
      writePageFromBufferToEeprom(0);
    }

    if (isKeyInData(m_pageBuffer, "toggleRandomEvents")) {
      char toggleRandomEventsBuffer[2] = "0";
      getValueFromData(m_pageBuffer,
                       "toggleRandomEvents=", toggleRandomEventsBuffer, 2);
      m_toggleRandomEvents = atoi(toggleRandomEventsBuffer);
      writeToEepromBuffer(MEM_SLOT_RANDOM_EVENTS, &m_toggleRandomEvents, 1);

      writePageIntegrity(0);
      writePageFromBufferToEeprom(0);
    }

    if (isKeyInData(m_pageBuffer, "togglePropagateEvents")) {
      char togglePropagateEventsBuffer[2] = "0";
      getValueFromData(m_pageBuffer,
                       "togglePropagateEvents=", togglePropagateEventsBuffer,
                       2);
      m_togglePropagateEvents = atoi(togglePropagateEventsBuffer);
      writeToEepromBuffer(MEM_SLOT_PROPAGATE_EVENTS, &m_togglePropagateEvents,
                          1);

      writePageIntegrity(0);
      writePageFromBufferToEeprom(0);
    }

    if (isKeyInData(m_pageBuffer, "updateSettings")) {
      char clearEepromBuffer[10] = "";
      getValueFromData(m_pageBuffer, "clearEeprom=", clearEepromBuffer, 10);

      // TODO: move reset2024 to secrets file
      if (strcmp(clearEepromBuffer, "reset2024") == 0) {
        Serial.println("Clearing Eeprom!!!");
        clearEeprom();
      }

      uint16_t oldNumChannels = m_numChannels;

      m_renderNextPageWithOptionsVisible = true;
      m_renderNextPageWithChannelEditVisible = false;

      char numChannelsBuffer[4] = "0";
      getValueFromData(m_pageBuffer, "numChannels=", numChannelsBuffer, 4);
      m_numChannels = atoi(numChannelsBuffer);

      writeUInt16ToEepromBuffer(MEM_SLOT_CHANNELS, m_numChannels);

      writePageIntegrity(0);
      writePageFromBufferToEeprom(0);

      // If we have new channels, we load them, check them and wipe them if
      // nessesary
      if (oldNumChannels < m_numChannels) {
        for (int i = oldNumChannels; i < m_numChannels; i++) {
          Serial.print("Checking page for channel: ");
          Serial.println(i);
          loadPageAndCheckIntegrity(i + 1);
        }
      }

      shouldRerender = true;
    }

    if (isKeyInData(m_pageBuffer, "updateChannel")) {
      Serial.println("UPDATE CHANNEL");

      m_renderNextPageWithOptionsVisible = true;
      m_renderNextPageWithChannelEditVisible = false;

      getValueFromData(m_pageBuffer, "channelId=", m_channelIdBuffer, 5);
      uint16_t channelIdAsNumber = atoi(m_channelIdBuffer);

      char urlEncodedNameBuffer[21];
      getValueFromData(m_pageBuffer, "channelName=", urlEncodedNameBuffer, 21);
      urlDecode(urlEncodedNameBuffer, m_channelNameBuffer, 20);

      char channelBrightnessBuffer[5] = "0";
      getValueFromData(m_pageBuffer,
                       "channelBrightness=", channelBrightnessBuffer, 5);

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
      getValueFromData(m_pageBuffer, "linkedChannelId=", linkedChannelIdBuffer,
                       4);

      uint8_t randomOn = atoi(randomOnBuffer);
      uint8_t randomOnFreq = atoi(randomOnFreqBuffer);

      uint8_t randomOff = atoi(randomOffBuffer);
      uint8_t randomOffFreq = atoi(randomOffFreqBuffer);

      uint8_t isLinked = atoi(isLinkedBuffer);
      uint16_t linkedChannelId = atoi(linkedChannelIdBuffer);

      if (m_toggleOneBasedAddresses) {
        linkedChannelId--;
      }

      Serial.print("Channel ");
      Serial.print(channelIdAsNumber);

      uint16_t startAddress = 64 + channelIdAsNumber * 64;
      Serial.print(" got startAddress ");
      Serial.println(startAddress);

      uint16_t channelBrightness = atoi(channelBrightnessBuffer);

      writeChannelNameFromChannelNameBufferToEepromBuffer(channelIdAsNumber);
      writeUint16tForChannelToEepromBuffer(
          channelIdAsNumber, MEM_SLOT_BRIGHTNESS, channelBrightness);

      writeUint8tToEepromBuffer(channelIdAsNumber, MEM_SLOT_RANDOM_ON,
                                randomOn);
      if (randomOn == 1) {
        writeUint8tToEepromBuffer(channelIdAsNumber, MEM_SLOT_RANDOM_ON_FREQ,
                                  randomOnFreq);
      }

      writeUint8tToEepromBuffer(channelIdAsNumber, MEM_SLOT_RANDOM_OFF,
                                randomOff);
      if (randomOff == 1) {
        writeUint8tToEepromBuffer(channelIdAsNumber, MEM_SLOT_RANDOM_OFF_FREQ,
                                  randomOffFreq);
      }

      writeUint8tToEepromBuffer(channelIdAsNumber, MEM_SLOT_IS_LINKED,
                                isLinked);
      if (isLinked == 1) {
        writeUint16tForChannelToEepromBuffer(
            channelIdAsNumber, MEM_SLOT_LINKED_CHANNEL, linkedChannelId);
      }

      applyAndPropagateValue(channelIdAsNumber, channelBrightness);

      writePageIntegrity(channelIdAsNumber + 1);
      writePageFromBufferToEeprom(channelIdAsNumber + 1);

      shouldRerender = true;
      m_renderAnchor = true;
      m_anchorChannelId = channelIdAsNumber;
      m_foundRecursion = false;
    }

    if (isKeyInData(m_pageBuffer, "resetAllChannels")) {
      applyInitialState();
    }

    if (isKeyInData(m_pageBuffer, "turnAllChannelsOff")) {
      turnAllChannelsOff();
    }

    if (isKeyInData(m_pageBuffer, "turnAllChannels50")) {
      turnAllChannels50();
    }

    if (isKeyInData(m_pageBuffer, "turnAllChannels100")) {
      turnAllChannels100();
    }

    if (isKeyInData(m_pageBuffer, "turnEvenChannelsOn")) {
      turnEvenChannelsOn();
    }

    if (isKeyInData(m_pageBuffer, "turnOddChannelsOn")) {
      turnOddChannelsOn();
    }

    if (isKeyInData(m_pageBuffer, "countBinary")) {
      countBinary();
    }

    if (shouldRerender) {
      renderWebPage(
          client, m_foundRecursion, m_renderNextPageWithOptionsVisible,
          m_renderNextPageWithChannelEditVisible, m_renderAnchor,
          m_anchorChannelId, m_numChannels, m_toggleOneBasedAddresses,
          m_toggleCompactDisplay, m_toggleForceAllOff, m_toggleForceAllOn,
          m_toggleRandomChaos, m_toggleRandomEvents, m_togglePropagateEvents,
          m_channelIdToEdit);
    } else {
      replyToClientWithSuccess(client);
    }

    // dumpEepromData(0, MAX_EEPROM_RANGE - 1);
  } else {
    sn("processRequest NOT POST");
    // For get requests, we always want to render the page to the client if its
    // not for the favicon

    renderWebPage(client, m_foundRecursion, m_renderNextPageWithOptionsVisible,
                  m_renderNextPageWithChannelEditVisible, m_renderAnchor,
                  m_anchorChannelId, m_numChannels, m_toggleOneBasedAddresses,
                  m_toggleCompactDisplay, m_toggleForceAllOff,
                  m_toggleForceAllOn, m_toggleRandomChaos, m_toggleRandomEvents,
                  m_togglePropagateEvents, m_channelIdToEdit);
  }
}

void loadOptionsToMemberVariables() {
  m_numChannels = readUInt16FromEepromBuffer(MEM_SLOT_CHANNELS);
  readFromEepromBuffer(MEM_SLOT_FORCE_ALL_OFF, &m_toggleForceAllOff, 1);
  readFromEepromBuffer(MEM_SLOT_FORCE_ALL_ON, &m_toggleForceAllOn, 1);
  readFromEepromBuffer(MEM_SLOT_RANDOM_CHAOS, &m_toggleRandomChaos, 1);
  readFromEepromBuffer(MEM_SLOT_ONE_BASED_ADDRESSES, &m_toggleOneBasedAddresses,
                       1);
  readFromEepromBuffer(MEM_SLOT_COMPACT_DISPLAY, &m_toggleCompactDisplay, 1);
  readFromEepromBuffer(MEM_SLOT_RANDOM_EVENTS, &m_toggleRandomEvents, 1);
  readFromEepromBuffer(MEM_SLOT_PROPAGATE_EVENTS, &m_togglePropagateEvents, 1);
}

bool shouldInvokeEvent(uint8_t freq) {
  // Generate a random number between 0 and 3599
  uint16_t randNumber = random(3600);

  // Check if the random number is less than the threshold
  return randNumber < freq;
}

void commandLinkedChannel(uint16_t commandingChannelId, uint16_t brightness,
                          int depth, int maxDepth) {

  if (depth > maxDepth) {
    st("Detected and broke recusrion for commandingChannelId ");
    sn(commandingChannelId);

    m_foundRecursion = true;
    return;
  }

  for (uint16_t i = 0; i < m_numChannels; i++) {
    if (i == commandingChannelId) {
      continue;
    }

    bool isChannelLinked =
        readBoolForChannelFromEepromBuffer(i, MEM_SLOT_IS_LINKED);

    if (isChannelLinked == true) {

      uint16_t linkedChannelId =
          readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_LINKED_CHANNEL);

      if (linkedChannelId == commandingChannelId) {

        setChannelBrightness(i, brightness);

        commandLinkedChannel(i, brightness, depth + 1, maxDepth);
      }
    }
  }
}

void calculateRandomEvents() {
  for (int i = 0; i < m_numChannels; i++) {
    bool randomOn = readBoolForChannelFromEepromBuffer(i, MEM_SLOT_RANDOM_ON);
    bool isLinked = readBoolForChannelFromEepromBuffer(i, MEM_SLOT_RANDOM_ON);
    uint16_t linkedChannel =
        readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_LINKED_CHANNEL);

    // No need for further checks if channel has no random events
    if (!randomOn) {
      continue;
    }

    uint8_t randomOnFreq =
        readUint8tForChannelFromEepromBuffer(i, MEM_SLOT_RANDOM_ON_FREQ);

    uint8_t randomOffFreq =
        readUint8tForChannelFromEepromBuffer(i, MEM_SLOT_RANDOM_OFF_FREQ);

    bool turnOn = shouldInvokeEvent(randomOnFreq);
    bool turnOff = shouldInvokeEvent(randomOffFreq);

    if (turnOn) {
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
    }

    if (turnOff) {
      st("Got random off event for channel ");
      sn(i);

      applyAndPropagateValue(i, 0);

      if (isLinked) {
        applyAndPropagateValue(linkedChannel, 0);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting");
  Wire.begin();

  // Comment in for HARD RESET
  // clearEeprom();

  clearEepromBuffer();
  clearPageBuffer();

  // Load first page which contains information such as channel count etc...
  loadPageAndCheckIntegrity(0);

  loadOptionsToMemberVariables();

  for (int i = 0; i < m_numChannels; i++) {
    // Page for channel is always channelId + 1 since page 0 = general config;
    loadPageAndCheckIntegrity(i + 1);
  }

  // dumpEepromData(0, MAX_EEPROM_RANGE - 1);

  // Initializte pwm boards
  for (int i = 0; i < PWM_BOARDS; i++) {
    int pwmAddress = 0x40 + i;

    m_pwmBoards[i] = Adafruit_PWMServoDriver(pwmAddress);
    m_pwmBoards[i].begin();
    m_pwmBoards[i].setOscillatorFrequency(27000000);
    m_pwmBoards[i].setPWMFreq(PWM_REFRESH_RATE);

    Serial.print("Board added: ");
    Serial.println(pwmAddress);
  }

  WiFi.begin(SECRET_SSID, SECRET_PASS);
  Serial.println("Attempting to connect to WiFi network...");
  delay(500);
  WiFi.begin(SECRET_SSID, SECRET_PASS);

  int tries = 0;
  // Attempt to connect to WiFi network
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Waiting for WiFi network...");
    //

    if (tries > 10) {
      Serial.println("Re-attempting to connect to WiFi network...");
      WiFi.begin(SECRET_SSID, SECRET_PASS);
      tries = 0;
    }

    delay(1000);
    tries++;
  }

  Serial.println("Connected to WiFi");

  // Print the IP address
  Serial.print("Server IP Address: ");
  Serial.println(WiFi.localIP());

  // Start the server
  server.begin();

  ArduinoOTA.onStart([]() { Serial.println("Start OTA"); });

  // start the WiFi OTA library with internal (flash) based storage
  ArduinoOTA.begin(WiFi.localIP(), "Arduino_MKR_WiFi_1010", SECRET_OTA,
                   InternalStorage);

  Serial.println("Server started");

  float analogValue = analogRead(0);

  st("Priming randomSeed with ");
  sn(analogValue);
  randomSeed(analogValue);

  applyInitialState();
}

void loop() {
  // check for WiFi OTA updates
  ArduinoOTA.poll();

  WiFiClient client = server.available(); // Listen for incoming clients

  if ((m_toggleRandomChaos == 1) &&
      (millis() > (m_lastRandom + RANDOM_INTERVAL))) {
    m_lastRandom = millis();
    applyInitialState();
  }

  if ((m_toggleRandomEvents == 1) &&
      millis() > (m_lastRandomEvent + RANDOM_EVENT_INTERVAL)) {
    m_lastRandomEvent = millis();
    calculateRandomEvents();
  }

  if (client) {
    boolean currentLineIsBlank = true;

    clearPageBuffer();
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
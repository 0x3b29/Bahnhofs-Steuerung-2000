#include "eeprom.h"
#include "helpers.h"
#include "led_controller.h"
#include "main.h"
#include "render.h"
#include "state_manager.h"
#include "server_controller.h"
#include <ArduinoOTA.h>
#include <WiFiNINA.h>

// If you check out this project, this file does not exist.
// You need to create a copy of example_arduino_secrets.h and rename it to
// arduino_secrets.h and fill in your WiFI name and password
#include "arduino_secrets.h"

#define RANDOM_INTERVAL 200
#define RANDOM_EVENT_INTERVAL 1000

StateManager m_stateManager;
LedController m_ledController;
ServerController m_serverController(&m_ledController);

uint16_t m_numChannels = 0;
uint8_t m_toggleRandomChaos = false;
uint8_t m_toggleForceAllOff = false;
uint8_t m_toggleForceAllOn = false;
uint8_t m_toggleOneBasedAddresses = false;
uint8_t m_toggleCompactDisplay = false;
uint8_t m_toggleRandomEvents = false;
uint8_t m_togglePropagateEvents = false;
uint8_t m_toggleShowOptions = false;
uint8_t m_toggleShowActions = false;

char m_channelIdBuffer[4] = "0";

char m_channelIdToEditBuffer[4] = "";
uint16_t m_channelIdToEdit = 0;
bool m_renderNextPageWithOptionsVisible = true;
bool m_renderNextPageWithChannelEditVisible = false;

bool m_renderAnchor = false;
uint16_t m_anchorChannelId;

long m_lastRandom = 0;
long m_lastRandomEvent = 0;

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
  readFromEepromBuffer(MEM_SLOT_SHOW_OPTIONS, &m_toggleShowOptions, 1);
  readFromEepromBuffer(MEM_SLOT_SHOW_ACTIONS, &m_toggleShowActions, 1);

  m_ledController.setNumChannels(m_numChannels);
  m_ledController.setToggleRandomChaos(m_toggleRandomChaos);
  m_ledController.setToggleForceAllOff(m_toggleForceAllOff);
  m_ledController.setToggleForceAllOn(m_toggleForceAllOn);
  m_ledController.setToggleOneBasedAddresses(m_toggleOneBasedAddresses);
  m_ledController.setTogglePropagateEvents(m_togglePropagateEvents);
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting");
  Wire.begin();

  // Comment in for HARD RESET
  // clearEeprom();

  clearEepromBuffer();

  // Load first page which contains information such as channel count etc...
  loadPageAndCheckIntegrity(0);

  loadOptionsToMemberVariables();

  for (int i = 0; i < m_numChannels; i++) {
    // Page for channel is always channelId + 1 since page 0 = general config;
    loadPageAndCheckIntegrity(i + 1);
  }

  // dumpEepromData(0, MAX_EEPROM_RANGE - 1);

  m_ledController.initializePwmBoards();

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

  // Start the m_wifiServer
  m_serverController.begin();

  ArduinoOTA.onStart([]() { Serial.println("Start OTA"); });

  // start the WiFi OTA library with internal (flash) based storage
  ArduinoOTA.begin(WiFi.localIP(), "Arduino_MKR_WiFi_1010", SECRET_OTA,
                   InternalStorage);

  Serial.println("Server started");

  float analogValue = analogRead(0);

  st("Priming randomSeed with ");
  sn(analogValue);
  randomSeed(analogValue);

  m_ledController.applyInitialState();
}

void loop() {
  // check for WiFi OTA updates
  ArduinoOTA.poll();

  if ((m_toggleRandomChaos == 1) &&
      (millis() > (m_lastRandom + RANDOM_INTERVAL))) {
    m_lastRandom = millis();
    m_ledController.setEveryChannelToRandomValue();
  }

  if ((m_toggleRandomEvents == 1) &&
      millis() > (m_lastRandomEvent + RANDOM_EVENT_INTERVAL)) {
    m_lastRandomEvent = millis();
    m_ledController.calculateRandomEvents();
  }

  m_serverController.loopEvent();
}
#include "channel_controller.h"
#include "eeprom.h"
#include "helpers.h"
#include "bahnhofssteuerung2000.h"
#include "render.h"
#include "server_controller.h"
#include "state_manager.h"
#include <WiFiNINA.h>

// If you check out this project, this file does not exist.
// You need to create a copy of example_arduino_secrets.h and rename it to
// arduino_secrets.h and fill in your WiFI name and password
#include "arduino_secrets.h"

#define RANDOM_INTERVAL 1000
#define RUNNING_LIGHT_INTERVAL 25
#define RANDOM_EVENT_INTERVAL 100
#define HEARTBEAT_INTERVAL 500

StateManager m_stateManager;
ChannelController m_channelController(&m_stateManager);
Renderer m_renderer(&m_stateManager);
ServerController m_serverController(&m_stateManager, &m_channelController,
                                    &m_renderer);

long m_lastRandom = 0;
long m_lastRandomEvent = 0;
long m_lastRunningLightEvent = 0;

long m_lastHeartbeatEvent = 0;
bool m_lastHeartbeatState = false;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting");

  Serial.print("Using ");
  Serial.print(MAX_EEPROM_RANGE);
  Serial.println(" bytes for eeprom mirror.");

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  Wire.begin();

  // Comment in for HARD RESET
  // clearEeprom();

  // Comment in to dump the eeprom to serial
  // dumpEepromData(0, MAX_EEPROM_RANGE - 1);

  clearEepromBuffer();

  // Load first page which contains information such as channel count etc...
  loadPageFromEepromToEepromBufferAndCheckIntegrity(0);
  m_stateManager.loadStateFromEepromBuffer();

  for (int i = 0; i < m_stateManager.getNumChannels(); i++) {
    // Page for channel is always channelId + 1 since page 0 = general config;
    loadPageFromEepromToEepromBufferAndCheckIntegrity(i + 1);
  }

  m_channelController.initializePwmBoards();

  WiFi.begin(SECRET_SSID, SECRET_PASS);
  Serial.println("Attempting to connect to WiFi network...");
  delay(500);
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  int tries = 0;
  // Attempt to connect to WiFi network
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Waiting for WiFi network...");
    //

    if (tries > 5) {
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
  Serial.println("Server started");

  float analogValue = analogRead(0);

  st("Priming randomSeed with ");
  sn(analogValue);
  randomSeed(analogValue);

  m_channelController.applyInitialState();
  m_channelController.initializePreviousMillis();
}

void loop() {
  if ((m_stateManager.getToggleRandomChaos() == 1) &&
      (millis() > (m_lastRandom + RANDOM_INTERVAL))) {
    m_lastRandom = millis();
    m_channelController.setEveryChannelToRandomValue();
  }

  if ((m_stateManager.getToggleRandomEvents() == 1) &&
      millis() > (m_lastRandomEvent + RANDOM_EVENT_INTERVAL)) {
    m_lastRandomEvent = millis();
    m_channelController.calculateRandomEvents();
  }

  if ((m_stateManager.getToggleRunningLights() == 1) &&
      millis() > (m_lastRunningLightEvent + RUNNING_LIGHT_INTERVAL)) {
    m_lastRunningLightEvent = millis();
    m_channelController.setNextRunningLight();
  }

  if (millis() > (m_lastHeartbeatEvent + HEARTBEAT_INTERVAL)) {
    m_lastHeartbeatEvent = millis();

    if (m_lastHeartbeatState == true) {
      m_lastHeartbeatState = false;
    } else {
      m_lastHeartbeatState = true;
    }

    digitalWrite(LED_BUILTIN, m_lastHeartbeatState);
  }

  m_channelController.loopEvent();
  m_serverController.loopEvent();
}
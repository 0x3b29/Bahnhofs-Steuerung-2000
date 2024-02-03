#ifndef server_controller_h
#define server_controller_h

#include "led_controller.h"
#include <Arduino.h>
#include <WiFiNINA.h>
#include <WiFiServer.h>

class ServerController {

private:
  bool m_x;
  LedController *m_ledController;

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

  void replyToClientWithSuccess(WiFiClient client);
  void replyToClientWithFail(WiFiClient client);
  void processRequest(WiFiClient client);
  void getValueFromData(const char *formData, const char *key, char *value,
                        int valueLen);
  bool isKeyInData(const char *formData, const char *key);
  void clearPageBuffer();
  void urlDecode(const char *urlEncoded, char *decoded, int maxLen);

public:
  ServerController(LedController *ledController);
  void begin();
  void loopEvent();
};

#endif // web_h
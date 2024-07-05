#ifndef server_controller_h
#define server_controller_h

#include "led_controller.h"
#include "render.h"
#include "state_manager.h"
#include <Arduino.h>
#include <WiFiNINA.h>
#include <WiFiServer.h>

class ServerController {

private:
  LedController *m_ledController;
  StateManager *m_stateManager;
  Renderer *m_renderer;

  char m_channelIdBuffer[4] = "0";
  char m_channelIdToEditBuffer[4] = "";

  void replyToClientWithSuccess(WiFiClient client);
  void replyToClientWithFail(WiFiClient client);

  void cancelChannelUpdate();
  void toggleOneBasedAddresses();
  void testBrightness();
  void updateChannel();
  void toggleCompactDisplay();
  void turnChannelOff();
  void turnChannelOn();
  void editChannel();
  void toggleForceAllOff();
  void toggleForceAllOn();
  void toggleRandomChaos();
  void toggleRunningLights();
  void toggleRandomEvents();
  void togglePropagateEvents();
  void updateNumberOfChannels();
  void toggleShowOptions();
  void toggleShowActions();
  void setAllChannels();

  void processRequest(WiFiClient client);
  void sendHttp413Response(WiFiClient client);

  void getValueFromData(const char *formData, const char *key, char *value,
                        int valueLen);
  bool isKeyInData(const char *formData, const char *key);
  void clearRequestBuffer();
  void urlDecode(const char *urlEncoded, char *decoded, int maxLen);

public:
  ServerController(StateManager *stateManager, LedController *ledController,
                   Renderer *renderer);
  void begin();
  void loopEvent();
};

#endif // server_controller_h
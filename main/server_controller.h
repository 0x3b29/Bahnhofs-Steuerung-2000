#ifndef server_controller_h
#define server_controller_h

#include "channel_controller.h"
#include "render.h"
#include "state_manager.h"
#include <Arduino.h>
#include <WiFiNINA.h>
#include <WiFiServer.h>

class ServerController {

private:
  ChannelController *m_channelController;
  StateManager *m_stateManager;
  Renderer *m_renderer;

  char m_channelIdBuffer[4] = "0";
  char m_channelIdToEditBuffer[4] = "";

  void replyToClientWithSuccess(WiFiClient client);
  void replyToClientWithFail(WiFiClient client);

  void cancelChannelUpdate();
  void toggleOneBasedAddresses();
  void setCustomValue();
  void updateChannel();
  void toggleCompactDisplay();
  void setChannelToValue1();
  void setChannelToValue2();
  void toggleForceAllOff();
  void toggleForceAllOn();
  void toggleRandomChaos();
  void toggleRunningLights();
  void toggleRandomEvents();
  void togglePropagateEvents();
  void updateNumberOfChannels();
  void toggleShowOptions();
  void toggleShowActions();
  void toggleHighPwmBoard();
  void setAllChannels();

  void prepareRenderChannels(WiFiClient client);
  void prepareRenderEditChannel(WiFiClient client, uint16_t channelId);

  void processPostRequest(WiFiClient client);
  void processGetRequest(WiFiClient client);
  void processRequest(WiFiClient client);

  void updateBoolIfFound(uint16_t channelId, const char *buffer,
                         const char *key, int memorySlot);
  void updateUint8tIfFound(uint16_t channelId, const char *buffer,
                           const char *key, int memorySlot);
  void updateUint16tIfFound(uint16_t channelId, const char *buffer,
                            const char *key, int memorySlot);
  void updateFloatIfFound(uint16_t channelId, const char *buffer,
                          const char *key, int memorySlot);

  bool getValueFromData(const char *data, const char *key, char *value,
                        int valueLen);

  bool isKeyInData(const char *data, const char *key);
  void clearRequestBuffer();
  void urlDecode(const char *urlEncoded, char *decoded, int maxLen);

  bool isArgInRequest(char *request, char *arg);
  uint16_t getUint16tFromRequest(char *request, char *arg);

public:
  ServerController(StateManager *stateManager,
                   ChannelController *channelController, Renderer *renderer);
  void begin();
  void loopEvent();
};

#endif // server_controller_h
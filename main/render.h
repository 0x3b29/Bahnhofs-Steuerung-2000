#ifndef render_h
#define render_h

#include "main.h"
#include "state_manager.h"
#include <Arduino.h>
#include <WiFiNINA.h>

#define pt client.print
#define pn client.println

class Renderer {
private:
  StateManager *m_stateManager;
  void renderActionsHeading(WiFiClient client);
  void renderOptionsHeading(WiFiClient client);
  void renderHeadJavascript(WiFiClient client);

  void renderChannelDetailCompact(WiFiClient client, uint16_t channelId,
                                  bool renderHorizontalRule);

  void renderChannelDetail(WiFiClient client, uint16_t channelId,
                           bool renderHorizontalRule);

  void renderEditChannel(WiFiClient client);

  void renderActions(WiFiClient client);
  void renderOptions(WiFiClient client);

public:
  Renderer(StateManager *stateManager);
  void renderWebPage(WiFiClient client, bool foundRecursion);
};
#endif // render_h
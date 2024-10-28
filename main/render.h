#ifndef render_h
#define render_h

#include "main.h"
#include "state_manager.h"
#include <Arduino.h>
#include <WiFiNINA.h>

// #include "i18n/english.h"
#include "i18n/german.h"
// #include "i18n/french.h"
// #include "i18n/dutch.h"
// #include "i18n/luxembourgish.h"

#include "helpers.h"

class Renderer {
private:
  StateManager *m_stateManager;
  void pn(WiFiClient client, char *buffer);
  void renderActionsHeading(WiFiClient client);
  void renderOptionsHeading(WiFiClient client);
  void renderHeadJavascript(WiFiClient client);
  void renderHeadCss(WiFiClient client);

  void renderChannelDetailCompact(WiFiClient client, uint16_t channelId);

  void renderChannelDetail(WiFiClient client, uint16_t channelId,
                           bool renderHorizontalRule);

  uint16_t renderSlider(char *outputBuffer, uint16_t bufferSize,
                        uint16_t channelId);

  void renderSaveAndDiscardJavascript(WiFiClient client);
  void renderEditNormalChannelJavascript(WiFiClient client);
  void renderEditCustomChannelJavascript(WiFiClient client);

  void renderEditChannelName(WiFiClient client, uint16_t channelIdToEdit);
  void renderEditInitialState(WiFiClient client, uint16_t channelIdToEdit,
                              bool useCustomRange);
  void renderEditRandomOn(WiFiClient client, uint16_t channelIdToEdit,
                          bool useCustomRange);
  void renderEditRandomOff(WiFiClient client, uint16_t channelIdToEdit,
                           bool useCustomRange);
  void renderEditChannelLinked(WiFiClient client, uint16_t channelIdToEdit);
  void renderEditChannelHiddenInCompactView(WiFiClient client,
                                            uint16_t channelIdToEdit);
  void renderEditShowSlider(WiFiClient client, uint16_t channelIdToEdit);
  void renderEditCustomChannelToggle(WiFiClient client,
                                     uint16_t channelIdToEdit,
                                     bool useCustomRange);

  void renderEditNormalChannel(WiFiClient client);
  void renderEditCustomChannel(WiFiClient client);
  void renderEditSaveAndDiscardButtons(WiFiClient client);

  void renderActions(WiFiClient client);
  void renderOptions(WiFiClient client);
  void renderOptionsJavascript(WiFiClient client);

  static char m_checkedBuffer[];
  static char m_emptyBuffer[];
  static char m_renderHiddenBuffer[];
  static char m_textMutedBuffer[];

public:
  Renderer(StateManager *stateManager);
  void renderWebPage(WiFiClient client, bool foundRecursion);
  void renderHttp413ErrorPage(WiFiClient client);
  void renderHttp400ErrorPage(WiFiClient client);
  void renderHttp404ErrorPage(WiFiClient client);
};
#endif // render_h
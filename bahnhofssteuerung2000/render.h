#ifndef render_h
#define render_h

#include "bahnhofssteuerung2000.h"
#include "state_manager.h"
#include <Arduino.h>
#include <WiFiNINA.h>

#include "helpers.h"

class Renderer {
private:
  StateManager *m_stateManager;
  void pn(WiFiClient client, char *buffer);
  void renderActionsHeading(WiFiClient client);
  void renderOptionsHeading(WiFiClient client);
  void renderHeadJavascript(WiFiClient client);
  void renderHeadCss(WiFiClient client);

  uint16_t renderDisplayChannelExpandedIdsAndButtons(char *outputBuffer,
                                                     uint16_t bufferSize,
                                                     uint16_t channelId,
                                                     bool isSimpleRange);

  uint16_t renderDisplayChannelExpandedName(char *outputBuffer,
                                            uint16_t bufferSize);

  uint16_t renderDisplayChannelExpandedLinked(char *outputBuffer,
                                              uint16_t bufferSize,
                                              uint16_t channelId,
                                              bool toggleOneBasedAddresses);

  uint16_t renderDisplayChannelExpandedLinkDelayed(char *outputBuffer,
                                              uint16_t bufferSize,
                                              uint16_t channelId);

  uint16_t renderDisplayChannelExpandedHiddenInCompactView(char *outputBuffer,
                                                           uint16_t bufferSize,
                                                           uint16_t channelId);

  uint16_t renderDisplayChannelExpandedLerped(char *outputBuffer,
                                              uint16_t bufferSize,
                                              uint16_t channelId);

  void renderChannelDetailCompact(WiFiClient client, uint16_t channelId);

  void renderChannelDetailExpandedWithSimpleRange(WiFiClient client,
                                                  uint16_t channelId,
                                                  bool renderHorizontalRule);

  void renderChannelDetailExpandedWithCustomRange(WiFiClient client,
                                                  uint16_t channelId,
                                                  bool renderHorizontalRule);

  uint16_t renderSlider(char *outputBuffer, uint16_t bufferSize,
                        uint16_t channelId);

  void renderUpdateChannelJavascript(WiFiClient client);
  void renderEditNormalChannelJavascript(WiFiClient client);
  void renderEditCustomChannelJavascript(WiFiClient client);

  void renderEditAddSpacer(WiFiClient client);
  void renderEditDisplayHeading(WiFiClient client, char *heading);

  void renderEditChannelHeading(WiFiClient client, uint16_t channelIdToEdit);
  void renderEditChannelName(WiFiClient client, uint16_t channelIdToEdit);
  void renderEditCustomChannelToggle(WiFiClient client,
                                     uint16_t channelIdToEdit,
                                     bool useCustomRange);
  void renderEditInitialState(WiFiClient client, uint16_t channelIdToEdit,
                              bool useCustomRange);
  void renderEditRandomValue2(WiFiClient client, uint16_t channelIdToEdit,
                              bool useCustomRange);
  void renderEditRandomValue1(WiFiClient client, uint16_t channelIdToEdit,
                              bool useCustomRange);
  void renderEditChannelLinked(WiFiClient client, uint16_t channelIdToEdit);
  void renderEditChannelLinkDelay(WiFiClient client, uint16_t channelIdToEdit);
  void renderEditChannelHiddenInCompactView(WiFiClient client,
                                            uint16_t channelIdToEdit);
  void renderEditShowSlider(WiFiClient client, uint16_t channelIdToEdit);
  void renderEditChannelLerp(WiFiClient client, uint16_t channelIdToEdit);

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

  static char m_yesBuffer[];
  static char m_noBuffer[];

public:
  Renderer(StateManager *stateManager);
  void renderWebPage(WiFiClient client, bool foundRecursion);
  void renderHttp413ErrorPage(WiFiClient client);
  void renderHttp400ErrorPage(WiFiClient client);
  void renderHttp404ErrorPage(WiFiClient client);
};
#endif // render_h
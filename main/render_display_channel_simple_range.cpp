#include "eeprom.h"
#include "render.h"
#include "state_manager.h"

#include <WiFiNINA.h>

#include "helpers.h"

void Renderer::renderChannelDetailWithSimpleRange(WiFiClient client,
                                                  uint16_t channelId,
                                                  bool renderHorizontalRule) {
  readChannelNameFromEepromBufferToChannelNameBuffer(channelId);

  uint16_t brightness =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_OUTPUT_VALUE2);
  uint8_t brightnessAsPercentage = (int)(((float)brightness / 4095) * 100);

  bool initialState = readBoolForChannelFromEepromBuffer(
      channelId, MEM_SLOT_IS_START_VALUE_OUTPUT_VALUE2);

  bool toggleOneBasedAddresses = m_stateManager->getToggleOneBasedAddresses();

  uint16_t channelIdToDisplay =
      toggleOneBasedAddresses ? channelId + 1 : channelId;

  uint8_t boardIndex = getBoardIndexForChannel(channelId);
  uint8_t subAddress = getBoardSubAddressForChannel(channelId);

  uint8_t boardIndexToDisplay =
      toggleOneBasedAddresses ? boardIndex + 1 : boardIndex;

  uint8_t boardSubAddressToDisplay =
      toggleOneBasedAddresses ? subAddress + 1 : subAddress;

  char enabledBuffer[] = I18N_CHANNEL_ON;
  char disabledBuffer[] = I18N_CHANNEL_OFF;

  char yesBuffer[] = I18N_CHANNEL_YES;
  char noBuffer[] = I18N_CHANNEL_NO;

  char *toggleInitialStateCheckedBuffer =
      initialState ? enabledBuffer : disabledBuffer;

  // --- Prepare random on events ---
  bool randomOn =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_ON);

  uint8_t randomOnFreq =
      readUint8tForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_ON_FREQ);

  char *randomOnEventsEnabledBuffer = randomOn ? yesBuffer : noBuffer;

  char randomOnFrequencyHtmlInputBuffer[] = R"html(
  <div class='row'>
    <div class='col'>
      <span class='h6'>%s</span>
    </div>
    <div class='col mtba'>
      %d/h
    </div>
  </div>)html";

  char randomOnFrequencyHtmlOutputBuffer[512];

  snprintf(randomOnFrequencyHtmlOutputBuffer,
           sizeof(randomOnFrequencyHtmlOutputBuffer),
           randomOnFrequencyHtmlInputBuffer, I18N_CHANNEL_RANDOM_ON_FREQ,
           randomOnFreq);

  char *randomOnEventsFrequencyHtmlToDisplayBuffer =
      randomOn ? randomOnFrequencyHtmlOutputBuffer : m_emptyBuffer;
  // --- /Prepare random on events ---

  // --- Prepare random off events ---
  bool randomOff =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_OFF);

  uint8_t randomOffFreq =
      readUint8tForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_OFF_FREQ);

  char *randomOffEventsEnabledBuffer = randomOff ? yesBuffer : noBuffer;

  char randomOffFrequencyHtmlInputBuffer[] = R"html(
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col mtba">%d/h</div>
  </div>
      )html";

  char randomOffFrequencyHtmlOutputBuffer[512];

  snprintf(randomOffFrequencyHtmlOutputBuffer,
           sizeof(randomOffFrequencyHtmlOutputBuffer),
           randomOffFrequencyHtmlInputBuffer, I18N_CHANNEL_RANDOM_ON_FREQ,
           randomOffFreq);

  char *randomOffEventsFrequencyHtmlToDisplayBuffer =
      randomOff ? randomOffFrequencyHtmlOutputBuffer : m_emptyBuffer;

  // --- /Prepare random off events ---

  // --- Prepare linked ---
  bool isLinked =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_IS_LINKED);

  uint16_t linkedChannelId =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_LINKED_CHANNEL);

  uint16_t linkedChannelIdToDisplay =
      toggleOneBasedAddresses ? linkedChannelId + 1 : linkedChannelId;

  char *isChannelLinkedBuffer = isLinked ? yesBuffer : noBuffer;

  char linkedChannelHtmlInputBuffer[] = R"html(
  <div class='row'>
    <div class='col'>
      <span class='h6'>%s</span>
    </div>
    <div class='col mtba'>
      %d
    </div>
  </div>
  )html";

  char linkedChannelHtmlOutputBuffer[512];
  snprintf(linkedChannelHtmlOutputBuffer, sizeof(linkedChannelHtmlOutputBuffer),
           linkedChannelHtmlInputBuffer, I18N_CHANNEL_COMMANDED_BY_CHANNEL,
           linkedChannelIdToDisplay);

  char *linkedChannelHtmlToDisplayBuffer =
      isLinked ? linkedChannelHtmlOutputBuffer : m_emptyBuffer;
  // --- /Prepare linked ---

  // --- Prepare note on visibility in compact view
  bool isChannelHiddenInCompactView = readBoolForChannelFromEepromBuffer(
      channelId, MEM_SLOT_HIDE_IN_COMPACT_VIEW);

  char isChannelHiddenInCompactViewHtmlInputBuffer[] = R"html(
  <div class='row'>
    <div class='col'>
      <span class='h6'>%s</span>
    </div>
    <div class='col mtba'>
      %s
    </div>
  </div>
  )html";

  char *isChannelHiddenBuffer =
      isChannelHiddenInCompactView ? yesBuffer : noBuffer;

  char isChannelHiddenInCompactViewHtmlToDisplayBuffer[512];
  snprintf(isChannelHiddenInCompactViewHtmlToDisplayBuffer,
           sizeof(isChannelHiddenInCompactViewHtmlToDisplayBuffer),
           isChannelHiddenInCompactViewHtmlInputBuffer,
           I18N_IS_HIDDEN_IN_COMPACT_VIEW, isChannelHiddenBuffer);
  // --- /Prepare note on visibility in compact view

  // --- Prepare horizontal seperator
  char horizontalRuleHtmlBuffer[] = "<hr class='mb-3 mt-3'/>";
  char *horizontalRuleHtmlToDisplayBuffer =
      renderHorizontalRule ? horizontalRuleHtmlBuffer : m_emptyBuffer;
  // --- /Prepare horizontal seperator

  char outputBuffer[4096] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written += snprintf(
      outputBuffer + written, bufferSize - written, R"html(
<div id="channel-%d" class="pl-1 pr-1">
  <div class="row">
    <div class="col-9">
      <span class="h4">%s %d</span>
      %s %d, %s %d
    </div>

    <div class="col-3">
      <div class="d-flex justify-content-end">
        <button class="btn" name="editChannel" onclick="openEditChannelPage('%d')">
          🖊
        </button>
        <button class="btn" onclick="sendValue('setChannelToValue1', '%d')">
          ⛭
        </button>
        <button
          class="btn text-warning"
          onclick="sendValue('setChannelToValue2', '%d')"
        >
          ⛭
        </button>
      </div>
    </div>
  </div>)html",
      channelIdToDisplay, I18N_CHANNEL_CHANNEL, channelIdToDisplay,
      I18N_CHANNEL_BOARD, boardIndexToDisplay, I18N_CHANNEL_PIN,
      boardSubAddressToDisplay, channelIdToDisplay, channelId, channelId);

  bool toggleShowSlider =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_SHOW_SLIDER);

  if (toggleShowSlider) {
    written +=
        renderSlider(outputBuffer + written, bufferSize - written, channelId);
  }

  written += snprintf(
      outputBuffer + written, bufferSize - written, R"html(
  <!-- Description -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col font-weight-bold mtba">
      <b> %s </b>
    </div>
  </div>

  <!-- Start State -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col mtba">%s</div>
  </div>

  <!-- Brightness -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col mtba">%d %%</div>
  </div>

  <!-- Randomly turning on -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col mtba">%s</div>
  </div>

  <!-- Randomly turning on frequency if randomly turning on -->
  %s

  <!-- Randomly turning off -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col mtba">%s</div>
  </div>

  <!-- Randomly turning off frequency if randomly turning off -->
  %s

  <!-- Is channel linked -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col mtba">%s</div>
  </div>

  <!-- Linked channel if channel is linked -->
  %s

  <!-- Information if channel is hidden in compact view -->
  %s
</div>

<!-- Newline -->
%s
)html",
      I18N_CHANNEL_DESCRIPTION, m_channelNameBuffer, I18N_CHANNEL_START_STATE,
      toggleInitialStateCheckedBuffer, I18N_CHANNEL_BRIGHTNESS,
      brightnessAsPercentage, I18N_CHANNEL_RANDOMLY_ON,
      randomOnEventsEnabledBuffer, randomOnEventsFrequencyHtmlToDisplayBuffer,
      I18N_CHANNEL_RANDOMLY_OFF, randomOffEventsEnabledBuffer,
      randomOffEventsFrequencyHtmlToDisplayBuffer, I18N_CHANNEL_LINKED,
      isChannelLinkedBuffer, linkedChannelHtmlToDisplayBuffer,
      isChannelHiddenInCompactViewHtmlToDisplayBuffer,
      horizontalRuleHtmlToDisplayBuffer);

  pn(client, outputBuffer);
}

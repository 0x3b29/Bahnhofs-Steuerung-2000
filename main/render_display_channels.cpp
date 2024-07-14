#include "eeprom.h"
#include "render.h"
#include "state_manager.h"

#include <WiFiNINA.h>

#include "helpers.h"

void Renderer::renderChannelDetail(WiFiClient client, uint16_t channelId,
                                   bool renderHorizontalRule) {
  readChannelNameFromEepromBufferToChannelNameBuffer(channelId);

  uint16_t brightness =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_OUTPUT_VALUE1);
  uint8_t brightnessAsPercentage = (int)(((float)brightness / 4095) * 100);

  bool initialState = readBoolForChannelFromEepromBuffer(
      channelId, MEM_SLOT_START_OUTPUT_VALUE1);

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
           randomOnFrequencyHtmlInputBuffer, I18N_CHANNEL_RANDOM_FREQ,
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
           randomOffFrequencyHtmlInputBuffer, I18N_CHANNEL_RANDOM_FREQ,
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
        <button class="btn" onclick="sendValue('turnChannelOff', '%d')">
          ⛭
        </button>
        <button
          class="btn text-warning"
          onclick="sendValue('turnChannelOn', '%d')"
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

void Renderer::renderChannelDetailCompact(WiFiClient client,
                                          uint16_t channelId) {
  bool isChannelHiddenInCompactView = readBoolForChannelFromEepromBuffer(
      channelId, MEM_SLOT_HIDE_IN_COMPACT_VIEW);

  if (isChannelHiddenInCompactView) {
    return;
  }

  readChannelNameFromEepromBufferToChannelNameBuffer(channelId);

  int boardIndex = getBoardIndexForChannel(channelId);
  int subAddress = getBoardSubAddressForChannel(channelId);

  bool toggleOneBasedAddresses = m_stateManager->getToggleOneBasedAddresses();

  int boardIndexToDisplay =
      toggleOneBasedAddresses ? boardIndex + 1 : boardIndex;

  int boardSubAddressToDisplay =
      toggleOneBasedAddresses ? subAddress + 1 : subAddress;

  uint16_t brightness =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_OUTPUT_VALUE1);
  uint8_t brightnessAsPercentage = (int)(((float)brightness / 4095) * 100);

  uint16_t channelIdToDisplay =
      toggleOneBasedAddresses ? channelId + 1 : channelId;

  char channelNameToDisplay[MAX_CHANNEL_NAME_LENGTH];

  if (strcmp(m_channelNameBuffer, "")) {
    strcpy(channelNameToDisplay, m_channelNameBuffer);
  } else {
    snprintf(channelNameToDisplay, sizeof(channelNameToDisplay),
             "Board %d, Pin %d", boardIndexToDisplay, boardSubAddressToDisplay);
  }

  char outputBuffer[2048];
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written += snprintf(outputBuffer + written, bufferSize - written,
                      R"html(
<div id="channel-%d" class="pl-1 pr-1">
  <div class="d-flex g-0 align-items-center">
                      )html",
                      channelIdToDisplay);

  written += snprintf(outputBuffer + written, bufferSize - written,
                      R"html(
    <div class="d-flex flex-fill align-items-center">
      <div class="text-muted">%d.</div>
      <button
        class="btn p-0 ps-2 flex-fill align-items-start"
        onclick="openEditChannelPage('%d')"
      >
        <p class="text-start m-0">%s</p>
      </button>
      <div class="text-muted">%d&nbsp;%%</div>
    </div>
                      )html",
                      channelIdToDisplay, channelIdToDisplay,
                      channelNameToDisplay, brightnessAsPercentage);

  written += snprintf(outputBuffer + written, bufferSize - written,
                      R"html(
    <div class="d-flex">
      <button
        class="btn px-1 px-sm-2 px-md-3"
        onclick="sendValue('turnChannelOff','%d')"
      >
        ⛭
      </button>
      <button
        class="btn text-warning px-1 px-sm-2 px-md-3"
        onclick="sendValue('turnChannelOn','%d')"
      >
        ⛭
      </button>
    </div>
                    )html",
                      channelId, channelId);

  written += snprintf(outputBuffer + written, bufferSize - written,
                      R"html(
  </div>
                      )html");

  bool toggleShowSlider =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_SHOW_SLIDER);

  if (toggleShowSlider) {
    written +=
        renderSlider(outputBuffer + written, bufferSize - written, channelId);
  }

  written += snprintf(outputBuffer + written, bufferSize - written,
                      R"html(
</div>
                      )html");

  pn(client, outputBuffer);
}

uint16_t Renderer::renderSlider(char *outputBuffer, uint16_t bufferSize,
                                uint16_t channelId) {
  uint16_t outputValue1 =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_OUTPUT_VALUE1);

  return snprintf(outputBuffer, bufferSize,
                  R"html(
  <div class="row pt-1">
    <div class="col-12">
      <input
        class="form-range"
        type="range"
        min="0"
        max="%d"
        name="outputValue1"
        value="%d"
        onchange="onBrightnessValueChanged(this.value, %d)"
      />
    </div>
  </div> 
                  )html",
                  outputValue1, outputValue1, channelId);
}

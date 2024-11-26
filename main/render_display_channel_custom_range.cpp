#include "eeprom.h"
#include "render.h"
#include "state_manager.h"

#include <WiFiNINA.h>

#include "helpers.h"

void Renderer::renderChannelDetailWithCustomRange(WiFiClient client,
                                                  uint16_t channelId,
                                                  bool renderHorizontalRule) {
  readChannelNameFromEepromBufferToChannelNameBuffer(channelId);

  uint16_t value2 =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_OUTPUT_VALUE2);
  uint8_t value2AsPercentage = (int)(((float)value2 / 4095) * 100);

  uint16_t value1 =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_OUTPUT_VALUE1);
  uint8_t value1AsPercentage = (int)(((float)value1 / 4095) * 100);

  bool isInitialStateValue2 = readBoolForChannelFromEepromBuffer(
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

  char value2Buffer[] = I18N_EDIT_CUSTOM_PWM_VALUE_2;
  char value1Buffer[] = I18N_EDIT_CUSTOM_PWM_VALUE_1;

  char yesBuffer[] = I18N_CHANNEL_YES;
  char noBuffer[] = I18N_CHANNEL_NO;

  char *toggleIsInitialStateValue2CheckedBuffer =
      isInitialStateValue2 ? value2Buffer : value1Buffer;

  // --- Prepare random on events ---
  bool doRandomlySetValue2 =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_ON);

  uint8_t randomlySetValue2Freq =
      readUint8tForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_ON_FREQ);

  char *randomlySetValue2EventsEnabledBuffer =
      doRandomlySetValue2 ? yesBuffer : noBuffer;

  char randomlySetValue2FrequencyHtmlInputBuffer[] = R"html(
  <div class='row'>
    <div class='col'>
      <span class='h6'>%s</span>
    </div>
    <div class='col mtba'>
      %d/h
    </div>
  </div>)html";

  char randomlySetValue2FrequencyHtmlOutputBuffer[512];

  snprintf(randomlySetValue2FrequencyHtmlOutputBuffer,
           sizeof(randomlySetValue2FrequencyHtmlOutputBuffer),
           randomlySetValue2FrequencyHtmlInputBuffer,
           I18N_CHANNEL_RANDOM_ON_FREQ, randomlySetValue2Freq);

  char *randomlySetValue2EventsFrequencyHtmlToDisplayBuffer =
      doRandomlySetValue2 ? randomlySetValue2FrequencyHtmlOutputBuffer
                          : m_emptyBuffer;
  // --- /Prepare random on events ---

  // --- Prepare random off events ---
  bool doRandomlySetValue1 =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_OFF);

  uint8_t randomlySetValue1Freq =
      readUint8tForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_OFF_FREQ);

  char *randomlySetValue1EventsEnabledBuffer =
      doRandomlySetValue1 ? yesBuffer : noBuffer;

  char randomlySetValue1FrequencyHtmlInputBuffer[] = R"html(
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col mtba">%d/h</div>
  </div>
      )html";

  char randomlySetValue1FrequencyHtmlOutputBuffer[512];

  snprintf(randomlySetValue1FrequencyHtmlOutputBuffer,
           sizeof(randomlySetValue1FrequencyHtmlOutputBuffer),
           randomlySetValue1FrequencyHtmlInputBuffer,
           I18N_CHANNEL_RANDOM_ON_FREQ, randomlySetValue1Freq);

  char *randomlySetValue1EventsFrequencyHtmlToDisplayBuffer =
      doRandomlySetValue1 ? randomlySetValue1FrequencyHtmlOutputBuffer
                          : m_emptyBuffer;

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
        <button class="btn text-primary" onclick="sendValue('setChannelToValue1', '%d')">
          ⮘
        </button>
        <button
          class="btn text-primary"
          onclick="sendValue('setChannelToValue2', '%d')"
        >
          ⮚
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

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
  <!-- Description -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col font-weight-bold mtba">
      <b> %s </b>
    </div>
  </div>
  )html",
                      I18N_CHANNEL_DESCRIPTION, m_channelNameBuffer);

  const char *initialStateBuffer;

  if (isInitialStateValue2) {
    initialStateBuffer = I18N_EDIT_CUSTOM_PWM_VALUE_2;
  } else {
    initialStateBuffer = I18N_EDIT_CUSTOM_PWM_VALUE_1;
  }

  written +=
      snprintf(outputBuffer + written, bufferSize - written, R"html(
  <!-- Start State -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col mtba">%s</div>
  </div>

  <!-- Range -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col mtba">%d %% - %d %%</div>
  </div>
)html",
               I18N_CHANNEL_START_STATE, initialStateBuffer, I18N_CHANNEL_RANGE,
               value2AsPercentage, value1AsPercentage);

  written += snprintf(
      outputBuffer + written, bufferSize - written, R"html(
  <!-- Randomly setting value2 -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col mtba">%s</div>
  </div>

  <!-- Setting value2 frequency if randomly setting value2 active -->
  %s

  <!-- Randomly setting value1 -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col mtba">%s</div>
  </div>

  <!-- Setting value1 frequency if randomly setting value1 active -->
  %s
)html",
      I18N_EDIT_CUSTOM_RANDOM_VALUE_2, randomlySetValue2EventsEnabledBuffer,
      randomlySetValue2EventsFrequencyHtmlToDisplayBuffer,
      I18N_EDIT_CUSTOM_RANDOM_VALUE_1, randomlySetValue1EventsEnabledBuffer,
      randomlySetValue1EventsFrequencyHtmlToDisplayBuffer);

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
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
                      I18N_CHANNEL_LINKED, isChannelLinkedBuffer,
                      linkedChannelHtmlToDisplayBuffer,
                      isChannelHiddenInCompactViewHtmlToDisplayBuffer,
                      horizontalRuleHtmlToDisplayBuffer);

  pn(client, outputBuffer);
}

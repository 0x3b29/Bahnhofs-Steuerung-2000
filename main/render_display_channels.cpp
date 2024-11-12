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

  bool toggleUseCustomRange = readBoolForChannelFromEepromBuffer(
      channelId, MEM_SLOT_USES_OUTPUT_VALUE1);

  uint16_t value2 =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_OUTPUT_VALUE2);
  uint8_t value2AsPercentage = (int)(((float)value2 / 4095) * 100);

  uint16_t value1 =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_OUTPUT_VALUE1);
  uint8_t value1AsPercentage = (int)(((float)value1 / 4095) * 100);

  if (!toggleUseCustomRange) {
    value1 = 0;
  }

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

  written +=
      snprintf(outputBuffer + written, bufferSize - written,
               R"html(
    <div class="d-flex flex-fill align-items-center">
      <div class="text-muted">%d.</div>
      <button
        class="btn p-0 ps-2 flex-fill align-items-start"
        onclick="openEditChannelPage('%d')"
      >
        <p class="text-start m-0">%s</p>
      </button>
      )html",
               channelIdToDisplay, channelIdToDisplay, channelNameToDisplay);

  if (toggleUseCustomRange) {
    written += snprintf(outputBuffer + written, bufferSize - written,
                        R"html(
      <div class="text-muted">%d&nbsp;%% … %d&nbsp;%%</div>
                      )html",
                        value1AsPercentage, value2AsPercentage);
  } else {
    written += snprintf(outputBuffer + written, bufferSize - written,
                        R"html(
      <div class="text-muted">%d&nbsp;%%</div>
                      )html",
                        value2AsPercentage);
  }

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
    </div>
                      )html");

  char *leftSymbol;
  char *rightSymbol;
  char *leftClass;
  char *rightClass;

  if (toggleUseCustomRange) {
    leftSymbol = "⮘";
    rightSymbol = "⮚";
    leftClass = "text-primary";
    rightClass = "text-primary";

  } else {
    leftSymbol = "⛭";
    rightSymbol = "⛭";
    leftClass = "";
    rightClass = "text-warning";
  }

  written += snprintf(outputBuffer + written, bufferSize - written,
                      R"html(
    <div class="d-flex">
      <button
        class="btn %s px-1 px-sm-2 px-md-3"
        onclick="sendValue('setChannelToValue1','%d')"
      >
        %s
      </button>
      <button
        class="btn %s px-1 px-sm-2 px-md-3"
        onclick="sendValue('setChannelToValue2','%d')"
      >
        %s
      </button>
    </div>
                    )html",
                      leftClass, channelId, leftSymbol, rightClass, channelId,
                      rightSymbol);

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

  bool toggleUseCustomRange = readBoolForChannelFromEepromBuffer(
      channelId, MEM_SLOT_USES_OUTPUT_VALUE1);

  uint16_t outputValue2 =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_OUTPUT_VALUE2);

  uint16_t outputValue1 = 0;

  if (toggleUseCustomRange) {
    outputValue1 = readUint16tForChannelFromEepromBuffer(
        channelId, MEM_SLOT_OUTPUT_VALUE1);
  }

  bool isInitialStateValue2 = readBoolForChannelFromEepromBuffer(
      channelId, MEM_SLOT_IS_START_VALUE_OUTPUT_VALUE2);

  uint16_t startValue;

  if (isInitialStateValue2) {
    startValue = outputValue2;
  } else {
    startValue = outputValue1;
  }

  int sliderMin;
  int sliderMax;

  if (outputValue2 > outputValue1) {
    sliderMin = outputValue1;
    sliderMax = outputValue2;
  } else {
    sliderMin = -outputValue1;
    sliderMax = -outputValue2;
  }

  return snprintf(outputBuffer, bufferSize,
                  R"html(
  <div class="row pt-1">
    <div class="col-12">
      <input
        class="form-range"
        type="range"
        min="%d"
        max="%d"
        name="outputValue2"
        value="%d"
        onchange="onSliderValueChanged(Math.abs(this.value), %d)"
      />
    </div>
  </div> 
                  )html",
                  sliderMin, sliderMax, startValue, channelId);
}

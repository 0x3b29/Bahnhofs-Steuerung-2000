#include "eeprom.h"
#include "render.h"
#include "state_manager.h"

#include <WiFiNINA.h>

#include "helpers.h"
#include "symbols.h"

void Renderer::renderChannelDetailExpandedWithCustomRange(
    WiFiClient client, uint16_t channelId, bool renderHorizontalRule) {
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

  char *toggleIsInitialStateValue2CheckedBuffer =
      isInitialStateValue2 ? value2Buffer : value1Buffer;

  // --- Prepare random on events ---
  bool doRandomlySetValue2 = readBoolForChannelFromEepromBuffer(
      channelId, MEM_SLOT_DO_RANDOMLY_SET_VALUE2);

  uint8_t randomlySetValue2Freq = readUint8tForChannelFromEepromBuffer(
      channelId, MEM_SLOT_RANDOMLY_SET_VALUE2_FREQ);

  char *randomlySetValue2EventsEnabledBuffer =
      doRandomlySetValue2 ? m_yesBuffer : m_noBuffer;

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
  bool doRandomlySetValue1 = readBoolForChannelFromEepromBuffer(
      channelId, MEM_SLOT_DO_RANDOMLY_SET_VALUE1);

  uint8_t randomlySetValue1Freq = readUint8tForChannelFromEepromBuffer(
      channelId, MEM_SLOT_RANDOMLY_SET_VALUE1_FREQ);

  char *randomlySetValue1EventsEnabledBuffer =
      doRandomlySetValue1 ? m_yesBuffer : m_noBuffer;

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

  // --- Prepare horizontal seperator
  char horizontalRuleHtmlBuffer[] = "<hr class='mb-3 mt-3'/>";
  char *horizontalRuleHtmlToDisplayBuffer =
      renderHorizontalRule ? horizontalRuleHtmlBuffer : m_emptyBuffer;
  // --- /Prepare horizontal seperator

  char outputBuffer[4096] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;
  written += renderDisplayChannelExpandedIdsAndButtons(
      outputBuffer + written, bufferSize - written, channelId, false);

  bool toggleShowSlider =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_SHOW_SLIDER);

  if (toggleShowSlider) {
    written +=
        renderSlider(outputBuffer + written, bufferSize - written, channelId);
  }

  written += renderDisplayChannelExpandedName(outputBuffer + written,
                                              bufferSize - written);

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

  written += renderDisplayChannelExpandedLinked(outputBuffer + written,
                                                bufferSize - written, channelId,
                                                toggleOneBasedAddresses);

  bool isLinked =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_IS_LINKED);
  
  if (isLinked) {
    written += renderDisplayChannelExpandedLinkDelayed(
        outputBuffer + written, bufferSize - written, channelId);
  }

  written += renderDisplayChannelExpandedLerped(
      outputBuffer + written, bufferSize - written, channelId);

  written += renderDisplayChannelExpandedHiddenInCompactView(
      outputBuffer + written, bufferSize - written, channelId);

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
</div>

<!-- Newline -->
%s
)html",
                      horizontalRuleHtmlToDisplayBuffer);

  pn(client, outputBuffer);
}

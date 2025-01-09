#include "eeprom.h"
#include "render.h"
#include "state_manager.h"

#include <WiFiNINA.h>

#include "helpers.h"
#include "symbols.h"

void Renderer::renderChannelDetailExpandedWithSimpleRange(
    WiFiClient client, uint16_t channelId, bool renderHorizontalRule) {
      
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

  char *toggleInitialStateCheckedBuffer =
      initialState ? enabledBuffer : disabledBuffer;

  // --- Prepare random on events ---
  bool doRandomlySetValue2 = readBoolForChannelFromEepromBuffer(
      channelId, MEM_SLOT_DO_RANDOMLY_SET_VALUE2);

  uint8_t doRandomlySetValue2Freq = readUint8tForChannelFromEepromBuffer(
      channelId, MEM_SLOT_RANDOMLY_SET_VALUE2_FREQ);

  char *doRandomlySetValue2EventsEnabledBuffer =
      doRandomlySetValue2 ? m_yesBuffer : m_noBuffer;

  char doRandomlySetValue2FrequencyHtmlInputBuffer[] = R"html(
  <div class='row'>
    <div class='col'>
      <span class='h6'>%s</span>
    </div>
    <div class='col mtba'>
      %d/h
    </div>
  </div>)html";

  char doRandomlySetValue2FrequencyHtmlOutputBuffer[512];

  snprintf(doRandomlySetValue2FrequencyHtmlOutputBuffer,
           sizeof(doRandomlySetValue2FrequencyHtmlOutputBuffer),
           doRandomlySetValue2FrequencyHtmlInputBuffer,
           I18N_CHANNEL_RANDOM_ON_FREQ, doRandomlySetValue2Freq);

  char *doRandomlySetValue2EventsFrequencyHtmlToDisplayBuffer =
      doRandomlySetValue2 ? doRandomlySetValue2FrequencyHtmlOutputBuffer
                          : m_emptyBuffer;
  // --- /Prepare random on events ---

  // --- Prepare random off events ---
  bool doRandomlySetValue1 = readBoolForChannelFromEepromBuffer(
      channelId, MEM_SLOT_DO_RANDOMLY_SET_VALUE1);

  uint8_t doRandomlySetValue1Freq = readUint8tForChannelFromEepromBuffer(
      channelId, MEM_SLOT_RANDOMLY_SET_VALUE1_FREQ);

  char *doRandomlySetValue1EventsEnabledBuffer =
      doRandomlySetValue1 ? m_yesBuffer : m_noBuffer;

  char doRandomlySetValue1FrequencyHtmlInputBuffer[] = R"html(
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col mtba">%d/h</div>
  </div>
      )html";

  char doRandomlySetValue1FrequencyHtmlOutputBuffer[512];

  snprintf(doRandomlySetValue1FrequencyHtmlOutputBuffer,
           sizeof(doRandomlySetValue1FrequencyHtmlOutputBuffer),
           doRandomlySetValue1FrequencyHtmlInputBuffer,
           I18N_CHANNEL_RANDOM_ON_FREQ, doRandomlySetValue1Freq);

  char *doRandomlySetValue1EventsFrequencyHtmlToDisplayBuffer =
      doRandomlySetValue1 ? doRandomlySetValue1FrequencyHtmlOutputBuffer
                          : m_emptyBuffer;

  // --- /Prepare random off events ---

  char outputBuffer[4096] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written += renderDisplayChannelExpandedIdsAndButtons(
      outputBuffer + written, bufferSize - written, channelId, true);

  bool toggleShowSlider =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_SHOW_SLIDER);

  if (toggleShowSlider) {
    written +=
        renderSlider(outputBuffer + written, bufferSize - written, channelId);
  }

  written += renderDisplayChannelExpandedName(outputBuffer + written,
                                              bufferSize - written);

  written +=
      snprintf(outputBuffer + written, bufferSize - written, R"html(
  <!-- Start State -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col mtba">%s</div>
  </div>
  )html",
               I18N_CHANNEL_START_STATE, toggleInitialStateCheckedBuffer);

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
  <!-- Brightness -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col mtba">%d %%</div>
  </div>
)html",
                      I18N_CHANNEL_BRIGHTNESS, brightnessAsPercentage);

  written +=
      snprintf(outputBuffer + written, bufferSize - written, R"html(
  <!-- Randomly turning on -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col mtba">%s</div>
  </div>

  <!-- Randomly turning on frequency if randomly turning on -->
  %s
)html",
               I18N_CHANNEL_RANDOMLY_ON, doRandomlySetValue2EventsEnabledBuffer,
               doRandomlySetValue2EventsFrequencyHtmlToDisplayBuffer);

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
  <!-- Randomly turning off -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col mtba">%s</div>
  </div>

  <!-- Randomly turning off frequency if randomly turning off -->
  %s
)html",
                      I18N_CHANNEL_RANDOMLY_OFF,
                      doRandomlySetValue1EventsEnabledBuffer,
                      doRandomlySetValue1EventsFrequencyHtmlToDisplayBuffer);

  written += renderDisplayChannelExpandedLinked(outputBuffer + written,
                                                bufferSize - written, channelId,
                                                toggleOneBasedAddresses);

  // --- Prepare note on visibility in compact view
  written += renderDisplayChannelExpandedHiddenInCompactView(
      outputBuffer + written, bufferSize - written, channelId);


  // --- Prepare horizontal seperator
  char horizontalRuleHtmlBuffer[] = "<hr class='mb-3 mt-3'/>";
  char *horizontalRuleHtmlToDisplayBuffer =
      renderHorizontalRule ? horizontalRuleHtmlBuffer : m_emptyBuffer;
  // --- /Prepare horizontal seperator

  written += snprintf(outputBuffer + written, bufferSize - written,
                      R"html(
</div>

<!-- Newline -->
%s
)html",
                      horizontalRuleHtmlToDisplayBuffer);

  pn(client, outputBuffer);
}

#include "eeprom.h"
#include "render.h"
#include "state_manager.h"

#include <WiFiNINA.h>

#include "helpers.h"
#include "symbols.h"

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

uint16_t Renderer::renderDisplayChannelExpandedIdsAndButtons(
    char *outputBuffer, uint16_t bufferSize, uint16_t channelId,
    bool isSimpleRange) {

  bool toggleOneBasedAddresses = m_stateManager->getToggleOneBasedAddresses();

  uint16_t channelIdToDisplay =
      toggleOneBasedAddresses ? channelId + 1 : channelId;

  uint8_t boardIndex = getBoardIndexForChannel(channelId);
  uint8_t subAddress = getBoardSubAddressForChannel(channelId);

  uint8_t boardIndexToDisplay =
      toggleOneBasedAddresses ? boardIndex + 1 : boardIndex;

  uint8_t boardSubAddressToDisplay =
      toggleOneBasedAddresses ? subAddress + 1 : subAddress;

  char *setValue1IconBuffer;
  char *setValue1IconColorBuffer;
  char *setValue2IconBuffer;
  char *setValue2IconColorBuffer;

  if (isSimpleRange) {
    setValue1IconBuffer = SYMBOL_TURN_LIGHT_OFF;
    setValue1IconColorBuffer = SYMBOL_TURN_LIGHT_OFF_CLASS;

    setValue2IconBuffer = SYMBOL_TURN_LIGHT_ON;
    setValue2IconColorBuffer = SYMBOL_TURN_LIGHT_ON_CLASS;
  } else {
    setValue1IconBuffer = SYMBOL_SET_VALUE_1;
    setValue1IconColorBuffer = SYMBOL_SET_VALUE_1_CLASS;

    setValue2IconBuffer = SYMBOL_SET_VALUE_2;
    setValue2IconColorBuffer = SYMBOL_SET_VALUE_2_CLASS;
  }

  return snprintf(outputBuffer, bufferSize, R"html(
<div id="channel-%d" class="pl-1 pr-1">
  <div class="row">
    <div class="col-9">
      <span class="h4">%s %d</span>
      %s %d, %s %d
    </div>

    <div class="col-3">
      <div class="d-flex justify-content-end">
        <button class="btn" name="editChannel" onclick="openEditChannelPage('%d')">
          %s
        </button>
        <button class="btn %s" onclick="sendValue('setChannelToValue1', '%d')">
          %s
        </button>
        <button
          class="btn %s"
          onclick="sendValue('setChannelToValue2', '%d')"
        >
          %s
        </button>
      </div>
    </div>
  </div>)html",
                  channelIdToDisplay, I18N_CHANNEL_CHANNEL, channelIdToDisplay,
                  I18N_CHANNEL_BOARD, boardIndexToDisplay, I18N_CHANNEL_PIN,
                  boardSubAddressToDisplay, channelIdToDisplay,
                  SYMBOL_EDIT_CHANNEL, setValue1IconColorBuffer, channelId,
                  setValue1IconBuffer, setValue2IconColorBuffer, channelId,
                  setValue2IconBuffer);
}

uint16_t Renderer::renderDisplayChannelExpandedName(char *outputBuffer,
                                                    uint16_t bufferSize) {
  return snprintf(outputBuffer, bufferSize, R"html(
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
}

uint16_t Renderer::renderDisplayChannelExpandedLinked(
    char *outputBuffer, uint16_t bufferSize, uint16_t channelId,
    bool toggleOneBasedAddresses) {

  bool isLinked =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_IS_LINKED);

  uint16_t linkedChannelId =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_LINKED_CHANNEL);

  uint16_t linkedChannelIdToDisplay =
      toggleOneBasedAddresses ? linkedChannelId + 1 : linkedChannelId;

  char *isChannelLinkedBuffer = isLinked ? m_yesBuffer : m_noBuffer;

  char linkedChannelHtmlInputBuffer[] = R"html(
  <div class='row'>
    <div class='col'>
      <span class='h6'>%s</span>
    </div>
    <div class='col mtba'>
      %u
    </div>
  </div>
  )html";

  char linkedChannelHtmlOutputBuffer[512];

  snprintf(linkedChannelHtmlOutputBuffer, sizeof(linkedChannelHtmlOutputBuffer),
           linkedChannelHtmlInputBuffer, I18N_CHANNEL_COMMANDED_BY_CHANNEL,
           linkedChannelIdToDisplay);

  char *linkedChannelHtmlToDisplayBuffer =
      isLinked ? linkedChannelHtmlOutputBuffer : m_emptyBuffer;

  return snprintf(outputBuffer, bufferSize, R"html(
  <!-- Is channel linked -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col mtba">%s</div>
  </div>

  <!-- Linked channel if channel is linked -->
  %s
)html",
                  I18N_CHANNEL_LINKED, isChannelLinkedBuffer,
                  linkedChannelHtmlToDisplayBuffer);
}

uint16_t Renderer::renderDisplayChannelExpandedLerped(char *outputBuffer,
                                                      uint16_t bufferSize,
                                                      uint16_t channelId) {
  bool isLerped =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_IS_LERPED);
  char *isChannelLerpedBuffer = isLerped ? m_yesBuffer : m_noBuffer;
  char lerpedChannelHtmlOutputBuffer[512] = "";

  if (isLerped) {
    float lerpSpeed =
        readFloatForChannelFromEepromBuffer(channelId, MEM_SLOT_LERP_SPEED);

    // We render lerpSpeed in a seperate buffer using our own helper function to
    // fix random crash during usage of %f in the big buffer
    char lerpSpeedBuffer[16];
    floatToBuffer(lerpSpeed, lerpSpeedBuffer, sizeof(lerpSpeedBuffer), 4);

    char lerpedChannelHtmlInputBuffer[] = R"html(
  <div class='row'>
    <div class='col'>
      <span class='h6'>%s</span>
    </div>
    <div class='col mtba'>
      %s
    </div>
  </div>
  )html";

    snprintf(lerpedChannelHtmlOutputBuffer,
             sizeof(lerpedChannelHtmlOutputBuffer),
             lerpedChannelHtmlInputBuffer, "I18N LERP SPEED", lerpSpeedBuffer);
  }

  return snprintf(outputBuffer, bufferSize, R"html(
  <!-- Is channel lerped -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col mtba">%s</div>
  </div>

  <!-- Lerp speed if channel is lerped -->
  %s
)html",
                  I18N_CHANNEL_LINKED, isChannelLerpedBuffer,
                  lerpedChannelHtmlOutputBuffer);
}

uint16_t Renderer::renderDisplayChannelExpandedHiddenInCompactView(
    char *outputBuffer, uint16_t bufferSize, uint16_t channelId) {
  bool isChannelHiddenInCompactView = readBoolForChannelFromEepromBuffer(
      channelId, MEM_SLOT_HIDE_IN_COMPACT_VIEW);

  char *isChannelHiddenBuffer =
      isChannelHiddenInCompactView ? m_yesBuffer : m_noBuffer;

  return snprintf(outputBuffer, bufferSize, R"html(
  <!-- Information if channel is hidden in compact view -->
  <div class='row'>
    <div class='col'>
      <span class='h6'>%s</span>
    </div>
    <div class='col mtba'>
      %s
    </div>
  </div>
  )html",
                  I18N_IS_HIDDEN_IN_COMPACT_VIEW, isChannelHiddenBuffer);
}
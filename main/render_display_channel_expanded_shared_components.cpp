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

uint16_t Renderer::renderDisplayChannelExpandedNameAndButtons(
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
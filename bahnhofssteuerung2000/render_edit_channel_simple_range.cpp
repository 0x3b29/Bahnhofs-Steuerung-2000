#include "eeprom.h"
#include "render.h"
#include "state_manager.h"

#include <WiFiNINA.h>

void Renderer::renderEditNormalChannelJavascript(WiFiClient client) {
  pn(client, R"html(
  function onSliderValueInput(value){
    var brightnessAsPercentage = Math.floor((value / 4095) * 100);
    var percentDisplay = document.getElementById("rangeAsPercentage");

    percentDisplay.textContent = "(" + brightnessAsPercentage + "%)"; 
  }

  function onSliderValueChanged(value, channelId) {
    var dataString =
      "setCustomValue=1" +
      "&propagateValue=0" +
      "&customValue=" +
      encodeURIComponent(value) +
      "&channelId=" +
      encodeURIComponent(channelId);
    fetch("/", {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded" },
      body: dataString,
    }).then((response) => {});
  }
  )html");
}

void Renderer::renderEditNormalChannel(WiFiClient client) {

  uint16_t channelIdToEdit = m_stateManager->getChannelIdToEdit();
  bool toggleOneBasedAddresses = m_stateManager->getToggleOneBasedAddresses();
  uint16_t channelIdToDisplay =
      toggleOneBasedAddresses ? channelIdToEdit + 1 : channelIdToEdit;

  uint16_t outputValue2 = readUint16tForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_OUTPUT_VALUE2);

  uint8_t brightnessAsPercentage = (int)(((float)outputValue2 / 4095) * 100);

  bool toggleUseCustomRange = readBoolForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_USES_OUTPUT_VALUE1);

  renderEditChannelHeading(client, channelIdToEdit);
  renderEditDisplayHeading(client, I18N_EDIT_HEADING_BASIC_SETTINGS);
  renderEditChannelName(client, channelIdToEdit);
  renderEditCustomChannelToggle(client, channelIdToEdit, toggleUseCustomRange);
  renderEditInitialState(client, channelIdToEdit, toggleUseCustomRange);

  char outputBuffer[1024] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(

<div class="row pt-1">
  <div class="col">
    <div class="col d-flex">
      <div>%s</div>
      <div id="rangeAsPercentage" class="ps-1 text-muted">(%d %%)</div>
    </div>
  </div>
  <div class="col-12">
    <input
      class="form-range"
      type="range"
      min="0"
      max="4095"
      name="outputValue2"
      value="%d"
      oninput="onSliderValueInput(Math.abs(this.value))"
      onchange="onSliderValueChanged(Math.abs(this.value), %d)"
    />
  </div>
</div>
    )html",
                      I18N_EDIT_BRIGHTNESS, brightnessAsPercentage,
                      outputValue2, channelIdToEdit);
  pn(client, outputBuffer);

  renderEditAddSpacer(client);
  renderEditDisplayHeading(client, I18N_EDIT_HEADING_OPTIONAL_SETTINGS);
  renderEditRandomValue2(client, channelIdToEdit, toggleUseCustomRange);
  renderEditRandomValue1(client, channelIdToEdit, toggleUseCustomRange);
  renderEditChannelLinked(client, channelIdToEdit);
  renderEditChannelLinkDelay(client, channelIdToEdit);
  renderEditChannelLerp(client, channelIdToEdit);

  renderEditAddSpacer(client);
  renderEditDisplayHeading(client, I18N_EDIT_HEADING_DISPLAY_SETTINGS);
  renderEditShowSlider(client, channelIdToEdit);
  renderEditChannelHiddenInCompactView(client, channelIdToEdit);
}

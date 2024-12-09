#include "eeprom.h"
#include "render.h"
#include "state_manager.h"
#include <WiFiNINA.h>

void Renderer::renderEditCustomChannelJavascript(WiFiClient client) {

  pn(client, R"html(
  function cancelEditChannelUpdates() {
    const channelId = document.querySelector('input[name="channelId"]').value;
    const channelIdToDisplay = document.querySelector('input[name="channelIdToDisplay"]').value;

    var dataString = `cancelChannelUpdate=true`+
    `&channelId=${channelId}`;

    fetch("/", {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded" },
      body: dataString,
    }).then((response) => {
      if (response.ok && response.status === 200) {
        window.location.href = `/#channel-${channelIdToDisplay}`;
      }
    });
  }

  function onSliderValueInput(value, display) {
      var brightnessAsPercentage = Math.floor((Math.abs(value) / 4095) * 100);
      display.textContent = "(" + brightnessAsPercentage + "%)";
  }

  function onSliderValueChanged(value, channelId, usage) {
    if (usage === 'value2' || usage === 'value1') {
        const rangeTestSlider = document.querySelector('input[name="rangeTestSlider"]');
        const outputValue2 = parseInt(document.querySelector('input[name="outputValue2"]').value);
        const outputValue1 = parseInt(document.querySelector('input[name="outputValue1"]').value);

        if (outputValue2 > outputValue1) {
          rangeTestSlider.min = outputValue1;
          rangeTestSlider.max = outputValue2;
        } else {
          rangeTestSlider.min = -outputValue1;
          rangeTestSlider.max = -outputValue2;
        }

        var percentDisplay = document.getElementById("rangeAsPercentageValueTest");
        var testSliderAsPercentage = Math.floor((Math.abs(rangeTestSlider.value) / 4095) * 100);
        percentDisplay.textContent = "(" + testSliderAsPercentage + "%)";
    }

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

void Renderer::renderEditCustomChannel(WiFiClient client) {
  uint16_t channelIdToEdit = m_stateManager->getChannelIdToEdit();
  bool toggleOneBasedAddresses = m_stateManager->getToggleOneBasedAddresses();
  uint8_t maxChannelNameLength = MAX_CHANNEL_NAME_LENGTH - 1;

  uint16_t outputValue2 = readUint16tForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_OUTPUT_VALUE2);

  uint16_t outputValue1 = readUint16tForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_OUTPUT_VALUE1);

  bool isInitialStateValue2 = readBoolForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_IS_START_VALUE_OUTPUT_VALUE2);

  char *toggleInitialStateCheckedBuffer =
      isInitialStateValue2 ? m_checkedBuffer : m_emptyBuffer;

  uint8_t outputValue2AsPercentage = (int)(((float)outputValue2 / 4095) * 100);
  uint8_t outputValue1AsPercentage = (int)(((float)outputValue1 / 4095) * 100);

  int rangeTestSliderMin;
  int rangeTestSliderMax;
  int rangeTestSliderValue;
  uint8_t rangeTestSliderPercentage;

  if (isInitialStateValue2) {
    rangeTestSliderValue = outputValue2;
    rangeTestSliderPercentage = outputValue2AsPercentage;
  } else {
    rangeTestSliderValue = outputValue1;
    rangeTestSliderPercentage = outputValue1AsPercentage;
  }

  if (outputValue2 > outputValue1) {
    rangeTestSliderMin = outputValue1;
    rangeTestSliderMax = outputValue2;
  } else {
    rangeTestSliderMin = -outputValue1;
    rangeTestSliderMax = -outputValue2;

    if (isInitialStateValue2) {
      rangeTestSliderValue = -outputValue2;
    } else {
      rangeTestSliderValue = -outputValue1;
    }
  }

  uint16_t channelIdToDisplay =
      toggleOneBasedAddresses ? channelIdToEdit + 1 : channelIdToEdit;

  bool toggleUseCustomRange = readBoolForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_USES_OUTPUT_VALUE1);

  renderEditChannelName(client, channelIdToEdit);
  renderEditInitialState(client, channelIdToEdit, toggleUseCustomRange);

  char outputBuffer[4096] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
<div class="row pt-1">
  <div class="col">
    <div class="col d-flex">
      <div>%s</div>
      <div id="rangeAsPercentageValue1" class="ps-1 text-muted">(%d %%)</div>
    </div>
  </div>
  <div class="col-12">
    <input
      class="form-range"
      type="range"
      min="0"
      max="4095"
      name="outputValue1"
      value="%d"
      oninput="onSliderValueInput(Math.abs(this.value), rangeAsPercentageValue1)"
      onchange="onSliderValueChanged(Math.abs(this.value), %d, 'value1')"
    />
  </div>
</div>
    )html",
                      I18N_EDIT_CUSTOM_PWM_VALUE_1, outputValue1AsPercentage,
                      outputValue1, channelIdToEdit);

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
<div class="row pt-1">
  <div class="col">
    <div class="col d-flex">
      <div>%s</div>
      <div id="rangeAsPercentageValue2" class="ps-1 text-muted">(%d %%)</div>
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
      oninput="onSliderValueInput(Math.abs(this.value), rangeAsPercentageValue2)"
      onchange="onSliderValueChanged(Math.abs(this.value), %d, rangeAsPercentageValue2, 'value2')"
    />
  </div>
</div>
    )html",
                      I18N_EDIT_CUSTOM_PWM_VALUE_2, outputValue2AsPercentage,
                      outputValue2, channelIdToEdit);

  written +=
      snprintf(outputBuffer + written, bufferSize - written, R"html(
<div class="row pt-1">
  <div class="col">
    <div class="col d-flex">
      <div>%s</div>
      <div id="rangeAsPercentageValueTest" class="ps-1 text-muted">(%d %%)</div>
    </div>
  </div>
  <div class="col-12">
    <input
      class="form-range"
      type="range"
      min="%d"
      max="%d"
      name="rangeTestSlider"
      value="%d"
      oninput="onSliderValueInput(Math.abs(this.value), rangeAsPercentageValueTest)"
      onchange="onSliderValueChanged(Math.abs(this.value), %d, rangeAsPercentageValueTest, 'test')"
    />
  </div>
</div>
    )html",
               I18N_EDIT_CUSTOM_PWM_OUTPUT_TEST, rangeTestSliderPercentage,
               rangeTestSliderMin, rangeTestSliderMax, rangeTestSliderValue,
               channelIdToEdit);

  pn(client, outputBuffer);

  renderEditRandomValue2(client, channelIdToEdit, toggleUseCustomRange);
  renderEditRandomValue1(client, channelIdToEdit, toggleUseCustomRange);
  renderEditChannelLinked(client, channelIdToEdit);
  renderEditChannelLinkDelay(client, channelIdToEdit);
  renderEditChannelHiddenInCompactView(client, channelIdToEdit);
  renderEditShowSlider(client, channelIdToEdit);
  renderEditCustomChannelToggle(client, channelIdToEdit, toggleUseCustomRange);
  renderEditChannelLerp(client, channelIdToEdit);
}

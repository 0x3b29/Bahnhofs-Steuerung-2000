#include "eeprom.h"
#include "render.h"
#include "state_manager.h"

#include <WiFiNINA.h>

void Renderer::renderEditNormalChannelJavascript(WiFiClient client) {
  pn(client, R"html(
  function onBrightnessValueChanged(value, channelId) {
    var brightnessAsPercentage = Math.floor((value / 4095) * 100);

    var percentDisplay = document.getElementById("rangeAsPercentage");

    if (percentDisplay) 
    {
      percentDisplay.textContent =
      "(" + brightnessAsPercentage + "%)";
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

  function onBrightnessValueChanged(value, channelId, display, usage) {
    var brightnessAsPercentage = Math.floor((Math.abs(value) / 4095) * 100);

    // var percentDisplay = document.getElementById(display);

    if (usage === 'value1' || usage === 'value2') {
        const rangeTestSlider = document.querySelector('input[name="rangeTestSlider"]');
        const outputValue1 = parseInt(document.querySelector('input[name="outputValue1"]').value);
        const outputValue2 = parseInt(document.querySelector('input[name="outputValue2"]').value);

        if (outputValue1 > outputValue2) {
          rangeTestSlider.min = outputValue2;
          rangeTestSlider.max = outputValue1;
        } else {
          rangeTestSlider.min = -outputValue2;
          rangeTestSlider.max = -outputValue1;
        }
    }

    if (display) 
    {
      display.textContent =
      "(" + brightnessAsPercentage + "%)";
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

void Renderer::renderSaveAndDiscardJavascript(WiFiClient client) {

  pn(client, R"html(
  function sendEditChannelUpdates(stayOnPageAfterUpdate) {
    const channelId = document.querySelector('input[name="channelId"]').value;
    const channelIdToDisplay = document.querySelector('input[name="channelIdToDisplay"]').value;
    const channelName = document.querySelector('input[name="channelName"]').value;
    const initialState = document.querySelector('input[name="initialState"]').checked ? 1 : 0;
    const outputValue1 = document.querySelector('input[name="outputValue1"]').value;
    const randomOn = document.querySelector('input[name="randomOn"]').checked ? 1 : 0;
    const frequencyOn = document.querySelector('input[name="frequencyOn"]').value;
    const randomOff = document.querySelector('input[name="randomOff"]').checked ? 1 : 0;
    const frequencyOff = document.querySelector('input[name="frequencyOff"]').value;
    const channelLinked = document.querySelector('input[name="channelLinked"]').checked ? 1 : 0;
    const linkedChannelId = document.querySelector('input[name="linkedChannelId"]').value;
    const channelHiddenInCompactView = document.querySelector('input[name="channelHiddenInCompactView"]').checked ? 1 : 0;
    const showSlider = document.querySelector('input[name="showSlider"]').checked ? 1 : 0;
    const useOutputValue2 = document.querySelector('input[name="useOutputValue2"]').checked ? 1 : 0;

    const outputValue2Slider = document.querySelector('input[name="outputValue2"]');
    

    var dataString = `updateChannel=true` + 
    `&channelId=${channelId}` + 
    `&channelName=${encodeURIComponent(channelName)}` + 
    `&initialState=${initialState}` +
    `&outputValue1=${outputValue1}` +
    `&randomOn=${randomOn}` +
    `&frequencyOn=${frequencyOn}` +
    `&randomOff=${randomOff}` +
    `&frequencyOff=${frequencyOff}` +
    `&channelLinked=${channelLinked}` +
    `&linkedChannelId=${linkedChannelId}` +
    `&channelHiddenInCompactView=${channelHiddenInCompactView}` +
    `&showSlider=${showSlider}`+ 
    `&useOutputValue2=${useOutputValue2}`;

    if (outputValue2Slider) {
      const outputValue2 = outputValue2Slider.value;
      dataString += `&outputValue2=${outputValue2}`;
    }

    fetch("/", {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded" },
      body: dataString,
    }).then((response) => {
      if (response.ok && response.status === 200) {
        if (stayOnPageAfterUpdate) {
          window.location.reload(true);
        } else {
          window.location.href = `/#channel-${channelIdToDisplay}`;
        }
      }
    });
  }

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
  )html");
}

void Renderer::renderEditChannelName(WiFiClient client,
                                     uint16_t channelIdToEdit) {
  bool toggleOneBasedAddresses = m_stateManager->getToggleOneBasedAddresses();

  uint16_t channelIdToDisplay =
      toggleOneBasedAddresses ? channelIdToEdit + 1 : channelIdToEdit;

  bool toggleUseCustomRange = readBoolForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_USES_OUTPUT_VALUE2);

  const char *channelMode =
      toggleUseCustomRange ? I18N_EDIT_MODE_CUSTOM : I18N_EDIT_MODE_NORMAL;

  uint8_t maxChannelNameLength = MAX_CHANNEL_NAME_LENGTH - 1;

  char outputBuffer[1024] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
  <div class="d-flex align-items-baseline">
    <h3>%s %s %d</h3> &nbsp; <div class="text-muted ">%s</div>
  </div>

<input type="hidden" name="channelId" value="%d" />
<input type="hidden" name="channelIdToDisplay" value="%d" />

<div class="row">
  <div class="col">%s</div>
  <div class="col d-flex flex-fill justify-content-end">
    <input
      class="form-control"
      type="text"
      maxlength="%d"
      size="%d"
      name="channelName"
      value="%s"
    />
  </div>
</div>
)html",
                      I18N_EDIT_EDIT, I18N_EDIT_CHANNEL, channelIdToDisplay,
                      channelMode, channelIdToEdit, channelIdToDisplay,
                      I18N_EDIT_DESCRIPTION, maxChannelNameLength,
                      maxChannelNameLength, m_channelNameBuffer);

  pn(client, outputBuffer);
}

void Renderer::renderEditInitialState(WiFiClient client,
                                      uint16_t channelIdToEdit,
                                      bool useCustomRange) {
  bool initialState = readBoolForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_IS_START_VALUE_OUTPUT_VALUE1);

  char *toggleInitialStateCheckedBuffer =
      initialState ? m_checkedBuffer : m_emptyBuffer;

  const char *i18n_initial_state = useCustomRange
                                       ? I18N_EDIT_CUSTOM_INITIAL_STATE_ON
                                       : I18N_EDIT_INITIAL_STATE_ON;

  char outputBuffer[1024] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
<div class="form-check form-switch pt-3">
  <input
    class="form-check-input"
    type="checkbox"
    name="initialState"
    value="1"
    role="switch"
    id="initialState"
    value="1"
    %s
  />
  <label class="form-check-label" for="initialState">%s</label>
</div>
)html",
                      toggleInitialStateCheckedBuffer, i18n_initial_state);

  pn(client, outputBuffer);
}

void Renderer::renderEditRandomOn(WiFiClient client, uint16_t channelIdToEdit,
                                  bool useCustomRange) {
  bool hasRandomOnEvents =
      readBoolForChannelFromEepromBuffer(channelIdToEdit, MEM_SLOT_RANDOM_ON);

  char *toggleHasRandomOnEventsCheckedBuffer =
      hasRandomOnEvents ? m_checkedBuffer : m_emptyBuffer;

  const char *i18n_random_on =
      useCustomRange ? I18N_EDIT_CUSTOM_RANDOM_VALUE_1 : I18N_EDIT_RANDOM_ON;

  uint8_t randomOnFreq = readUint8tForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_RANDOM_ON_FREQ);

  char outputBuffer[1024] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
  <div class="form-check form-switch pt-3">
    <input
      class="form-check-input"
      type="checkbox"
      name="randomOn"
      value="1"
      role="switch"
      id="randomOn"
      value="1"
      %s
    />
    <label class="form-check-label" for="randomOn">
      %s
    </label>
  </div>
  <div class="row">
    <div class="col d-flex align-items-center">%s</div>
    <div class="col d-flex justify-content-end">
      <input
        class="form-control"
        type="number"
        name="frequencyOn"
        min="0"
        max="255"
        value="%d"
      />
    </div>
  </div>
)html",
                      toggleHasRandomOnEventsCheckedBuffer, i18n_random_on,
                      I18N_EDIT_RANDOM_FREQ, randomOnFreq);

  pn(client, outputBuffer);
}

void Renderer::renderEditRandomOff(WiFiClient client, uint16_t channelIdToEdit,
                                   bool useCustomRange) {
  bool hasRandomOffEvents =
      readBoolForChannelFromEepromBuffer(channelIdToEdit, MEM_SLOT_RANDOM_OFF);

  char *toggleHasRandomOffEventsCheckedBuffer =
      hasRandomOffEvents ? m_checkedBuffer : m_emptyBuffer;

  const char *i18n_random_off =
      useCustomRange ? I18N_EDIT_CUSTOM_RANDOM_VALUE_2 : I18N_EDIT_RANDOM_OFF;

  uint8_t randomOffFreq = readUint8tForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_RANDOM_OFF_FREQ);

  char outputBuffer[1024] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
  <div class="form-check form-switch pt-3">
    <input
      class="form-check-input"
      type="checkbox"
      name="randomOff"
      value="1"
      role="switch"
      id="randomOff"
      value="1"
      %s
    />
    <label class="form-check-label" for="randomOff">
      %s
    </label>
  </div>

  <div class="row">
    <div class="col d-flex align-items-center">%s</div>
    <div class="col d-flex justify-content-end">
      <input
        class="form-control"
        type="number"
        name="frequencyOff"
        min="0"
        max="255"
        value="%d"
      />
    </div>
  </div>
  )html",
                      toggleHasRandomOffEventsCheckedBuffer, i18n_random_off,
                      I18N_EDIT_RANDOM_FREQ, randomOffFreq);

  pn(client, outputBuffer);
}

void Renderer::renderEditChannelLinked(WiFiClient client,
                                       uint16_t channelIdToEdit) {
  bool isChannelLinked =
      readBoolForChannelFromEepromBuffer(channelIdToEdit, MEM_SLOT_IS_LINKED);

  uint16_t numChannels = m_stateManager->getNumChannels();

  bool toggleOneBasedAddresses = m_stateManager->getToggleOneBasedAddresses();

  char *toggleIsChannelLinkedCheckedBuffer =
      isChannelLinked ? m_checkedBuffer : m_emptyBuffer;

  uint8_t smallesPossibleLinkedAddress = toggleOneBasedAddresses ? 1 : 0;
  uint16_t largestPossibleLinkedAddress =
      toggleOneBasedAddresses ? numChannels : numChannels - 1;

  uint16_t linkedChannelId = readUint16tForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_LINKED_CHANNEL);

  uint16_t displayedLinkedChannelId =
      toggleOneBasedAddresses ? linkedChannelId + 1 : linkedChannelId;

  char outputBuffer[1024] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written +=
      snprintf(outputBuffer + written, bufferSize - written, R"html(
  <div class="form-check form-switch pt-3">
    <input
      class="form-check-input"
      type="checkbox"
      name="channelLinked"
      value="1"
      role="switch"
      id="channelLinked"
      value="1"
      %s
    />
    <label class="form-check-label" for="channelLinked">%s</label>
  </div>

  <div class="row">
    <div class="col d-flex align-items-center">%s</div>
    <div class="col d-flex align-items-center justify-content-end">
      <input
        class="form-control"
        type="number"
        name="linkedChannelId"
        min="%d"
        max="%d"
        value="%d"
      />
    </div>
  </div>
  )html",
               toggleIsChannelLinkedCheckedBuffer, I18N_EDIT_LINKED,
               I18N_EDIT_CONTROLLED_BY_CHANNEL, smallesPossibleLinkedAddress,
               largestPossibleLinkedAddress, displayedLinkedChannelId);

  pn(client, outputBuffer);
}

void Renderer::renderEditChannelHiddenInCompactView(WiFiClient client,
                                                    uint16_t channelIdToEdit) {
  bool isChannelHiddenInCompactView = readBoolForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_HIDE_IN_COMPACT_VIEW);

  char *toggleIsHiddenInCompactViewCheckedBuffer =
      isChannelHiddenInCompactView ? m_checkedBuffer : m_emptyBuffer;

  char outputBuffer[1024] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
  <div class="form-check form-switch pt-3">
    <input
      class="form-check-input"
      type="checkbox"
      name="channelHiddenInCompactView"
      value="1"
      role="switch"
      id="channelHiddenInCompactView"
      value="1"
      %s
    />
    <label class="form-check-label" for="channelHiddenInCompactView">%s</label>
  </div>
)html",
                      toggleIsHiddenInCompactViewCheckedBuffer,
                      I18N_IS_HIDDEN_IN_COMPACT_VIEW);

  pn(client, outputBuffer);
}

void Renderer::renderEditShowSlider(WiFiClient client,
                                    uint16_t channelIdToEdit) {
  bool isChannelHiddenInCompactView = readBoolForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_HIDE_IN_COMPACT_VIEW);

  char *toggleIsHiddenInCompactViewCheckedBuffer =
      isChannelHiddenInCompactView ? m_checkedBuffer : m_emptyBuffer;

  char outputBuffer[1024] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  bool toggleShowSlider =
      readBoolForChannelFromEepromBuffer(channelIdToEdit, MEM_SLOT_SHOW_SLIDER);

  char *toggleShowSliderCheckedBuffer =
      toggleShowSlider ? m_checkedBuffer : m_emptyBuffer;

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
  <div class="form-check form-switch pt-3">
    <input
      class="form-check-input"
      type="checkbox"
      name="showSlider"
      value="1"
      role="switch"
      id="showSlider"
      value="1"
      %s
    />
    <label class="form-check-label" for="showSlider">%s</label>
  </div>
      )html",
                      toggleShowSliderCheckedBuffer, I18N_EDIT_SHOW_SLIDER);

  pn(client, outputBuffer);
}

void Renderer::renderEditCustomChannelToggle(WiFiClient client,
                                             uint16_t channelIdToEdit,
                                             bool toggleUseCustomRange) {
  char *toggleUseCustomRangeCheckedBuffer =
      toggleUseCustomRange ? m_checkedBuffer : m_emptyBuffer;

  char outputBuffer[1024] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written +=
      snprintf(outputBuffer + written, bufferSize - written, R"html(
  <div class="form-check form-switch pt-3">
    <input
      class="form-check-input"
      type="checkbox"
      name="useOutputValue2"
      value="1"
      role="switch"
      id="useOutputValue2"
      value="1"
      onclick="sendEditChannelUpdates(true)"
      %s
    />
    <label class="form-check-label" for="useOutputValue2">%s</label>
  </div>
      )html",
               toggleUseCustomRangeCheckedBuffer, I18N_EDIT_USE_CUSTOM_RANGE);

  pn(client, outputBuffer);
}

void Renderer::renderEditNormalChannel(WiFiClient client) {

  uint16_t channelIdToEdit = m_stateManager->getChannelIdToEdit();
  bool toggleOneBasedAddresses = m_stateManager->getToggleOneBasedAddresses();
  uint16_t channelIdToDisplay =
      toggleOneBasedAddresses ? channelIdToEdit + 1 : channelIdToEdit;

  uint16_t outputValue1 = readUint16tForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_OUTPUT_VALUE1);

  uint8_t brightnessAsPercentage = (int)(((float)outputValue1 / 4095) * 100);

  bool toggleUseCustomRange = readBoolForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_USES_OUTPUT_VALUE2);

  renderEditChannelName(client, channelIdToEdit);
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
      name="outputValue1"
      value="%d"
      onchange="onBrightnessValueChanged(Math.abs(this.value), %d)"
    />
  </div>
</div>
    )html",
                      I18N_EDIT_BRIGHTNESS, brightnessAsPercentage,
                      outputValue1, channelIdToEdit);
  pn(client, outputBuffer);

  renderEditRandomOn(client, channelIdToEdit, toggleUseCustomRange);
  renderEditRandomOff(client, channelIdToEdit, toggleUseCustomRange);
  renderEditChannelLinked(client, channelIdToEdit);
  renderEditChannelHiddenInCompactView(client, channelIdToEdit);
  renderEditShowSlider(client, channelIdToEdit);
  renderEditCustomChannelToggle(client, channelIdToEdit, toggleUseCustomRange);
}

void Renderer::renderEditCustomChannel(WiFiClient client) {
  uint16_t channelIdToEdit = m_stateManager->getChannelIdToEdit();
  bool toggleOneBasedAddresses = m_stateManager->getToggleOneBasedAddresses();
  uint8_t maxChannelNameLength = MAX_CHANNEL_NAME_LENGTH - 1;

  uint16_t outputValue1 = readUint16tForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_OUTPUT_VALUE1);

  uint16_t outputValue2 = readUint16tForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_OUTPUT_VALUE2);

  bool initialState = readBoolForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_IS_START_VALUE_OUTPUT_VALUE1);

  char *toggleInitialStateCheckedBuffer =
      initialState ? m_checkedBuffer : m_emptyBuffer;

  uint8_t outputValue1AsPercentage = (int)(((float)outputValue1 / 4095) * 100);
  uint8_t outputValue2AsPercentage = (int)(((float)outputValue2 / 4095) * 100);

  int rangeTestSliderMin;
  int rangeTestSliderMax;
  int rangeTestSliderValue;
  uint8_t rangeTestSliderPercentage;

  if (initialState == true) {
    rangeTestSliderValue = outputValue1;
    rangeTestSliderPercentage = outputValue1AsPercentage;
  } else {
    rangeTestSliderValue = outputValue2;
    rangeTestSliderPercentage = outputValue2AsPercentage;
  }

  if (outputValue1 > outputValue2) {
    rangeTestSliderMin = outputValue2;
    rangeTestSliderMax = outputValue1;
  } else {
    rangeTestSliderMin = -outputValue2;
    rangeTestSliderMax = -outputValue1;
  }

  uint16_t channelIdToDisplay =
      toggleOneBasedAddresses ? channelIdToEdit + 1 : channelIdToEdit;

  bool toggleUseCustomRange = readBoolForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_USES_OUTPUT_VALUE2);

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
      onchange="onBrightnessValueChanged(Math.abs(this.value), %d, rangeAsPercentageValue1, 'value1')"
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
      onchange="onBrightnessValueChanged(Math.abs(this.value), %d, rangeAsPercentageValue2, 'value2')"
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
      <div id="rangeAsPercentageValue3" class="ps-1 text-muted">(%d %%)</div>
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
      onchange="onBrightnessValueChanged(Math.abs(this.value), %d, rangeAsPercentageValue3, 'test')"
    />
  </div>
</div>
    )html",
               I18N_EDIT_CUSTOM_PWM_OUTPUT_TEST, rangeTestSliderPercentage,
               rangeTestSliderMin, rangeTestSliderMax, rangeTestSliderValue,
               channelIdToEdit);

  pn(client, outputBuffer);

  renderEditRandomOn(client, channelIdToEdit, toggleUseCustomRange);
  renderEditRandomOff(client, channelIdToEdit, toggleUseCustomRange);
  renderEditChannelLinked(client, channelIdToEdit);
  renderEditChannelHiddenInCompactView(client, channelIdToEdit);
  renderEditShowSlider(client, channelIdToEdit);
  renderEditCustomChannelToggle(client, channelIdToEdit, toggleUseCustomRange);
}

void Renderer::renderEditSaveAndDiscardButtons(WiFiClient client) {
  char outputBuffer[1024] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
<br />

<div class="row">
  <div class="col">
    <input
      class="btn btn-warning col-6"
      value="%s"
      type="button"
      onclick="cancelEditChannelUpdates();"
    />
  </div>
  <div class="col d-flex justify-content-end">
    <input
      class="btn btn-primary col-6"
      value="%s"
      type="button"
      onclick="sendEditChannelUpdates();"
    />
    &nbsp;
  </div>
</div>
)html",
                      I18N_EDIT_DISCARD, I18N_EDIT_SAVE);

  pn(client, outputBuffer);
}
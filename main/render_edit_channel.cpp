#include "eeprom.h"
#include "render.h"
#include "state_manager.h"

#include <WiFiNINA.h>

void Renderer::renderEditChannelJavascript(WiFiClient client) {
  pn(client, R"html(
  <script>
  function sendEditChannelUpdates() {
    const channelId = document.querySelector('input[name="channelId"]').value;
    const channelIdToDisplay = document.querySelector('input[name="channelIdToDisplay"]').value;
    const channelName = document.querySelector('input[name="channelName"]').value;
    const initialState = document.querySelector('input[name="initialState"]').checked ? 1 : 0;
    const channelBrightness = document.querySelector('input[name="channelBrightness"]').value;
    const randomOn = document.querySelector('input[name="randomOn"]').checked ? 1 : 0;
    const frequencyOn = document.querySelector('input[name="frequencyOn"]').value;
    const randomOff = document.querySelector('input[name="randomOff"]').checked ? 1 : 0;
    const frequencyOff = document.querySelector('input[name="frequencyOff"]').value;
    const channelLinked = document.querySelector('input[name="channelLinked"]').checked ? 1 : 0;
    const linkedChannelId = document.querySelector('input[name="linkedChannelId"]').value;
    const channelHiddenInCompactView = document.querySelector('input[name="channelHiddenInCompactView"]').checked ? 1 : 0;
    const showSlider = document.querySelector('input[name="showSlider"]').checked ? 1 : 0;

    var dataString = `updateChannel=true` + 
    `&channelId=${channelId}` + 
    `&channelName=${encodeURIComponent(channelName)}` + 
    `&initialState=${initialState}` +
    `&channelBrightness=${channelBrightness}` +
    `&randomOn=${randomOn}` +
    `&frequencyOn=${frequencyOn}` +
    `&randomOff=${randomOff}` +
    `&frequencyOff=${frequencyOff}` +
    `&channelLinked=${channelLinked}` +
    `&linkedChannelId=${linkedChannelId}` +
    `&channelHiddenInCompactView=${channelHiddenInCompactView}` +
    `&showSlider=${showSlider}`;

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

  function onBrightnessValueChanged(value, channelId) {
    var brightnessAsPercentage = Math.floor((value / 4095) * 100);

    var percentDisplay = document.getElementById("rangeAsPercentage");

    if (percentDisplay) 
    {
      percentDisplay.textContent =
      "(" + brightnessAsPercentage + "%)";
    }

    var dataString =
      "testBrightness=1&" +
      encodeURIComponent("channelBrightness") +
      "=" +
      encodeURIComponent(value) +
      "&" +
      encodeURIComponent("channelId") +
      "=" +
      encodeURIComponent(channelId);
    fetch("/", {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded" },
      body: dataString,
    }).then((response) => {});
  }
  </script>
  )html");
}

void Renderer::renderEditChannel(WiFiClient client) {
  uint16_t numChannels = m_stateManager->getNumChannels();
  uint16_t channelIdToEdit = m_stateManager->getChannelIdToEdit();
  bool toggleOneBasedAddresses = m_stateManager->getToggleOneBasedAddresses();

  uint16_t channelBrightness = readUint16tForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_BRIGHTNESS);

  uint8_t brightnessAsPercentage =
      (int)(((float)channelBrightness / 4095) * 100);

  uint16_t channelIdToDisplay =
      toggleOneBasedAddresses ? channelIdToEdit + 1 : channelIdToEdit;

  uint8_t maxChannelNameLength = MAX_CHANNEL_NAME_LENGTH - 1;

  char *toggleOneBasedAddressesCheckedBuffer =
      toggleOneBasedAddresses ? m_checkedBuffer : m_emptyBuffer;

  bool initialState = readBoolForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_INITIAL_STATE);

  char *toggleInitialStateCheckedBuffer =
      initialState ? m_checkedBuffer : m_emptyBuffer;

  bool hasRandomOnEvents =
      readBoolForChannelFromEepromBuffer(channelIdToEdit, MEM_SLOT_RANDOM_ON);

  char *toggleHasRandomOnEventsCheckedBuffer =
      hasRandomOnEvents ? m_checkedBuffer : m_emptyBuffer;

  uint8_t randomOnFreq = readUint8tForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_RANDOM_ON_FREQ);

  bool hasRandomOffEvents =
      readBoolForChannelFromEepromBuffer(channelIdToEdit, MEM_SLOT_RANDOM_OFF);

  char *toggleHasRandomOffEventsCheckedBuffer =
      hasRandomOffEvents ? m_checkedBuffer : m_emptyBuffer;

  uint8_t randomOffFreq = readUint8tForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_RANDOM_OFF_FREQ);

  bool isChannelLinked =
      readBoolForChannelFromEepromBuffer(channelIdToEdit, MEM_SLOT_IS_LINKED);

  char *toggleIsChannelLinkedCheckedBuffer =
      isChannelLinked ? m_checkedBuffer : m_emptyBuffer;

  uint8_t smallesPossibleLinkedAddress = toggleOneBasedAddresses ? 1 : 0;
  uint16_t largestPossibleLinkedAddress =
      toggleOneBasedAddresses ? numChannels : numChannels - 1;

  uint16_t linkedChannelId = readUint16tForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_LINKED_CHANNEL);

  uint16_t displayedLinkedChannelId =
      toggleOneBasedAddresses ? linkedChannelId + 1 : linkedChannelId;

  bool isChannelHiddenInCompactView = readBoolForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_HIDE_IN_COMPACT_VIEW);

  char *toggleIsHiddenInCompactViewCheckedBuffer =
      isChannelHiddenInCompactView ? m_checkedBuffer : m_emptyBuffer;

  char outputBuffer[4096] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written += snprintf(
      outputBuffer + written, bufferSize - written, R"html(
<h3>%s %d %s</h3>

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
      name="channelBrightness"
      value="%d"
      onchange="onBrightnessValueChanged(this.value, %d)"
    />
  </div>
</div>

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
      I18N_EDIT_CHANNEL, channelIdToDisplay, I18N_EDIT_EDIT, channelIdToEdit,
      channelIdToDisplay, I18N_EDIT_DESCRIPTION, maxChannelNameLength,
      maxChannelNameLength, m_channelNameBuffer,
      toggleInitialStateCheckedBuffer, I18N_EDIT_START_STATE,
      I18N_EDIT_BRIGHTNESS, brightnessAsPercentage, channelBrightness,
      channelIdToEdit, toggleHasRandomOnEventsCheckedBuffer,
      I18N_EDIT_RANDOM_ON, I18N_EDIT_RANDOM_FREQ, randomOnFreq,
      toggleHasRandomOffEventsCheckedBuffer, I18N_EDIT_RANDOM_OFF,
      I18N_EDIT_RANDOM_FREQ, randomOffFreq, toggleIsChannelLinkedCheckedBuffer,
      I18N_EDIT_LINKED, I18N_EDIT_CONTROLLED_BY_CHANNEL,
      smallesPossibleLinkedAddress, largestPossibleLinkedAddress,
      displayedLinkedChannelId, toggleIsHiddenInCompactViewCheckedBuffer,
      I18N_IS_HIDDEN_IN_COMPACT_VIEW);

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

  snprintf(outputBuffer, sizeof(outputBuffer), R"html(
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

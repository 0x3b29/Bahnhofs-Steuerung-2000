#include "eeprom.h"
#include "render.h"
#include "state_manager.h"

#include <WiFiNINA.h>

void Renderer::renderSaveAndDiscardJavascript(WiFiClient client) {

  pn(client, R"html(
  function sendEditChannelUpdates(stayOnPageAfterUpdate) {
    const channelId = document.querySelector('input[name="channelId"]').value;
    const channelIdToDisplay = document.querySelector('input[name="channelIdToDisplay"]').value;
    const channelName = document.querySelector('input[name="channelName"]').value;
    const initialState = document.querySelector('input[name="initialState"]').checked ? 1 : 0;
    const outputValue2 = document.querySelector('input[name="outputValue2"]').value;
    const randomOn = document.querySelector('input[name="randomOn"]').checked ? 1 : 0;
    const frequencyOn = document.querySelector('input[name="frequencyOn"]').value;
    const randomOff = document.querySelector('input[name="randomOff"]').checked ? 1 : 0;
    const frequencyOff = document.querySelector('input[name="frequencyOff"]').value;
    const channelLinked = document.querySelector('input[name="channelLinked"]').checked ? 1 : 0;
    const linkedChannelId = document.querySelector('input[name="linkedChannelId"]').value;
    const channelHiddenInCompactView = document.querySelector('input[name="channelHiddenInCompactView"]').checked ? 1 : 0;
    const showSlider = document.querySelector('input[name="showSlider"]').checked ? 1 : 0;
    const useOutputValue1 = document.querySelector('input[name="useOutputValue1"]').checked ? 1 : 0;
    const channelLerped = document.querySelector('input[name="channelLerped"]').checked ? 1 : 0;
    const lerpSpeed = document.querySelector('input[name="lerpSpeed"]').value;

    // Special case, the data is fetched later based on outputValue1Slider
    const outputValue1Slider = document.querySelector('input[name="outputValue1"]');

    var dataString = `updateChannel=true` + 
    `&channelId=${channelId}` + 
    `&channelName=${encodeURIComponent(channelName)}` + 
    `&initialState=${initialState}` +
    `&outputValue2=${outputValue2}` +
    `&randomOn=${randomOn}` +
    `&frequencyOn=${frequencyOn}` +
    `&randomOff=${randomOff}` +
    `&frequencyOff=${frequencyOff}` +
    `&channelLinked=${channelLinked}` +
    `&linkedChannelId=${linkedChannelId}` +
    `&channelHiddenInCompactView=${channelHiddenInCompactView}` +
    `&showSlider=${showSlider}`+ 
    `&useOutputValue1=${useOutputValue1}`+ 
    `&channelLerped=${channelLerped}`+ 
    `&lerpSpeed=${lerpSpeed}`;

    if (outputValue1Slider) {
      const outputValue1 = outputValue1Slider.value;
      dataString += `&outputValue1=${outputValue1}`;
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
      channelIdToEdit, MEM_SLOT_USES_OUTPUT_VALUE1);

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
      channelIdToEdit, MEM_SLOT_IS_START_VALUE_OUTPUT_VALUE2);

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
    %s
  />
  <label class="form-check-label" for="initialState">%s</label>
</div>
)html",
                      toggleInitialStateCheckedBuffer, i18n_initial_state);

  pn(client, outputBuffer);
}

void Renderer::renderEditRandomValue2(WiFiClient client,
                                      uint16_t channelIdToEdit,
                                      bool useCustomRange) {
  bool hasRandomOnEvents =
      readBoolForChannelFromEepromBuffer(channelIdToEdit, MEM_SLOT_RANDOM_ON);

  char *toggleHasRandomOnEventsCheckedBuffer =
      hasRandomOnEvents ? m_checkedBuffer : m_emptyBuffer;

  const char *i18n_random_on =
      useCustomRange ? I18N_EDIT_CUSTOM_RANDOM_VALUE_2 : I18N_EDIT_RANDOM_ON;

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
        oninput="validateInputData(this, 0, 255)"
      />
    </div>
  </div>
)html",
                      toggleHasRandomOnEventsCheckedBuffer, i18n_random_on,
                      I18N_EDIT_RANDOM_FREQ, randomOnFreq);

  pn(client, outputBuffer);
}

void Renderer::renderEditRandomValue1(WiFiClient client,
                                      uint16_t channelIdToEdit,
                                      bool useCustomRange) {
  bool hasRandomOffEvents =
      readBoolForChannelFromEepromBuffer(channelIdToEdit, MEM_SLOT_RANDOM_OFF);

  char *toggleHasRandomOffEventsCheckedBuffer =
      hasRandomOffEvents ? m_checkedBuffer : m_emptyBuffer;

  const char *i18n_random_off =
      useCustomRange ? I18N_EDIT_CUSTOM_RANDOM_VALUE_1 : I18N_EDIT_RANDOM_OFF;

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
        oninput="validateInputData(this, 0, 255)"
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
        oninput="validateInputData(this, %d, %d)"
      />
    </div>
  </div>
  )html",
               toggleIsChannelLinkedCheckedBuffer, I18N_EDIT_LINKED,
               I18N_EDIT_CONTROLLED_BY_CHANNEL, smallesPossibleLinkedAddress,
               largestPossibleLinkedAddress, displayedLinkedChannelId,
               smallesPossibleLinkedAddress, largestPossibleLinkedAddress);

  pn(client, outputBuffer);
}

void Renderer::renderEditChannelLerp(WiFiClient client,
                                     uint16_t channelIdToEdit) {
  bool isChannelLerped =
      readBoolForChannelFromEepromBuffer(channelIdToEdit, MEM_SLOT_IS_LERPED);

  float lerpSpeed =
      readFloatForChannelFromEepromBuffer(channelIdToEdit, MEM_SLOT_LERP_SPEED);

  char *toggleIsChannelLerpedCheckedBuffer =
      isChannelLerped ? m_checkedBuffer : m_emptyBuffer;

  char outputBuffer[1024] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
  <div class="form-check form-switch pt-3">
    <input
      class="form-check-input"
      type="checkbox"
      name="channelLerped"
      role="switch"
      id="channelLerped"
      value="1"
      %s
    />
    <label class="form-check-label" for="channelLerped">%s</label>
  </div>

  <div class="row">
    <div class="col d-flex align-items-center">%s</div>
    <div class="col d-flex align-items-center justify-content-end">
      <input
        class="form-control"
        type="number"
        step="any"
        name="lerpSpeed"
        value="%f"
      />
    </div>
  </div>
  )html",
                      toggleIsChannelLerpedCheckedBuffer, I18N_EDIT_IS_LERPED,
                      I18N_EDIT_LERP_SPEED, lerpSpeed);

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
      name="useOutputValue1"
      value="1"
      role="switch"
      id="useOutputValue1"
      onclick="sendEditChannelUpdates(true)"
      %s
    />
    <label class="form-check-label" for="useOutputValue1">%s</label>
  </div>
      )html",
               toggleUseCustomRangeCheckedBuffer, I18N_EDIT_USE_CUSTOM_RANGE);

  pn(client, outputBuffer);
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
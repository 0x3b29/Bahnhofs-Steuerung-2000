#include "eeprom.h"
#include "render.h"
#include "state_manager.h"

#include <WiFiNINA.h>

void Renderer::renderUpdateChannelJavascript(WiFiClient client) {

  pn(client, R"html(
  function sendEditChannelUpdates(stayOnPageAfterUpdate) {
    const channelId = document.querySelector('input[name="channelId"]').value;
    const channelIdToDisplay = document.querySelector('input[name="channelIdToDisplay"]').value;
    const channelName = document.querySelector('input[name="channelName"]').value;
    const initialState = document.querySelector('input[name="initialState"]').checked ? 1 : 0;
    const outputValue2 = document.querySelector('input[name="outputValue2"]').value;
    const doRandomlySetValue2 = document.querySelector('input[name="doRandomlySetValue2"]').checked ? 1 : 0;
    const frequencyValue2 = document.querySelector('input[name="frequencyValue2"]').value;
    const doRandomlySetValue1 = document.querySelector('input[name="doRandomlySetValue1"]').checked ? 1 : 0;
    const frequencyValue1 = document.querySelector('input[name="frequencyValue1"]').value;
    const channelLinked = document.querySelector('input[name="channelLinked"]').checked ? 1 : 0;
    const linkedChannelId = document.querySelector('input[name="linkedChannelId"]').value;
    const channelLinkDelay = document.querySelector('input[name="channelLinkDelay"]').value;
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
    `&doRandomlySetValue2=${doRandomlySetValue2}` +
    `&frequencyValue2=${frequencyValue2}` +
    `&doRandomlySetValue1=${doRandomlySetValue1}` +
    `&frequencyValue1=${frequencyValue1}` +
    `&channelLinked=${channelLinked}` +
    `&linkedChannelId=${linkedChannelId}` +
    `&channelLinkDelay=${channelLinkDelay}` +
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
    )html");

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

  function toggleRowVisibility(checkbox, rowName, inputName) {
    const row = document.getElementById(rowName);
    const input = document.getElementById(inputName);
    if (checkbox.checked) {
      row.style.display = 'flex';
    } else {
      row.style.display = 'none';
      input.value = 0;
    }
  }
  )html");
}

void Renderer::renderEditChannelHeading(WiFiClient client,
                                        uint16_t channelIdToEdit) {
  bool toggleOneBasedAddresses = m_stateManager->getToggleOneBasedAddresses();

  uint16_t channelIdToDisplay =
      toggleOneBasedAddresses ? channelIdToEdit + 1 : channelIdToEdit;

  bool toggleUseCustomRange = readBoolForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_USES_OUTPUT_VALUE1);

  const char *channelMode =
      toggleUseCustomRange ? I18N_EDIT_MODE_CUSTOM : I18N_EDIT_MODE_NORMAL;

  char outputBuffer[1024] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
  <div class="d-flex align-items-baseline">
    <h3>%s %s %d</h3> &nbsp; <div class="text-muted ">%s</div>
  </div>

<input type="hidden" name="channelId" value="%d" />
<input type="hidden" name="channelIdToDisplay" value="%d" />
)html",
                      I18N_EDIT_EDIT, I18N_EDIT_CHANNEL, channelIdToDisplay,
                      channelMode, channelIdToEdit, channelIdToDisplay);

  pn(client, outputBuffer);
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
                      I18N_EDIT_CHANNEL_NAME, maxChannelNameLength,
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
                                       ? I18N_EDIT_CUSTOM_INITIAL_STATE_VALUE_2
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
  bool hasdoRandomlySetValue2Events = readBoolForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_DO_RANDOMLY_SET_VALUE2);

  char *toggleHasdoRandomlySetValue2EventsCheckedBuffer =
      hasdoRandomlySetValue2Events ? m_checkedBuffer : m_emptyBuffer;

  const char *i18n_random_on =
      useCustomRange ? I18N_EDIT_CUSTOM_RANDOM_VALUE_2 : I18N_EDIT_RANDOM_ON;

  uint8_t doRandomlySetValue2Freq = readUint8tForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_RANDOMLY_SET_VALUE2_FREQ);

  char outputBuffer[1024] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written +=
      snprintf(outputBuffer + written, bufferSize - written, R"html(
  <div class="form-check form-switch pt-3">
    <input
      class="form-check-input"
      type="checkbox"
      name="doRandomlySetValue2"
      value="1"
      role="switch"
      id="doRandomlySetValue2"
      onchange="toggleRowVisibility(this, 'frequencyValue2Row', 'frequencyValue2')"
      %s
    />
    <label class="form-check-label" for="doRandomlySetValue2">
      %s
    </label>
  </div>

  <div id="frequencyValue2Row" class="row" style="%s">
    <div class="col d-flex align-items-center">%s</div>
    <div class="col d-flex justify-content-end">
      <input
        id="frequencyValue2"
        class="form-control"
        type="number"
        name="frequencyValue2"
        min="0"
        max="255"
        value="%d"
        oninput="validateInputData(this, 0, 255)"
      />
    </div>
  </div>
)html",
               toggleHasdoRandomlySetValue2EventsCheckedBuffer, i18n_random_on,
               hasdoRandomlySetValue2Events ? "" : "display: none;",
               I18N_EDIT_RANDOM_FREQ, doRandomlySetValue2Freq);

  pn(client, outputBuffer);
}

void Renderer::renderEditRandomValue1(WiFiClient client,
                                      uint16_t channelIdToEdit,
                                      bool useCustomRange) {
  bool hasdoRandomlySetValue1Events = readBoolForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_DO_RANDOMLY_SET_VALUE1);

  char *toggleHasdoRandomlySetValue1EventsCheckedBuffer =
      hasdoRandomlySetValue1Events ? m_checkedBuffer : m_emptyBuffer;

  const char *i18n_random_off =
      useCustomRange ? I18N_EDIT_CUSTOM_RANDOM_VALUE_1 : I18N_EDIT_RANDOM_OFF;

  uint8_t doRandomlySetValue1Freq = readUint8tForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_RANDOMLY_SET_VALUE1_FREQ);

  char outputBuffer[1024] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written +=
      snprintf(outputBuffer + written, bufferSize - written, R"html(
  <div class="form-check form-switch pt-3">
    <input
      class="form-check-input"
      type="checkbox"
      name="doRandomlySetValue1"
      value="1"
      role="switch"
      id="doRandomlySetValue1"
      onchange="toggleRowVisibility(this, 'frequencyValue1Row', 'frequencyValue1')"
      %s
    />
    <label class="form-check-label" for="doRandomlySetValue1">
      %s
    </label>
  </div>

  <div id="frequencyValue1Row" class="row" style="%s">
    <div class="col d-flex align-items-center">%s</div>
    <div class="col d-flex justify-content-end">
      <input
        id="frequencyValue1"
        class="form-control"
        type="number"
        name="frequencyValue1"
        min="0"
        max="255"
        value="%d"
        oninput="validateInputData(this, 0, 255)"
      />
    </div>
  </div>
  )html",
               toggleHasdoRandomlySetValue1EventsCheckedBuffer, i18n_random_off,
               hasdoRandomlySetValue1Events ? "" : "display: none;",
               I18N_EDIT_RANDOM_FREQ, doRandomlySetValue1Freq);

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
      onchange="toggleRowVisibility(this, 'channelLinkedRow', 'linkedChannelId')"
      %s
    />
    <label class="form-check-label" for="channelLinked">%s</label>
  </div>

  <div id="channelLinkedRow" class="row" style="%s">
    <div class="col d-flex align-items-center">%s</div>
    <div class="col d-flex align-items-center justify-content-end">
      <input
        id="linkedChannelId"
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
               isChannelLinked ? "" : "display: none;",
               I18N_EDIT_CONTROLLED_BY_CHANNEL, smallesPossibleLinkedAddress,
               largestPossibleLinkedAddress, displayedLinkedChannelId,
               smallesPossibleLinkedAddress, largestPossibleLinkedAddress);

  pn(client, outputBuffer);
}

void Renderer::renderEditChannelLinkDelay(WiFiClient client,
                                          uint16_t channelIdToEdit) {

  uint16_t linkDelay = readUint16tForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_LINK_DELAY);

  char *toggleHasChannelLinkDelayCheckedBuffer =
      linkDelay ? m_checkedBuffer : m_emptyBuffer;

  char outputBuffer[1024] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
  <div class="form-check form-switch pt-3">
    <input
      class="form-check-input"
      type="checkbox"
      name="hasChannelLinkDelay"
      role="switch"
      id="hasChannelLinkDelay"
      value="1"
      %s
      onchange="toggleRowVisibility(this, 'linkDelayRow', 'channelLinkDelay')"
    />
    <label class="form-check-label" for="hasChannelLinkDelay">%s</label>
  </div>

  <div class="row" id="linkDelayRow" style="%s">
    <div class="col d-flex align-items-center">%s</div>
    <div class="col d-flex align-items-center justify-content-end">
      <input
        class="form-control"
        type="number"
        id="channelLinkDelay"
        name="channelLinkDelay"
        min="%d"
        max="%d"
        value="%d"
        oninput="validateInputData(this, %d, %d)"
      />
    </div>
  </div>
  )html",
                      toggleHasChannelLinkDelayCheckedBuffer,
                      I18N_EDIT_LINK_DELAY, linkDelay ? "" : "display: none;",
                      I18N_EDIT_LINK_DELAY_MS, 0, 65535, linkDelay, 0, 65535);

  pn(client, outputBuffer);
}

void Renderer::renderEditChannelLerp(WiFiClient client,
                                     uint16_t channelIdToEdit) {
  bool isChannelLerped =
      readBoolForChannelFromEepromBuffer(channelIdToEdit, MEM_SLOT_IS_LERPED);

  float lerpSpeed =
      readFloatForChannelFromEepromBuffer(channelIdToEdit, MEM_SLOT_LERP_SPEED);

  // We render lerpSpeed in a seperate buffer using our own helper function to
  // fix random crash during usage of %f in the big buffer
  char lerpSpeedBuffer[16];
  floatToBuffer(lerpSpeed, lerpSpeedBuffer, sizeof(lerpSpeedBuffer), 4);

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
      onchange="toggleRowVisibility(this, 'lerpSpeedRow', 'lerpSpeed')"
      %s
    />
    <label class="form-check-label" for="channelLerped">%s</label>
  </div>

  <div id="lerpSpeedRow" class="row" style="%s">
    <div class="col d-flex align-items-center">%s</div>
    <div class="col d-flex align-items-center justify-content-end">
      <input
        id="lerpSpeed"
        class="form-control"
        type="number"
        step="any"
        name="lerpSpeed"
        value="%s"
      />
    </div>
  </div>
  )html",
                      toggleIsChannelLerpedCheckedBuffer, I18N_EDIT_IS_LERPED,
                      isChannelLerped ? "" : "display: none;",
                      I18N_EDIT_LERP_SPEED, lerpSpeedBuffer);

  if (written > bufferSize) {
    sn("WARNING, buffer overflow");
  }

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

void Renderer::renderEditAddSpacer(WiFiClient client) {
  char outputBuffer[128] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
<br />
)html");

  pn(client, outputBuffer);
}

void Renderer::renderEditDisplayHeading(WiFiClient client, char *heading) {
  char outputBuffer[128] = {0};
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
<b>%s</b>
)html",
                      heading);

  pn(client, outputBuffer);
}
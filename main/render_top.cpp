#include "render.h"

void Renderer::renderActionsHeading(WiFiClient client) {
  bool toggleActionsVisible = m_stateManager->getToggleShowActions();

  char *toggleActionsVisibleCheckedBuffer =
      toggleActionsVisible ? m_checkedBuffer : m_emptyBuffer;

  char *mutedBuffer = toggleActionsVisible ? m_emptyBuffer : m_textMutedBuffer;

  char outputBuffer[512];

  snprintf(outputBuffer, sizeof(outputBuffer),
           R"html(
<div class="d-flex align-items-center %s">
  <div id="actions-heading" class="h3">%s</div>
  <div class="form-check form-switch ms-2">
    <input
      class="form-check-input"
      type="checkbox"
      name="toggleShowActions"
      value="1"
      id="toggleShowActions"
      onchange="sendCheckbox(this, false); alignActionsAndOptionsHeadings();"
      %s
    />
  </div>
</div>
    )html",
           mutedBuffer, I18N_HEADING_ACTIONS,
           toggleActionsVisibleCheckedBuffer);

  pn(client, outputBuffer);
}

void Renderer::renderOptionsHeading(WiFiClient client) {
  bool toggleOptionsVisible = m_stateManager->getToggleShowOptions();

  char *toggleOptionsVisibleCheckedBuffer =
      toggleOptionsVisible ? m_checkedBuffer : m_emptyBuffer;

  char *mutedBuffer = toggleOptionsVisible ? m_emptyBuffer : m_textMutedBuffer;

  char outputBuffer[512];

  snprintf(outputBuffer, sizeof(outputBuffer),
           R"html(
<div class="d-flex align-items-center %s me-3">
  <div id="options-heading" class="h3">%s</div>
  <div class="form-check form-switch ms-2 ">
    <input
      class="form-check-input"
      type="checkbox"
      name="toggleShowOptions"
      value="1"
      id="toggleShowOptions"
      onchange="sendCheckbox(this, false); alignActionsAndOptionsHeadings();"
      %s
    />
  </div>
</div>
    )html",
           mutedBuffer, I18N_HEADING_OPTIONS,
           toggleOptionsVisibleCheckedBuffer);

  pn(client, outputBuffer);
}

void Renderer::renderOptions(WiFiClient client) {

  uint16_t numChannels = m_stateManager->getNumChannels();
  uint8_t getHighPwmBoards = m_stateManager->getHighPwmBoards();

  char *toggleOneBasedAddressesCheckedBuffer =
      m_stateManager->getToggleOneBasedAddresses() ? m_checkedBuffer
                                                   : m_emptyBuffer;

  char *toggleCompactDisplayCheckedBuffer =
      m_stateManager->getToggleCompactDisplay() ? m_checkedBuffer
                                                : m_emptyBuffer;

  char *toggleForceAllOffCheckedBuffer =
      m_stateManager->getToggleForceAllOff() ? m_checkedBuffer : m_emptyBuffer;

  char *toggleForceAllOnCheckedBuffer =
      m_stateManager->getToggleForceAllOn() ? m_checkedBuffer : m_emptyBuffer;

  char *toggleRandomEventsCheckedBuffer =
      m_stateManager->getToggleRandomEvents() ? m_checkedBuffer : m_emptyBuffer;

  char *togglePropagateEventsCheckedBuffer =
      m_stateManager->getTogglePropagateEvents() ? m_checkedBuffer
                                                 : m_emptyBuffer;

  char *toggleRandomChaosCheckedBuffer =
      m_stateManager->getToggleRandomChaos() ? m_checkedBuffer : m_emptyBuffer;

  char *toggleRunningLightsCheckedBuffer =
      m_stateManager->getToggleRunningLights() ? m_checkedBuffer
                                               : m_emptyBuffer;

  char *renderHiddenBuffer = m_stateManager->getToggleShowOptions()
                                 ? m_emptyBuffer
                                 : m_renderHiddenBuffer;

  char optionsOutputBuffer[4096];
  snprintf(optionsOutputBuffer, sizeof(optionsOutputBuffer), R"html(
<div id="options" class="mb-3" %s>
  <!-- Number of channels -->
  <div class="row">
    <div class="col-3 d-flex align-items-center">%s</div>
    <div class="col-5">
      <input
        type="number"
        class="form-control w-100"
        name="numChannels"
        min="0"
        max="%d"
        value="%d"
        oninput="validateInputData(this, 0, %d)"
      />
    </div>
    <div class="col-4 d-flex justify-content-end">
      <button
        class="btn btn-primary"
        onClick="onUpdateChannelCountButtonClicked()"
      >
        %s
      </button>
    </div>
  </div>
  <!-- /Number of channels -->

  <br />

  <!-- 1 based addresses -->
  <div class="form-check form-switch">
    <input
      class="form-check-input"
      type="checkbox"
      name="toggleOneBasedAddresses"
      value="1"
      id="toggleOneBasedAddresses"
      onchange="sendCheckbox(this, true)"
      %s
    />
    <label class="form-check-label" for="toggleOneBasedAddresses">
      %s
    </label>
  </div>
  <!-- /1 based addresses -->

  <!-- Compact view -->
  <div class="form-check form-switch">
    <input
      class="form-check-input"
      type="checkbox"
      name="toggleCompactDisplay"
      value="1"
      id="toggleCompactDisplay"
      onchange="sendCheckbox(this, true)"
      %s
    />
    <label class="form-check-label" for="toggleCompactDisplay">
      %s
    </label>
  </div>
  <!-- /Compact view -->

  <!-- Force all lights off -->
  <div class="form-check form-switch">
    <input
      class="form-check-input"
      type="checkbox"
      name="toggleForceAllOff"
      value="1"
      id="toggleForceAllOff"
      onchange="sendCheckbox(this, false)"
      %s
    />
    <label class="form-check-label" for="toggleForceAllOff">
      %s
    </label>
  </div>
  <!-- Force all lights off -->

  <!-- Force all lights 100 %% -->
  <div class="form-check form-switch">
    <input
      class="form-check-input"
      type="checkbox"
      name="toggleForceAllOn"
      value="1"
      id="toggleForceAllOn"
      onchange="sendCheckbox(this, false)"
      %s
    />
    <label class="form-check-label" for="toggleForceAllOn">
      %s
    </label>
  </div>
  <!-- /Force all lights 100 %% -->

  <!-- Enable random events -->
  <div class="form-check form-switch">
    <input
      class="form-check-input"
      type="checkbox"
      name="toggleRandomEvents"
      value="1"
      role="switch"
      id="toggleRandomEvents"
      onchange="sendCheckbox(this, false)"
      %s
    />
    <label class="form-check-label" for="toggleRandomEvents">
      %s
    </label>
  </div>
  <!-- /Enable random events -->

  <!-- Enable event propagation -->
  <div class="form-check form-switch">
    <input
      class="form-check-input"
      type="checkbox"
      name="togglePropagateEvents"
      value="1"
      role="switch"
      id="togglePropagateEvents"
      onchange="sendCheckbox(this, false)"
      %s
    />
    <label class="form-check-label" for="togglePropagateEvents">
      %s
    </label>
  </div>
  <!-- /Enable event propagation -->

  <!-- Enable random blink -->
  <div class="form-check form-switch">
    <input
      class="form-check-input"
      type="checkbox"
      name="toggleRandomChaos"
      value="1"
      role="switch"
      id="toggleRandomChaos"
      onchange="sendCheckbox(this, false)"
      %s
    />
    <label class="form-check-label" for="toggleRandomChaos">
      %s
    </label>
  </div>
  <!-- /Enable random blink -->
)html",
           renderHiddenBuffer, I18N_OPTIONS_CHANNELS, MAX_TOTAL_CHANNELS,
           numChannels, MAX_TOTAL_CHANNELS, I18N_OPTIONS_SEND,
           toggleOneBasedAddressesCheckedBuffer,
           I18N_OPTIONS_ONE_BASED_ADDRESSES, toggleCompactDisplayCheckedBuffer,
           I18N_OPTIONS_COMPACT_VIEW, toggleForceAllOffCheckedBuffer,
           I18N_OPTIONS_FORCE_ALL_OFF, toggleForceAllOnCheckedBuffer,
           I18N_OPTIONS_FORCE_ALL_ON, toggleRandomEventsCheckedBuffer,
           I18N_OPTIONS_RANDOM_EVENTS, togglePropagateEventsCheckedBuffer,
           I18N_OPTIONS_PROPAGATE_EVENTS, toggleRandomChaosCheckedBuffer,
           I18N_OPTIONS_CRAZY_BLINK);

  pn(client, optionsOutputBuffer);

  snprintf(optionsOutputBuffer, sizeof(optionsOutputBuffer), R"html(
  <!-- Enable running lights -->
  <div class="form-check form-switch">
    <input
      class="form-check-input"
      type="checkbox"
      name="toggleRunningLights"
      value="1"
      role="switch"
      id="toggleRunningLights"
      onchange="sendCheckbox(this, false)"
      %s
    />
    <label class="form-check-label" for="toggleRunningLights">
      %s
    </label>
  </div>
  <!-- /Enable running lights -->
)html",
           toggleRunningLightsCheckedBuffer, I18N_OPTIONS_RUNNING_LIGHTS);

  pn(client, optionsOutputBuffer);

  uint8_t numberOfPwmBoards = (numChannels + 15) / 16;

  optionsOutputBuffer[0] = 0;
  uint16_t bufferSize = sizeof(optionsOutputBuffer);
  uint16_t written = 0;

  written +=
      snprintf(optionsOutputBuffer + written, bufferSize - written, R"html(

<div class="pt-3">
  %s
</div>

<div>
  
  )html",
               I18N_OPTIONS_SELECT_BOARDS_WITH_HIGH_PWM_FREQ);

  for (int i = 0; i < numberOfPwmBoards; i++) {

    bool isHigh = m_stateManager->getHighPwmBoard(i);

    uint8_t userFacingId = i;

    if (m_stateManager->getToggleOneBasedAddresses() == true) {
      userFacingId++;
    }

    written += snprintf(
        optionsOutputBuffer + written, bufferSize - written, R"html(
  <div class="form-check form-check-inline form-switch me-3">
    <input
      class="form-check-input"
      type="checkbox"
      name="toggleHighPwmBoard"
      value="1"
      role="switch"
      id="toggleHighPwmBoard%d"
      onchange="sendCheckbox(this, false, %d)"
      %s
    />
    <label class="form-check-label" for="toggleHighPwmBoard%d">
      %s %d
    </label>
  </div>
  )html",
        i, i, isHigh ? "checked" : "", i, I18N_CHANNEL_BOARD, userFacingId);

    if (i % 8 == 0) {
      pn(client, optionsOutputBuffer);
      optionsOutputBuffer[0] = 0;
      written = 0;
    }
  }

  written +=
      snprintf(optionsOutputBuffer + written, bufferSize - written, R"html(
  </div>
</div>
  )html");

  pn(client, optionsOutputBuffer);
}

void Renderer::renderActions(WiFiClient client) {

  char *renderHiddenBuffer = m_stateManager->getToggleShowActions()
                                 ? m_emptyBuffer
                                 : m_renderHiddenBuffer;

  char outputBuffer[2048];

  snprintf(outputBuffer, sizeof(outputBuffer),

           R"html(
<div id="actions" class="mb-3" %s>
  <button
    class="btn btn-primary text-white me-2 mb-2"
    onclick="sendValue('resetAllChannels','1')"
  >
    &nbsp;⌂&nbsp;
  </button>

  <button
    class="btn btn-primary text-white me-2 mb-2"
    onclick="sendValue('setAllChannels','0')"
  >
    0 %%
  </button>

  <button
    class="btn btn-primary text-white me-2 mb-2"
    onclick="sendValue('setAllChannels','5')"
  >
    5 %%
  </button>

  <button
    class="btn btn-primary text-white me-2 mb-2"
    onclick="sendValue('setAllChannels','10')"
  >
    10 %%
  </button>

  <button
    class="btn btn-primary text-white me-2 mb-2"
    onclick="sendValue('setAllChannels','25')"
  >
    25 %%
  </button>

  <button
    class="btn btn-primary text-white me-2 mb-2"
    onclick="sendValue('setAllChannels','50')"
  >
    50 %%
  </button>

  <button
    class="btn btn-primary text-white me-2 mb-2"
    onclick="sendValue('setAllChannels','100')"
  >
    100 %%
  </button>

  <button
    class="btn btn-primary text-white me-2 mb-2"
    onclick="sendValue('turnEvenChannelsOn','1')"
  >
    %s
  </button>

  <button
    class="btn btn-primary text-white me-2 mb-2"
    onclick="sendValue('turnOddChannelsOn','1')"
  >
    %s
  </button>

  <button
    class="btn btn-primary text-white me-2 mb-2"
    onclick="sendValue('countBinary','1')"
  >
    01010011
  </button>
</div>
)html",
           renderHiddenBuffer, I18N_ACTIONS_EVEN, I18N_ACTIONS_ODD);

  pn(client, outputBuffer);
}

void Renderer::renderOptionsJavascript(WiFiClient client) {
  pn(client, R"html(
    function onUpdateChannelCountButtonClicked() {
    const numChannels = document.querySelector('input[name="numChannels"]').value;
    
    var dataString = `updateNumberOfChannels=true` + 
    `&numChannels=${numChannels}`;

    fetch("/", {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded" },
      body: dataString,
    }).then((response) => {
      window.location.href = `/`;
    });
  }
  )html");
}

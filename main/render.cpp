#include "render.h"
#include "eeprom.h"
#include "state_manager.h"

#include <WiFiNINA.h>

#include "i18n/english.h"
// #include "i18n/german.h"
// #include "i18n/french.h"
// #include "i18n/dutch.h"
// #include "i18n/luxembourgish.h"

#include "helpers.h"

char m_checkedBuffer[] = "checked";
char m_emptyBuffer[] = "";
char m_renderHiddenBuffer[] = "style='display: none;'";
char m_textMutedBuffer[] = "text-muted";

Renderer::Renderer(StateManager *stateManager) {
  this->m_stateManager = stateManager;
}

void Renderer::renderOptions(WiFiClient client) {

  uint16_t numChannels = m_stateManager->getNumChannels();

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

  char *renderHiddenBuffer = m_stateManager->getToggleShowOptions()
                                 ? m_emptyBuffer
                                 : m_renderHiddenBuffer;

  char optionsOutputBuffer[4096];
  sprintf(optionsOutputBuffer, R"html(
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
      />
    </div>
    <div class="col-4 d-flex justify-content-end">
      <button
        class="btn btn-primary"
        type="submit"
        name="updateSettings"
        value="Absenden"
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

  <!-- This hidden field is meant to provide an 'easy' access to reset the // eeprom
  from the web interface -->
  <input type="hidden" name="clearEeprom" value="0" />
</div>
)html",
          renderHiddenBuffer, I18N_OPTIONS_CHANNELS, MAX_TOTAL_CHANNELS,
          numChannels, I18N_OPTIONS_SEND, toggleOneBasedAddressesCheckedBuffer,
          I18N_OPTIONS_ONE_BASED_ADDRESSES, toggleCompactDisplayCheckedBuffer,
          I18N_OPTIONS_COMPACT_VIEW, toggleForceAllOffCheckedBuffer,
          I18N_OPTIONS_FORCE_ALL_OFF, toggleForceAllOnCheckedBuffer,
          I18N_OPTIONS_FORCE_ALL_ON, toggleRandomEventsCheckedBuffer,
          I18N_OPTIONS_RANDOM_EVENTS, togglePropagateEventsCheckedBuffer,
          I18N_OPTIONS_PROPAGATE_EVENTS, toggleRandomChaosCheckedBuffer,
          I18N_OPTIONS_CRAZY_BLINK);

  pn(optionsOutputBuffer);
}

void Renderer::renderActions(WiFiClient client) {

  char *renderHiddenBuffer = m_stateManager->getToggleShowActions()
                                 ? m_emptyBuffer
                                 : m_renderHiddenBuffer;

  char outputBuffer[2048];

  sprintf(outputBuffer,

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
    onclick="sendValue('turnAllChannelsOff','1')"
  >
    0 %%
  </button>

  <button
    class="btn btn-primary text-white me-2 mb-2"
    onclick="sendValue('turnAllChannels25','1')"
  >
    25 %%
  </button>

  <button
    class="btn btn-primary text-white me-2 mb-2"
    onclick="sendValue('turnAllChannels50','1')"
  >
    50 %%
  </button>

  <button
    class="btn btn-primary text-white me-2 mb-2"
    onclick="sendValue('turnAllChannels100','1')"
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

  pn(outputBuffer);
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

  char outputBuffer[4096] = {0};
  sprintf(
      outputBuffer, R"html(
<h3>%s %d %s</h3>

<input type="hidden" name="channelId" value="%d" />

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

<br />

<div class="row">
  <div class="col">
    <input
      class="btn btn-warning"
      type="submit"
      name="cancelChannelUpdate"
      value="%s"
    />
  </div>
  <div class="col d-flex justify-content-end">
    <input
      class="btn btn-primary"
      type="submit"
      name="updateChannel"
      value="%s"
    />
    &nbsp;
  </div>
</div>
)html",
      I18N_EDIT_CHANNEL, channelIdToDisplay, I18N_EDIT_EDIT, channelIdToEdit,
      I18N_EDIT_DESCRIPTION, maxChannelNameLength, maxChannelNameLength,
      m_channelNameBuffer, toggleInitialStateCheckedBuffer,
      I18N_EDIT_START_STATE, I18N_EDIT_BRIGHTNESS, brightnessAsPercentage,
      channelBrightness, channelIdToEdit, toggleHasRandomOnEventsCheckedBuffer,
      I18N_EDIT_RANDOM_ON, I18N_EDIT_RANDOM_FREQ, randomOnFreq,
      toggleHasRandomOffEventsCheckedBuffer, I18N_EDIT_RANDOM_OFF,
      I18N_EDIT_RANDOM_FREQ, randomOffFreq, toggleIsChannelLinkedCheckedBuffer,
      I18N_EDIT_LINKED, I18N_EDIT_CONTROLLED_BY_CHANNEL,
      smallesPossibleLinkedAddress, largestPossibleLinkedAddress,
      displayedLinkedChannelId, I18N_EDIT_DISCARD, I18N_EDIT_SAVE);

  pn(outputBuffer);
}

void Renderer::renderChannelDetail(WiFiClient client, uint16_t channelId,
                                   bool renderHorizontalRule) {
  readChannelNameFromEepromBufferToChannelNameBuffer(channelId);

  uint16_t brightness =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_BRIGHTNESS);
  uint8_t brightnessAsPercentage = (int)(((float)brightness / 4095) * 100);

  bool initialState =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_INITIAL_STATE);

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

  char yesBuffer[] = I18N_CHANNEL_YES;
  char noBuffer[] = I18N_CHANNEL_NO;

  char *toggleInitialStateCheckedBuffer =
      initialState ? enabledBuffer : disabledBuffer;

  // --- Prepare random on events ---
  bool randomOn =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_ON);

  uint8_t randomOnFreq =
      readUint8tForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_ON_FREQ);

  char *randomOnEventsEnabledBuffer = randomOn ? yesBuffer : noBuffer;

  char randomOnFrequencyHtmlInputBuffer[] = R"html(
  <div class='row'>
    <div class='col'>
      <span class='h6'>%s</span>
    </div>
    <div class='col'>
      %d/h
    </div>
  </div>)html";

  char randomOnFrequencyHtmlOutputBuffer[512];

  sprintf(randomOnFrequencyHtmlOutputBuffer, randomOnFrequencyHtmlInputBuffer,
          I18N_CHANNEL_RANDOM_FREQ, randomOnFreq);

  char *randomOnEventsFrequencyHtmlToDisplayBuffer =
      randomOn ? randomOnFrequencyHtmlOutputBuffer : m_emptyBuffer;
  // --- /Prepare random on events ---

  // --- Prepare random off events ---
  bool randomOff =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_OFF);

  uint8_t randomOffFreq =
      readUint8tForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_OFF_FREQ);

  char *randomOffEventsEnabledBuffer = randomOff ? yesBuffer : noBuffer;

  char randomOffFrequencyHtmlInputBuffer[] = R"html(
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col">%d/h</div>
  </div>
      )html";

  char randomOffFrequencyHtmlOutputBuffer[512];

  sprintf(randomOffFrequencyHtmlOutputBuffer, randomOffFrequencyHtmlInputBuffer,
          I18N_CHANNEL_RANDOM_FREQ, randomOffFreq);

  char *randomOffEventsFrequencyHtmlToDisplayBuffer =
      randomOff ? randomOffFrequencyHtmlOutputBuffer : m_emptyBuffer;

  // --- /Prepare random off events ---

  // --- Prepare linked ---
  bool isLinked =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_IS_LINKED);

  uint16_t linkedChannelId =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_LINKED_CHANNEL);

  uint16_t linkedChannelIdToDisplay =
      toggleOneBasedAddresses ? linkedChannelId + 1 : linkedChannelId;

  char *isChannelLinkedBuffer = isLinked ? yesBuffer : noBuffer;

  char linkedChannelHtmlInputBuffer[] = R"html(
  <div class='row'>
    <div class='col'>
      <span class='h6'>%s</span>
    </div>
    <div class='col'>
      %d
    </div>
  </div>
  )html";

  char linkedChannelHtmlOutputBuffer[512];
  sprintf(linkedChannelHtmlOutputBuffer, linkedChannelHtmlInputBuffer,
          I18N_CHANNEL_COMMANDED_BY_CHANNEL, linkedChannelIdToDisplay);

  char *linkedChannelHtmlToDisplayBuffer =
      isLinked ? linkedChannelHtmlOutputBuffer : m_emptyBuffer;
  // --- /Prepare linked ---

  char horizontalRuleHtmlBuffer[] = "<hr class='mb-1 mt-1'/>";
  char *horizontalRuleHtmlToDisplayBuffer =
      renderHorizontalRule ? horizontalRuleHtmlBuffer : m_emptyBuffer;

  char outputBuffer[4096] = {0};
  // char inputBuffer[] = ;

  sprintf(outputBuffer, R"html(
<div id="channel-%d" class="pl-1 pr-1">
  <div class="row">
    <div class="col-9">
      <span class="h4">%s %d</span>
      %s %d, %s %d
    </div>

    <div class="col-3">
      <div class="d-flex justify-content-end">
        <button
          class="btn text-warning"
          onclick="sendValue('turnChannelOn', '%d')"
        >
          ⛭
        </button>
        <button class="btn" onclick="sendValue('turnChannelOff', '%d')">
          ⛭
        </button>
        <button class="btn" type="submit" name="editChannel" value="%d">
          🖊
        </button>
      </div>
    </div>
  </div>

  <!-- Description -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col font-weight-bold">
      <b> %s </b>
    </div>
  </div>

  <!-- Start State -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col">%s</div>
  </div>

  <!-- Brightness -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col">%d %%</div>
  </div>

  <!-- Randomly turning on -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col">%s</div>
  </div>

<!-- Randomly turning on frequency if randomly turning on -->
  %s

<!-- Randomly turning off -->
  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col">%s</div>
  </div>

<!-- Randomly turning off frequency if randomly turning off -->
  %s

  <div class="row">
    <div class="col">
      <span class="h6">%s</span>
    </div>
    <div class="col">%s</div>
  </div>

  %s
</div>

%s
)html",
          channelId, I18N_CHANNEL_CHANNEL, channelIdToDisplay,
          I18N_CHANNEL_BOARD, boardIndexToDisplay, I18N_CHANNEL_PIN,
          boardSubAddressToDisplay, channelId, channelId, channelId,
          I18N_CHANNEL_DESCRIPTION, m_channelNameBuffer,
          I18N_CHANNEL_START_STATE, toggleInitialStateCheckedBuffer,
          I18N_CHANNEL_BRIGHTNESS, brightnessAsPercentage,
          I18N_CHANNEL_RANDOMLY_ON, randomOnEventsEnabledBuffer,
          randomOnEventsFrequencyHtmlToDisplayBuffer, I18N_CHANNEL_RANDOMLY_OFF,
          randomOffEventsEnabledBuffer,
          randomOffEventsFrequencyHtmlToDisplayBuffer, I18N_CHANNEL_LINKED,
          isChannelLinkedBuffer, linkedChannelHtmlToDisplayBuffer,
          horizontalRuleHtmlToDisplayBuffer);

  pt(outputBuffer);
}

void Renderer::renderChannelDetailCompact(WiFiClient client, uint16_t channelId,
                                          bool renderHorizontalRule) {
  readChannelNameFromEepromBufferToChannelNameBuffer(channelId);

  int boardIndex = getBoardIndexForChannel(channelId);
  int subAddress = getBoardSubAddressForChannel(channelId);

  bool toggleOneBasedAddresses = m_stateManager->getToggleOneBasedAddresses();

  int boardIndexToDisplay =
      toggleOneBasedAddresses ? boardIndex + 1 : boardIndex;

  int boardSubAddressToDisplay =
      toggleOneBasedAddresses ? subAddress + 1 : subAddress;

  uint16_t brightness =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_BRIGHTNESS);
  uint8_t brightnessAsPercentage = (int)(((float)brightness / 4095) * 100);

  char horizontalRuleHtmlBuffer[] = "<hr class='mb-1 mt-1'/>";
  char *horizontalRuleHtmlToDisplayBuffer =
      renderHorizontalRule ? horizontalRuleHtmlBuffer : m_emptyBuffer;

  char outputBuffer[1024];

  uint16_t channelIdToDisplay =
      toggleOneBasedAddresses ? channelId + 1 : channelId;

  char channelNameToDisplay[MAX_CHANNEL_NAME_LENGTH];

  if (strcmp(m_channelNameBuffer, "")) {
    strcpy(channelNameToDisplay, m_channelNameBuffer);
  } else {
    sprintf(channelNameToDisplay, "Board %d, Pin %d", boardIndexToDisplay,
            boardSubAddressToDisplay);
  }

  sprintf(outputBuffer,
          R"html(
<div id="channel-%d" class="pl-1 pr-1">
  <div class="d-flex g-0 align-items-center">
    <div class="d-flex flex-fill align-items-center">
      <div class="text-muted">%d.</div>
      <button
        class="btn p-0 ps-1 flex-fill align-items-start"
        type="submit"
        name="editChannel"
        value="%d"
      >
        <p class="text-start m-0">%s</p>
      </button>
      <div class="text-muted">%d&nbsp;%%</div>
    </div>
    <div class="d-flex">
      <button
        class="btn text-warning px-1 px-sm-2 px-md-3"
        onclick="sendValue('turnChannelOn','%d')"
      >
        ⛭
      </button>
      <button
        class="btn px-1 px-sm-2 px-md-3"
        onclick="sendValue('turnChannelOff','%d')"
      >
        ⛭
      </button>
    </div>
  </div>
</div>
%s
)html",
          channelId, channelIdToDisplay, channelId, channelNameToDisplay,
          brightnessAsPercentage, channelId, channelId,
          horizontalRuleHtmlToDisplayBuffer);

  pt(outputBuffer);
}

void Renderer::renderHeadJavascript(WiFiClient client) {
  pn(R"html(
<script>
function sendValue(buttonName, buttonValue) {
  event.preventDefault();
  var dataString =
    encodeURIComponent(buttonName) + "=" + encodeURIComponent(buttonValue);
  fetch("/", {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: dataString,
  }).then((response) => {
    if (!response.ok && response.status === 400) {
      window.location.href = "/";
    }
  });
}

function sendCheckbox(checkbox, reloadAfterRequest) {
  var dataString =
    encodeURIComponent(checkbox.name) +
    "=" +
    encodeURIComponent(checkbox.checked ? 1 : 0);

  if (checkbox.id === "toggleShowOptions") {
    var optionsDiv = document.getElementById("options");
    var optionsHeading = document.getElementById("options-heading");
    optionsDiv.style.display = checkbox.checked ? "block" : "none";
    if (checkbox.checked) {
      optionsHeading.classList.remove("text-muted");
      optionsHeading.classList.add("text-body");
    } else {
      optionsHeading.classList.remove("text-body");
      optionsHeading.classList.add("text-muted");
    }
  }

  if (checkbox.id === "toggleShowActions") {
    var actionsDiv = document.getElementById("actions");
    var actionsHeading = document.getElementById("actions-heading");
    actionsDiv.style.display = checkbox.checked ? "block" : "none";
    if (checkbox.checked) {
      actionsHeading.classList.remove("text-muted");
      actionsHeading.classList.add("text-body");
    } else {
      actionsHeading.classList.remove("text-body");
      actionsHeading.classList.add("text-muted");
    }
  }

  fetch("/", {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: dataString,
  }).then((response) => {
    if (reloadAfterRequest) {
      window.location.href = "/";
    }
  });
}

function onBrightnessValueChanged(value, channelId) {
  var brightnessAsPercentage = Math.floor((value / 4095) * 100);
  document.getElementById("rangeAsPercentage").textContent =
    "(" + brightnessAsPercentage + "%)";
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
  }).then((response) => {
    if (!response.ok && response.status === 400) {
      window.location.href = "/";
    }
  });
}
</script>
)html");
}

void Renderer::renderOptionsHeading(WiFiClient client) {
  bool toggleOptionsVisible = m_stateManager->getToggleShowOptions();

  char *toggleOptionsVisibleCheckedBuffer =
      toggleOptionsVisible ? m_checkedBuffer : m_emptyBuffer;

  char *mutedBuffer = toggleOptionsVisible ? m_emptyBuffer : m_textMutedBuffer;

  char outputBuffer[512];

  sprintf(outputBuffer,
          R"html(
<div class="d-flex align-items-center %s">
  <div id="options-heading" class="h3">%s</div>
  <div class="form-check form-switch ms-2 ">
    <input
      class="form-check-input"
      type="checkbox"
      name="toggleShowOptions"
      value="1"
      id="toggleShowOptions"
      onchange="sendCheckbox(this, false)"
      %s
    />
  </div>
</div>
    )html",
          mutedBuffer, I18N_HEADING_OPTIONS, toggleOptionsVisibleCheckedBuffer);

  pn(outputBuffer);
}

void Renderer::renderActionsHeading(WiFiClient client) {
  bool toggleActionsVisible = m_stateManager->getToggleShowActions();

  char *toggleActionsVisibleCheckedBuffer =
      toggleActionsVisible ? m_checkedBuffer : m_emptyBuffer;

  char *mutedBuffer = toggleActionsVisible ? m_emptyBuffer : m_textMutedBuffer;

  char outputBuffer[512];

  sprintf(outputBuffer,
          R"html(
<div class="d-flex align-items-center %s">
  <div id="actions-heading" class="h3">%s</div>
  <div class="form-check form-switch ms-2 ">
    <input
      class="form-check-input"
      type="checkbox"
      name="toggleShowActions"
      value="1"
      id="toggleShowActions"
      onchange="sendCheckbox(this, false)"
      %s
    />
  </div>
</div>
    )html",
          mutedBuffer, I18N_HEADING_ACTIONS, toggleActionsVisibleCheckedBuffer);

  pn(outputBuffer);
}

void Renderer::renderWebPage(WiFiClient client, bool foundRecursion) {
  uint16_t numChannels = m_stateManager->getNumChannels();
  bool toggleCompactDisplay = m_stateManager->getToggleCompactDisplay();
  bool renderAnchor = m_stateManager->getRenderAnchor();
  uint16_t anchorChannelId = m_stateManager->getAnchorChannelId();
  // Send a standard HTTP response header
  pn("HTTP/1.1 200 OK");
  pn("Content-type:text/html");
  pn();

  pn("<!DOCTYPE html>");
  pn(R"html(
<html>
  <head>
    <meta charset="UTF-8" />
    <meta
      name="viewport"
      content="width=device-width, initial-scale=1"
    />
    <link
      href="https://cdn.jsdelivr.net/npm/bootstrap@5.2.3/dist/css/bootstrap.min.css"
      rel="stylesheet"
      integrity="sha384-rbsA2VBKQhggwzxH7pPCaAqO46MgnOM80zW1RWuH61DGLwZJEdK2Kadq2F9CUG65"
      crossorigin="anonymous"
    />
    <link
      href="https://fonts.googleapis.com/css2?family=Grape+Nuts&display=swap"
      rel="stylesheet"
    />
    <link rel="icon" href="data:;base64,iVBORw0KGgo=" />
  )html");

  renderHeadJavascript(client);

  pn(R"html(
</head>
<body style="min-height: 100vh">
  <div class="container" style="min-height: 93vh">
    <div class="row justify-content-center">
      <div class="col col-12 col-sm-10 col-md-8 col-lg-6">
        <div class='h1 mt-4 mb-3' style="font-family: 'Grape Nuts', bold; font-size: 195%;"> 
        Bahnhofs Steuerung 2000
      </div>
      <form id="myForm" action="/" method="POST" accept-charset="UTF-8">
)html");

  if (foundRecursion) {
    char renderRecursionWarningBuffer[512];
    sprintf(renderRecursionWarningBuffer,
            R"html(
<div class='pb-3'>
  <span class='text-danger'>
  %s %d %s
  </span>
</div>
    )html",
            I18N_WARNING_RECURSION_BEFORE_MAX, MAX_RECURSION,
            I18N_WARNING_RECURSION_AFTER_MAX);

    pn(renderRecursionWarningBuffer);
  }

  if (m_stateManager->getRenderEditChannel() == true) {
    renderEditChannel(client);
  } else {
    renderOptionsHeading(client);
    renderOptions(client);
    renderActionsHeading(client);
    renderActions(client);

    char renderAnchorBuffer[20];
    sprintf(renderAnchorBuffer, "<h3>%s</h3>", I18N_HEADING_CHANNELS);
    pn(renderAnchorBuffer);

    if (renderAnchor) {
      char renderAnchorBuffer[100];
      renderAnchorBuffer[0] = 0;
      sprintf(renderAnchorBuffer,
              R"html(
            <div id="navigateTo" data-anchor="#channel-%d" style="display: none"></div>
                  )html",
              anchorChannelId);
      pn(renderAnchorBuffer);
    }

    pn("<div>");

    if (toggleCompactDisplay) {
      for (int channelId = 0; channelId < numChannels; channelId++) {
        bool renderHorizontalRule = true;

        if (channelId == numChannels - 1) {
          renderHorizontalRule = false;
        }

        renderChannelDetailCompact(client, channelId, renderHorizontalRule);
      }
    } else {
      for (int channelId = 0; channelId < numChannels; channelId++) {
        bool renderHorizontalRule = true;

        if (channelId == numChannels - 1) {
          renderHorizontalRule = false;
        }

        renderChannelDetail(client, channelId, renderHorizontalRule);
      }
    }

    pn("</div>");
  }

  pn(R"html(
            </form>
        </div>
      </div>
    </div>
  <div class='h6 mt-3 mb-3 d-flex justify-content-center' 
       style="font-family: 'Grape Nuts', bold; font-size: large;">
    made with ♥ by olivier.berlin
  </div>
<script>
document.addEventListener('DOMContentLoaded', function() {
  var navigationTag = document.getElementById('navigateTo');
  if (navigationTag) {
    var anchor = navigationTag.getAttribute('data-anchor');
    if (anchor) {
      window.location.hash = anchor;
    }
  }
});
</script>
</body>
</html>
)html");
}

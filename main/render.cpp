#include "render.h"
#include "eeprom.h"
#include "helpers.h"
#include <WiFiNINA.h>

char m_checkedBuffer[] = "checked";
char m_emptyBuffer[] = "";

void renderOptions(WiFiClient client, uint16_t numChannels,
                   bool toggleOneBasedAddresses, bool toggleCompactDisplay,
                   bool toggleForceAllOff, bool toggleForceAllOn,
                   bool toggleRandomChaos, bool toggleRandomEvents,
                   bool togglePropagateEvents, uint16_t channelIdToEdit) {

  char optionsOutputBuffer[4096];

  char *toggleOneBasedAddressesCheckedBuffer =
      toggleOneBasedAddresses ? m_checkedBuffer : m_emptyBuffer;

  char *toggleCompactDisplayCheckedBuffer =
      toggleCompactDisplay ? m_checkedBuffer : m_emptyBuffer;

  char *toggleForceAllOffCheckedBuffer =
      toggleForceAllOff ? m_checkedBuffer : m_emptyBuffer;

  char *toggleForceAllOnCheckedBuffer =
      toggleForceAllOn ? m_checkedBuffer : m_emptyBuffer;

  char *toggleRandomEventsCheckedBuffer =
      toggleRandomEvents ? m_checkedBuffer : m_emptyBuffer;

  char *togglePropagateEventsCheckedBuffer =
      togglePropagateEvents ? m_checkedBuffer : m_emptyBuffer;

  char *toggleRandomChaosCheckedBuffer =
      toggleRandomChaos ? m_checkedBuffer : m_emptyBuffer;

  char optionsInputBuffer[] =
      "<div class='h3'>\n"
      "  Optionen\n"
      "</div>\n\n"

      // --- Number of channels ---
      // Max value = Max number of boards (62 are max, but -1 because eeprom
      // address so 61) * number of pins => 61 * 16 = 976
      "<div class='row'>\n"
      "  <div class='col-3 d-flex align-items-center'>\n"
      "     Kanäle:\n"
      "  </div>\n"
      "  <div class='col-5'>\n"
      "    <input type='number' class='form-control w-100' "
      "name='numChannels' min='0' max='%d' value='%d'>\n"
      "  </div>\n"
      "  <div class='col-4 d-flex justify-content-end'>\n"
      "    <button class='btn btn-primary' type='submit' name='updateSettings' "
      "value='Absenden'>\n"
      "      Senden\n"
      "    </button>\n"
      "  </div>\n"
      "</div>\n\n"
      // --- /Number of channels ---

      "<br>\n\n"

      // --- 1 based addresses ---
      "<div class='form-check form-switch'>\n"
      "  <input class='form-check-input' type='checkbox' "
      "name='toggleOneBasedAddresses' "
      "value='1' id='toggleOneBasedAddresses' onchange='sendCheckbox(this, "
      "true)' %s />\n"
      "  <label class='form-check-label' "
      "for='toggleOneBasedAddresses'>\n"
      "      Adressierung startet bei 1\n"
      "  </label>\n"
      "</div>\n\n"
      // --- /1 based addresses ---

      // --- Compact view ---
      "<div class='form-check form-switch'>\n"
      "  <input class='form-check-input' type='checkbox' "
      "name='toggleCompactDisplay' "
      "value='1'  id='toggleCompactDisplay' onchange='sendCheckbox(this, "
      "true)' %s/>\n"
      "  <label class='form-check-label' "
      "for='toggleCompactDisplay'>\n"
      "    Kompakte Übersicht\n"
      "  </label>\n"
      "</div>\n\n"
      // --- /Compact view ---

      // --- Force all lights off ---
      "<div class='form-check form-switch'>\n"
      "  <input class='form-check-input' type='checkbox' "
      "name='toggleForceAllOff' "
      "value='1'  id='toggleForceAllOff' onchange='sendCheckbox(this, "
      "false)' %s/>\n"
      "  <label class='form-check-label' for='toggleForceAllOff'>\n"
      "    Alle Kanäle dauerhaft auf 0%%\n"
      "  </label>\n"
      "</div>\n\n"
      // --- Force all lights off ---

      // --- Force all lights 100% ---
      "<div class='form-check form-switch'>\n"
      "  <input class='form-check-input' type='checkbox' "
      "name='toggleForceAllOn' "
      "value='1' id='toggleForceAllOn' onchange='sendCheckbox(this, false)' "
      "%s/>\n"
      "  <label class='form-check-label' for='toggleForceAllOn'>\n"
      "    Alle Kanäle dauerhaft auf 100%%\n"
      "  </label>\n"
      "</div>\n\n"
      // --- /Force all lights 100% ---

      // --- Enable random events ---
      "<div class='form-check form-switch'>\n"
      "  <input class='form-check-input' type='checkbox' "
      "name='toggleRandomEvents'  "
      "value='1' role='switch' "
      "id='toggleRandomEvents' onchange='sendCheckbox(this, false)' %s/>\n"
      "  <label class='form-check-label' for='toggleRandomEvents'>\n"
      "    Zufällige Ereignisse\n"
      "  </label>\n"
      "</div>\n\n"
      // --- /Enable random events ---

      // --- Enable event propagation ---
      "<div class='form-check form-switch'>\n"
      "  <input class='form-check-input' type='checkbox' "
      "name='togglePropagateEvents' "
      "value='1' role='switch' "
      "id='togglePropagateEvents' onchange='sendCheckbox(this, false)' %s/>\n"
      "  <label class='form-check-label' "
      "for='togglePropagateEvents'>\n"
      "    Verknüpfungen aktiv\n"
      "  </label>\n"
      "</div>\n\n"
      // --- /Enable event propagation ---

      // --- Enable random blink ---
      "<div class='form-check form-switch'>\n"
      "  <input class='form-check-input' type='checkbox' "
      "name='toggleRandomChaos' value='1' role='switch' "
      "id='toggleRandomChaos' onchange='sendCheckbox(this, false)' %s/>\n"
      "  <label class='form-check-label' for='toggleRandomChaos'>\n"
      "    Verrücktes Blinken\n"
      "  </label>\n"
      "</div>\n\n"
      // --- /Enable random blink ---

      // This hidden field is meant to provide an 'easy' access to reset the
      // eeprom from the web interface
      "<input type='hidden'  name='clearEeprom' value='0'>\n";

  // Note for furure developers:
  // Currently the options are already ~2600 bytes. If you plan on adding many
  // more options and passing > 4k only double the output buffer after you
  // checked that your Arduino can handle such large buffers. My MKR 1010 WiFi
  // did not!

  sprintf(optionsOutputBuffer, optionsInputBuffer, MAX_TOTAL_CHANNELS,
          numChannels, toggleOneBasedAddressesCheckedBuffer,
          toggleCompactDisplayCheckedBuffer, toggleForceAllOffCheckedBuffer,
          toggleForceAllOnCheckedBuffer, toggleRandomEventsCheckedBuffer,
          togglePropagateEventsCheckedBuffer, toggleRandomChaosCheckedBuffer);

  pn(optionsOutputBuffer);
}

void renderButtons(WiFiClient client) {
  pt("<div>\n\n"

     "  <button class='btn btn-primary text-white me-2 mb-2' "
     "onclick=\"sendValue('resetAllChannels','1')\">\n"
     "    &nbsp;⌂&nbsp;\n"
     "  </button>\n\n"

     "  <button class='btn btn-primary text-white me-2 mb-2' "
     "onclick=\"sendValue('turnAllChannelsOff','1')\">\n"
     "    0%\n"
     "  </button>\n\n"

     "  <button class='btn btn-primary text-white me-2 mb-2' "
     "onclick=\"sendValue('turnAllChannels25','1')\">\n"
     "    25%\n"
     "  </button>\n\n"

     "  <button class='btn btn-primary text-white me-2 mb-2' "
     "onclick=\"sendValue('turnAllChannels50','1')\">\n"
     "    50%\n"
     "  </button>\n"

     "  <button class='btn btn-primary text-white me-2 mb-2' "
     "onclick=\"sendValue('turnAllChannels100','1')\" >\n"
     "    100%\n"
     "  </button>\n\n"

     "  <button class='btn btn-primary text-white me-2 mb-2' "
     "onclick=\"sendValue('turnEvenChannelsOn','1')\" >\n"
     "    Gerade\n"
     "  </button>\n\n"

     "  <button class='btn btn-primary text-white me-2 mb-2' "
     "onclick=\"sendValue('turnOddChannelsOn','1')\" >\n"
     "    Ungerade\n"
     "  </button>\n"

     "  <button class='btn btn-primary text-white me-2 mb-2' "
     "onclick=\"sendValue('countBinary','1')\" >\n"
     "    01010011\n"
     "  </button >\n\n"

     "</div>\n\n");
}

void renderEditChannel(WiFiClient client, bool renderAnchor,
                       uint16_t anchorChannelId, uint16_t numChannels,
                       bool toggleOneBasedAddresses, uint16_t channelIdToEdit) {
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
  char inputBuffer[] =
      "<h3>Kanal %d Bearbeiten</h3>\n\n"

      "<input type='hidden'  name='channelId' value='%d'>\n\n"

      "<div class='row'>\n"
      "  <div class='col'>\n"
      "    Beschreibung\n"
      "  </div>\n"
      "  <div class='col d-flex flex-fill justify-content-end'>\n"
      "<input class='form-control' type='text' maxlength='%d' size='%d' "
      "name='channelName' value='%s'>\n"
      "  </div>\n"
      "</div>\n\n"

      "<div class='form-check form-switch pt-3'>\n"
      "  <input class='form-check-input' type='checkbox' "
      "name='initialState' value='1' role='switch' id='initialState' "
      "value='1' %s/>\n"
      "  <label class='form-check-label' for='initialState'>\n"
      "    Startzustand\n"
      "  </label>\n"
      "</div>\n\n"

      "<div class='row pt-1'>\n"
      "  <div class='col'>\n"
      "    <div class='col d-flex'>\n"
      "      <div>\n"
      "        Helligkeit\n"
      "      </div>\n"
      "      <div id='rangeAsPercentage' class='ps-1 text-muted'>\n"
      "        (%d%%)\n"
      "      </div>\n"
      "    </div>\n"
      "  </div>\n"
      "  <div class='col-12'>\n"
      "    <input class='form-range' type='range' min='0' max='4095' "
      "name='channelBrightness' value='%d' "
      "onchange='onBrightnessValueChanged(this.value, %d)'>\n"
      "  </div>\n"
      "</div>\n\n"

      "<div class='form-check form-switch pt-3'>\n"
      "  <input class='form-check-input' type='checkbox' "
      "name='randomOn' value='1' role='switch' id='randomOn' "
      "value='1' %s/>\n"
      "  <label class='form-check-label' for='randomOn'>\n"
      "    Zufälliges Einschalten\n"
      "  </label>\n"
      "</div>\n"
      "<div class='row'>\n"
      "  <div class='col d-flex align-items-center'>\n"
      "    ~ Zufälle/h\n"
      "  </div>\n"
      "  <div class='col d-flex justify-content-end'>\n"
      "<input class='form-control' type='number' name='frequencyOn' min='0' "
      "max='255' value='%d'>\n"
      "  </div>\n"
      "</div>\n\n"

      "<div class='form-check form-switch pt-3'>\n"
      "  <input class='form-check-input' type='checkbox' "
      "name='randomOff' value='1' role='switch' id='randomOff' "
      "value='1' %s/>\n"
      "  <label class='form-check-label' for='randomOff'>\n"
      "    Zufälliges Ausschalten\n"
      "  </label>"
      "</div>\n\n"

      "<div class='row'>\n"
      "  <div class='col d-flex align-items-center'>\n"
      "    ~ Zufälle/h\n"
      "  </div>\n"
      "  <div class='col d-flex justify-content-end'>\n"
      "    <input class='form-control' type='number' name='frequencyOff' "
      "min='0' max='255' value='%d'>\n"
      "  </div>\n"
      "</div>\n\n"

      "<div class='form-check form-switch pt-3'>\n"
      "  <input class='form-check-input' type='checkbox' "
      "name='channelLinked' value='1' role='switch' id='channelLinked' "
      "value='1' %s/>\n"
      "  <label class='form-check-label' for='channelLinked'>\n"
      "    Verknüpft\n"
      "  </label>\n"
      "</div>\n\n"

      "<div class='row'>\n"
      "  <div class='col'>\n"
      "    Gesteuert durch Kanal\n"
      "  </div>\n"
      "  <div class='col d-flex align-items-center justify-content-end'>\n"
      "    <input class='form-control' type='number' "
      "name='linkedChannelId' min='%d' max='%d' value='%d'>\n"
      "  </div>\n"
      "</div>\n\n"

      "<br>\n\n"

      "<div class='row'>\n"
      "  <div class='col'>\n"
      "    <input class='btn btn-warning' type='submit' name='ignoreChannel' "
      "value='Verwerfen'/>\n"
      "  </div>\n"
      "  <div class='col d-flex justify-content-end'>\n"
      "    <input class='btn btn-primary ' type='submit' name='updateChannel' "
      "value='Speichern'/> &nbsp; \n"
      "  </div>\n"
      "</div>\n\n";

  sprintf(outputBuffer, inputBuffer, channelIdToDisplay, channelIdToEdit,
          maxChannelNameLength, maxChannelNameLength, m_channelNameBuffer,
          toggleInitialStateCheckedBuffer, brightnessAsPercentage,
          channelBrightness, channelIdToEdit,
          toggleHasRandomOnEventsCheckedBuffer, randomOnFreq,
          toggleHasRandomOffEventsCheckedBuffer, randomOffFreq,
          toggleIsChannelLinkedCheckedBuffer, smallesPossibleLinkedAddress,
          largestPossibleLinkedAddress, displayedLinkedChannelId);

  pn(outputBuffer);
}

void renderChannelDetail(WiFiClient client, bool toggleOneBasedAddresses,
                         uint16_t channelId, bool renderHorizontalRule) {
  readChannelNameFromEepromBufferToChannelNameBuffer(channelId);

  uint16_t brightness =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_BRIGHTNESS);
  uint8_t brightnessAsPercentage = (int)(((float)brightness / 4095) * 100);

  bool initialState =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_INITIAL_STATE);

  uint16_t channelIdToDisplay =
      toggleOneBasedAddresses ? channelId + 1 : channelId;

  uint8_t boardIndex = getBoardIndexForChannel(channelId);
  uint8_t subAddress = getBoardSubAddressForChannel(channelId);

  uint8_t boardIndexToDisplay =
      toggleOneBasedAddresses ? boardIndex + 1 : boardIndex;

  uint8_t boardSubAddressToDisplay =
      toggleOneBasedAddresses ? subAddress + 1 : subAddress;

  char enabledBuffer[] = "An";
  char disabledBuffer[] = "Aus";

  char yesBuffer[] = "Ja";
  char noBuffer[] = "Nein";

  char *toggleInitialStateCheckedBuffer =
      initialState ? enabledBuffer : disabledBuffer;

  // --- Prepare random on events ---
  bool randomOn =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_ON);

  uint8_t randomOnFreq =
      readUint8tForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_ON_FREQ);

  char *randomOnEventsEnabledBuffer = randomOn ? yesBuffer : noBuffer;

  char randomOnFrequencyHtmlInputBuffer[] =
      "  <div class='row'>"
      "    <div class='col'>"
      "      <span class='h6'>~Zufälle/h</span>"
      "    </div>"
      "    <div class='col'>"
      "      %d/h"
      "    </div>"
      "  </div>";

  char randomOnFrequencyHtmlOutputBuffer[512];

  sprintf(randomOnFrequencyHtmlOutputBuffer, randomOnFrequencyHtmlInputBuffer,
          randomOnFreq);

  char *randomOnEventsFrequencyHtmlToDisplayBuffer =
      randomOn ? randomOnFrequencyHtmlOutputBuffer : m_emptyBuffer;
  // --- /Prepare random on events ---

  // --- Prepare random off events ---
  bool randomOff =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_OFF);

  uint8_t randomOffFreq =
      readUint8tForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_OFF_FREQ);

  char *randomOffEventsEnabledBuffer = randomOff ? yesBuffer : noBuffer;

  char randomOffFrequencyHtmlInputBuffer[] =
      "  <div class='row'>"
      "    <div class='col'>"
      "      <span class='h6'>~Zufälle/h</span>"
      "    </div>"
      "    <div class='col'>"
      "      %d/h"
      "    </div>"
      "  </div>";

  char randomOffFrequencyHtmlOutputBuffer[512];

  sprintf(randomOffFrequencyHtmlOutputBuffer, randomOffFrequencyHtmlInputBuffer,
          randomOffFreq);

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
      <span class='h6'>
        Gesteuert durch Kanal
      </span>
    </div>
    <div class='col'>
      %d
    </div>
  </div>
  )html";

  char linkedChannelHtmlOutputBuffer[512];
  sprintf(linkedChannelHtmlOutputBuffer, linkedChannelHtmlInputBuffer,
          linkedChannelIdToDisplay);

  char *linkedChannelHtmlToDisplayBuffer =
      isLinked ? linkedChannelHtmlOutputBuffer : m_emptyBuffer;
  // --- /Prepare linked ---

  char horizontalRuleHtmlBuffer[] = "<hr class='mb-1 mt-1'/>";
  char *horizontalRuleHtmlToDisplayBuffer =
      renderHorizontalRule ? horizontalRuleHtmlBuffer : m_emptyBuffer;

  char outputBuffer[4096] = {0};
  char inputBuffer[] = R"html(
<div id='channel-%d' class='pl-1 pr-1'>
  <div class='row'>
    <div class='col-9'>
      <span class='h4'>
        Kanal %d
      </span > 
      Board %d, Pin %d
    </div>

    <div class='col-3'>
      <div class='d-flex justify-content-end'>
        <button class='btn text-warning' onclick="sendValue('turnChannelOn', '%d')">
          ⛭
        </button>
        <button class='btn' onclick="sendValue('turnChannelOff', '%d')">
          ⛭
        </button>
        <button class='btn' type='submit' name='editChannel' value='%d'>
          🖊
        </button >
      </div>
    </div>
  </div>

  <div class='row'>
    <div class='col'>
      <span class='h6'>
        Beschreibung
      </span>
    </div>
    <div class='col font-weight-bold'> 
      <b>
        %s
      </b>
    </div>
  </div>

  <div class='row'>
    <div class='col'>
      <span class='h6'>
        Startzustand
      </span>
    </div>
    <div class='col'>
      %s
    </div>
  </div>

  <div class='row'>
    <div class='col'>
      <span class='h6'>
        Helligkeit
      </span>
    </div>
    <div class='col'>
      %d%%
    </div>
  </div>

  <div class='row'>
    <div class='col'>
      <span class='h6'>
        Zufälliges Einschalten
      </span>
    </div>
    <div class='col'>
      %s
    </div>
  </div>

%s

  <div class='row'>
    <div class='col'>
      <span class='h6'>
        Zufälliges Ausschalten
      </span>
    </div>
    <div class='col'>
      %s
    </div>
  </div>

%s

  <div class='row'>
    <div class='col'>
      <span class='h6'>
        Verknüpft
      </span>
    </div>
    <div class='col'>
      %s
    </div>
  </div>

%s

</div>

%s
)html";

  sprintf(outputBuffer, inputBuffer, channelId, channelIdToDisplay,
          boardIndexToDisplay, boardSubAddressToDisplay, channelId, channelId,
          channelId, m_channelNameBuffer, toggleInitialStateCheckedBuffer,
          brightnessAsPercentage, randomOnEventsEnabledBuffer,
          randomOnEventsFrequencyHtmlToDisplayBuffer,
          randomOffEventsEnabledBuffer,
          randomOffEventsFrequencyHtmlToDisplayBuffer, isChannelLinkedBuffer,
          linkedChannelHtmlToDisplayBuffer, horizontalRuleHtmlToDisplayBuffer);

  pt(outputBuffer);
}

void renderChannelDetailCompact(WiFiClient client, bool toggleOneBasedAddresses,
                                uint16_t channelId, bool renderHorizontalRule) {
  readChannelNameFromEepromBufferToChannelNameBuffer(channelId);

  int boardIndex = getBoardIndexForChannel(channelId);
  int subAddress = getBoardSubAddressForChannel(channelId);

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
  char channelDetailBuffer[] =
      "<div id='channel-%d' class='pl-1 pr-1'>"
      "  <div class='d-flex g-0 align-items-center'>"
      "    <div class='d-flex flex-fill align-items-center'>"
      "      <div class='text-muted'>"
      "        %d."
      "      </div>"
      "      <button class='btn p-0 ps-1 flex-fill align-items-start' "
      "type='submit' name='editChannel' value='%d'>"
      "        <p class='text-start m-0'>"
      "          %s"
      "        </p>"
      "      </button>"
      "      <div class='text-muted'>"
      "        %d%%"
      "      </div>"
      "    </div>"
      "    <div class='d-flex'>"
      "      <button class='btn text-warning px-1 px-sm-2 px-md-3' "
      "onclick=\"sendValue('turnChannelOn','%d')\">⛭</button>"
      "      <button class='btn px-1 px-sm-2 px-md-3' "
      "onclick=\"sendValue('turnChannelOff','%d')\">⛭</button>"
      "    </div>"
      "  </div>"
      "</div>"
      "%s";

  uint16_t channelIdToDisplay =
      toggleOneBasedAddresses ? channelId + 1 : channelId;

  char channelNameToDisplay[MAX_CHANNEL_NAME_LENGTH];

  if (strcmp(m_channelNameBuffer, "")) {
    strcpy(channelNameToDisplay, m_channelNameBuffer);
  } else {
    sprintf(channelNameToDisplay, "Board %d, Pin %d", boardIndexToDisplay,
            boardSubAddressToDisplay);
  }

  sprintf(outputBuffer, channelDetailBuffer, channelId, channelIdToDisplay,
          channelId, channelNameToDisplay, brightnessAsPercentage, channelId,
          channelId, horizontalRuleHtmlToDisplayBuffer);

  pt(outputBuffer);
}

void renderHeadJavascript(WiFiClient client) {
  pn("<script>\n"
     "function sendValue(buttonName, buttonValue) {\n"
     "    event.preventDefault();\n"
     "    var dataString = encodeURIComponent(buttonName) + '=' + \n"
     "encodeURIComponent(buttonValue);\n"
     "    fetch('/', {\n"
     "       method: 'POST',\n"
     "       headers: {\n"
     "          'Content-Type': 'application/x-www-form-urlencoded'\n"
     "       },\n"
     "       body: dataString\n"
     "   });\n"
     "} \n\n"

     "function sendCheckbox(checkbox, reloadAfterRequest) {\n"
     "  var dataString = encodeURIComponent(checkbox.name) + '=' + \n"
     "  encodeURIComponent(checkbox.checked ? 1 : 0);\n"
     "  fetch('/', {\n"
     "      method: 'POST',\n"
     "      headers: {\n"
     "          'Content-Type': 'application/x-www-form-urlencoded'\n"
     "      },\n"
     "      body: dataString\n"
     "  })\n"
     "  .then(response => {\n"
     "    if (reloadAfterRequest) {\n"
     "      window.location.href = '/'; \n"
     "    }\n"
     "  })\n"
     "} \n\n"

     "function onBrightnessValueChanged(value, channelId) {\n"
     "  var brightnessAsPercentage = Math.floor((value / 4095) * 100);\n"
     "    document.getElementById('rangeAsPercentage').textContent = \n"
     "    '(' + brightnessAsPercentage + '%)';\n"
     "  var dataString = 'testBrightness=1&' + \n"
     "    encodeURIComponent('channelBrightness') + '=' + \n"
     "    encodeURIComponent(value) + '&' + "
     "    encodeURIComponent('channelId') + '=' +\n"
     "    encodeURIComponent(channelId);\n"
     "  fetch('/', {\n"
     "      method: 'POST',\n"
     "      headers: {\n"
     "        'Content-Type': 'application/x-www-form-urlencoded'\n"
     "      },\n"
     "      body: dataString\n"
     "  });\n"
     "}\n"
     "</script>");
}

void renderWebPage(WiFiClient client, bool foundRecursion,
                   bool renderWithOptionsVisible,
                   bool renderWithEditChannelVisible, bool renderAnchor,
                   uint16_t anchorChannelId, uint16_t numChannels,
                   bool toggleOneBasedAddresses, bool toggleCompactDisplay,
                   bool toggleForceAllOff, bool toggleForceAllOn,
                   bool toggleRandomChaos, bool toggleRandomEvents,
                   bool togglePropagateEvents, uint16_t channelIdToEdit) {

  // Send a standard HTTP response header
  pn("HTTP/1.1 200 OK");
  pn("Content-type:text/html");
  pn();

  pn("<!DOCTYPE html>\n");
  pn("<html>\n"
     "<head>\n"
     "  <meta charset='UTF-8'>\n"
     "  <meta name='viewport' content='width=device-width, "
     "initial-scale=1'>\n"
     "  <link "
     "href='https://cdn.jsdelivr.net/npm/bootstrap@5.2.3/dist/css/"
     "bootstrap.min.css' rel='stylesheet' "
     "integrity='sha384-"
     "rbsA2VBKQhggwzxH7pPCaAqO46MgnOM80zW1RWuH61DGLwZJEdK2Kadq2F9CUG65' "
     "crossorigin='anonymous'/>\n"
     "  <link "
     "href='https://fonts.googleapis.com/"
     "css2?family=Grape+Nuts&display=swap' "
     "rel='stylesheet'>\n"
     "  <link rel='icon' href='data:;base64,iVBORw0KGgo='>\n");

  renderHeadJavascript(client);

  pn("</head>\n"
     "<body style='min-height:100vh'>\n"
     "  <div class='container' style='min-height:93vh'>\n"
     "    <div class='row justify-content-center'>\n"
     "      <div class='col col-12 col-sm-10 col-md-8 col-lg-6'>\n"
     "        <div class='h1 mt-4 mb-3' style=\"font-family: 'Grape Nuts', "
     "bold; "
     "font-size: 195%;\">\n"
     "          Bahnhofs Steuerung 2000\n"
     "        </div>\n"
     "        <form id='myForm' action='/' method='POST' "
     "accept-charset='UTF-8'>\n");

  if (foundRecursion) {
    pt("          <div class='pb-3'>\n"
       "            <span class='text-danger'>Achtung: "
       "Schleife oder zu "
       "tiefe Verschachtelung (> ");
    pt(MAX_RECURSION);
    pn(") in "
       "verknüpften Kanälen "
       "entdeckt. Bitte überprüfe alle Verknüpfungen auf Schleifen oder "
       "erhöhe "
       "die maximale Verschachtelungstiefe!\n"
       "            </span>\n"
       "          </div>\n");
  }

  if (renderWithEditChannelVisible == true) {
    renderEditChannel(client, renderAnchor, anchorChannelId, numChannels,
                      toggleOneBasedAddresses, channelIdToEdit);
  } else {
    renderOptions(client, numChannels, toggleOneBasedAddresses,
                  toggleCompactDisplay, toggleForceAllOff, toggleForceAllOn,
                  toggleRandomChaos, toggleRandomEvents, togglePropagateEvents,
                  channelIdToEdit);

    pn("<br>");

    pn("<h3>Aktionen</h3>");
    renderButtons(client);
    pn("<br>");
    pn("<h3>Kanäle</h3>");

    if (renderAnchor) {
      char renderAnchorBuffer[100];
      renderAnchorBuffer[0] = 0;
      sprintf(renderAnchorBuffer,
              "<div id='navigateTo' data-anchor='#channel-%d' "
              "style='display:none;'></div>",
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

        renderChannelDetailCompact(client, toggleOneBasedAddresses, channelId,
                                   renderHorizontalRule);
      }
    } else {
      for (int channelId = 0; channelId < numChannels; channelId++) {
        bool renderHorizontalRule = true;

        if (channelId == numChannels - 1) {
          renderHorizontalRule = false;
        }

        renderChannelDetail(client, toggleOneBasedAddresses, channelId,
                            renderHorizontalRule);
      }
    }

    pn("</div>");
  }

  const char pageEndBuffer[] = R"html(
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
)html";

  pn(pageEndBuffer);
}

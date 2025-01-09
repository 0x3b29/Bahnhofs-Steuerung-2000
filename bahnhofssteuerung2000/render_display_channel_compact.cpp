#include "eeprom.h"
#include "helpers.h"
#include "render.h"
#include "state_manager.h"
#include "symbols.h"
#include <WiFiNINA.h>

void Renderer::renderChannelDetailCompact(WiFiClient client,
                                          uint16_t channelId) {
  bool isChannelHiddenInCompactView = readBoolForChannelFromEepromBuffer(
      channelId, MEM_SLOT_HIDE_IN_COMPACT_VIEW);

  if (isChannelHiddenInCompactView) {
    return;
  }

  readChannelNameFromEepromBufferToChannelNameBuffer(channelId);

  int boardIndex = getBoardIndexForChannel(channelId);
  int subAddress = getBoardSubAddressForChannel(channelId);

  bool toggleOneBasedAddresses = m_stateManager->getToggleOneBasedAddresses();

  int boardIndexToDisplay =
      toggleOneBasedAddresses ? boardIndex + 1 : boardIndex;

  int boardSubAddressToDisplay =
      toggleOneBasedAddresses ? subAddress + 1 : subAddress;

  bool toggleUseCustomRange = readBoolForChannelFromEepromBuffer(
      channelId, MEM_SLOT_USES_OUTPUT_VALUE1);

  uint16_t value2 =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_OUTPUT_VALUE2);
  uint8_t value2AsPercentage = (int)(((float)value2 / 4095) * 100);

  uint16_t value1 =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_OUTPUT_VALUE1);
  uint8_t value1AsPercentage = (int)(((float)value1 / 4095) * 100);

  if (!toggleUseCustomRange) {
    value1 = 0;
  }

  uint16_t channelIdToDisplay =
      toggleOneBasedAddresses ? channelId + 1 : channelId;

  char channelNameToDisplay[MAX_CHANNEL_NAME_LENGTH];

  if (strcmp(m_channelNameBuffer, "")) {
    strcpy(channelNameToDisplay, m_channelNameBuffer);
  } else {
    snprintf(channelNameToDisplay, sizeof(channelNameToDisplay), "%s %d, %s %d",
             I18N_CHANNEL_BOARD, boardIndexToDisplay, I18N_CHANNEL_PIN,
             boardSubAddressToDisplay);
  }

  char outputBuffer[2048];
  uint16_t bufferSize = sizeof(outputBuffer);
  uint16_t written = 0;

  written += snprintf(outputBuffer + written, bufferSize - written,
                      R"html(
<div id="channel-%d" class="pl-1 pr-1">
  <div class="d-flex g-0 align-items-center">
                      )html",
                      channelIdToDisplay);

  written +=
      snprintf(outputBuffer + written, bufferSize - written,
               R"html(
    <div class="d-flex flex-fill align-items-center">
      <div class="text-muted">%d.</div>
      <button
        class="btn p-0 ps-2 flex-fill align-items-start"
        onclick="openEditChannelPage('%d')"
      >
        <p class="text-start m-0">%s</p>
      </button>
      )html",
               channelIdToDisplay, channelIdToDisplay, channelNameToDisplay);

  if (toggleUseCustomRange) {
    written += snprintf(outputBuffer + written, bufferSize - written,
                        R"html(
      <div class="text-muted">%d&nbsp;%% … %d&nbsp;%%</div>
                      )html",
                        value1AsPercentage, value2AsPercentage);
  } else {
    written += snprintf(outputBuffer + written, bufferSize - written,
                        R"html(
      <div class="text-muted">%d&nbsp;%%</div>
                      )html",
                        value2AsPercentage);
  }

  written += snprintf(outputBuffer + written, bufferSize - written, R"html(
    </div>
                      )html");

  char *leftSymbol;
  char *rightSymbol;
  char *leftClass;
  char *rightClass;

  if (toggleUseCustomRange) {
    leftSymbol = SYMBOL_SET_VALUE_1;
    rightSymbol = SYMBOL_SET_VALUE_2;
    leftClass = "text-primary";
    rightClass = "text-primary";

  } else {
    leftSymbol = SYMBOL_TURN_LIGHT_ON;
    rightSymbol = SYMBOL_TURN_LIGHT_OFF;
    leftClass = "";
    rightClass = "text-warning";
  }

  written += snprintf(outputBuffer + written, bufferSize - written,
                      R"html(
    <div class="d-flex">
      <button
        class="btn %s px-1 px-sm-2 px-md-3"
        onclick="sendValue('setChannelToValue1','%d')"
      >
        %s
      </button>
      <button
        class="btn %s px-1 px-sm-2 px-md-3"
        onclick="sendValue('setChannelToValue2','%d')"
      >
        %s
      </button>
    </div>
                    )html",
                      leftClass, channelId, leftSymbol, rightClass, channelId,
                      rightSymbol);

  written += snprintf(outputBuffer + written, bufferSize - written,
                      R"html(
  </div>
                      )html");

  bool toggleShowSlider =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_SHOW_SLIDER);

  if (toggleShowSlider) {
    written +=
        renderSlider(outputBuffer + written, bufferSize - written, channelId);
  }

  written += snprintf(outputBuffer + written, bufferSize - written,
                      R"html(
</div>
                      )html");

  pn(client, outputBuffer);
}

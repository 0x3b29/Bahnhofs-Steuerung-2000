#include "eeprom.h"
#include "render.h"
#include "state_manager.h"

#include <WiFiNINA.h>

#include "helpers.h"

uint16_t Renderer::renderSlider(char *outputBuffer, uint16_t bufferSize,
                                uint16_t channelId) {

  bool toggleUseCustomRange = readBoolForChannelFromEepromBuffer(
      channelId, MEM_SLOT_USES_OUTPUT_VALUE1);

  uint16_t outputValue2 =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_OUTPUT_VALUE2);

  uint16_t outputValue1 = 0;

  if (toggleUseCustomRange) {
    outputValue1 = readUint16tForChannelFromEepromBuffer(
        channelId, MEM_SLOT_OUTPUT_VALUE1);
  }

  bool isInitialStateValue2 = readBoolForChannelFromEepromBuffer(
      channelId, MEM_SLOT_IS_START_VALUE_OUTPUT_VALUE2);

  uint16_t startValue;

  if (isInitialStateValue2) {
    startValue = outputValue2;
  } else {
    startValue = outputValue1;
  }

  int sliderMin;
  int sliderMax;

  if (outputValue2 > outputValue1) {
    sliderMin = outputValue1;
    sliderMax = outputValue2;
  } else {
    sliderMin = -outputValue1;
    sliderMax = -outputValue2;
  }

  return snprintf(outputBuffer, bufferSize,
                  R"html(
  <div class="row pt-1">
    <div class="col-12">
      <input
        class="form-range"
        type="range"
        min="%d"
        max="%d"
        name="outputValue2"
        value="%d"
        onchange="onSliderValueChanged(Math.abs(this.value), %d)"
      />
    </div>
  </div> 
                  )html",
                  sliderMin, sliderMax, startValue, channelId);
}

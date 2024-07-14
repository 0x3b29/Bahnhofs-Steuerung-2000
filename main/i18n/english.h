#ifndef english_h
#define english_h

#define I18N_OPTIONS_CHANNELS "Channels"
#define I18N_OPTIONS_SEND "Send"
#define I18N_OPTIONS_ONE_BASED_ADDRESSES "Addressing starts at 1"
#define I18N_OPTIONS_COMPACT_VIEW "Compact View"
#define I18N_OPTIONS_FORCE_ALL_OFF "All channels permanently at 0%"
#define I18N_OPTIONS_FORCE_ALL_ON "All channels permanently at 100%"
#define I18N_OPTIONS_RANDOM_EVENTS "Random Events"
#define I18N_OPTIONS_PROPAGATE_EVENTS "Linkages Active"
#define I18N_OPTIONS_CRAZY_BLINK "Crazy Blinking"
#define I18N_OPTIONS_RUNNING_LIGHTS "Running Lights"

#define I18N_ACTIONS_EVEN "Even"
#define I18N_ACTIONS_ODD "Odd"

#define I18N_EDIT_CHANNEL "Channel"
#define I18N_EDIT_EDIT "Edit"
#define I18N_EDIT_DESCRIPTION "Description"
#define I18N_EDIT_INITIAL_STATE_ON "Initial State On"
#define I18N_EDIT_BRIGHTNESS "Brightness"
#define I18N_EDIT_RANDOM_ON "Randomly Turn On"
#define I18N_EDIT_RANDOM_FREQ "~ Events/h"
#define I18N_EDIT_RANDOM_OFF "Randomly Turn Off"
#define I18N_EDIT_LINKED "Linked"
#define I18N_EDIT_CONTROLLED_BY_CHANNEL "Controlled by Channel"
#define I18N_EDIT_SHOW_SLIDER "Show Slider Control"
#define I18N_EDIT_DISCARD "Discard"
#define I18N_EDIT_SAVE "Save"

#define I18N_CHANNEL_CHANNEL "Channel"
#define I18N_CHANNEL_BOARD "Board"
#define I18N_CHANNEL_PIN "Pin"
#define I18N_CHANNEL_DESCRIPTION "Description"
#define I18N_CHANNEL_START_STATE "Initial State"
#define I18N_CHANNEL_BRIGHTNESS "Brightness"
#define I18N_CHANNEL_RANDOMLY_ON "Randomly Turn On"
#define I18N_CHANNEL_RANDOM_FREQ "~ Events/h"
#define I18N_CHANNEL_RANDOMLY_OFF "Randomly Turn Off"
#define I18N_CHANNEL_LINKED "Linked"
#define I18N_CHANNEL_COMMANDED_BY_CHANNEL "Controlled by Channel"
#define I18N_IS_HIDDEN_IN_COMPACT_VIEW "Hidden in Compact View"
#define I18N_CHANNEL_ON "On"
#define I18N_CHANNEL_OFF "Off"
#define I18N_CHANNEL_YES "Yes"
#define I18N_CHANNEL_NO "No"

#define I18N_HEADING_OPTIONS "Options"
#define I18N_HEADING_ACTIONS "Actions"
#define I18N_HEADING_CHANNELS "Channels"

#define I18N_WARNING_RECURSION_BEFORE_MAX                                      \
  "Warning: A loop or too deep nesting (more than"
#define I18N_WARNING_RECURSION_AFTER_MAX                                       \
  "levels) was detected in the linked channels. Please check all linkages "    \
  "for loops or increase the maximum nesting depth!"

#endif // english_h

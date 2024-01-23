#ifndef main_h
#define main_h

#include <Arduino.h>

#define st Serial.print
#define sn Serial.println

#define PWM_BOARDS 8
#define CHANNELS_PER_PWM_BOARD 16
#define MAX_TOTAL_CHANNELS PWM_BOARDS * CHANNELS_PER_PWM_BOARD

#define PWM_REFRESH_RATE  1042

#define MAX_RECURSION 5

#define MAX_CHANNEL_NAME_LENGTH 20

// Memory Slots for general settings
#define MEM_SLOT_CHANNELS  0
#define MEM_SLOT_FORCE_ALL_ON  2
#define MEM_SLOT_FORCE_ALL_OFF  3
#define MEM_SLOT_RANDOM  4

// Memory Slots for each channel
#define MEM_SLOT_BRIGHTNESS  30
#define MEM_SLOT_RANDOM_ON  32
#define MEM_SLOT_RANDOM_ON_FREQ  33
#define MEM_SLOT_RANDOM_OFF 34
#define MEM_SLOT_RANDOM_OFF_FREQ  35
#define MEM_SLOT_IS_LINKED  36
#define MEM_SLOT_LINKED_CHANNEL  37
#define MEM_SLOT_INITIAL_STATE 39
#define MEM_SLOT_ONE_BASED_ADDRESSES 40

#define MEM_SLOT_CRC 62

#endif // MAIN
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

#endif // MAIN
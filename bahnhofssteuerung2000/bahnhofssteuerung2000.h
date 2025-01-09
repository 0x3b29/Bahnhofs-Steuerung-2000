#ifndef bahnhofssteuerung2000_h
#define bahnhofssteuerung2000_h
#include <Arduino.h>

// In this file are some parameters that can be used to customise the project.
// Lines that start with // are excluded, lines that start with any other
// character are not.

// Select your prefered language for the web interface.
// Include only one single line with the language you would like to use.
// Exclude all the language lines.
//#include "i18n/english.h"
// #include "i18n/german.h"
#include "i18n/french.h"
// #include "i18n/dutch.h"
// #include "i18n/luxembourgish.h"

// Set this flag to true to see debug information on the serial interface
// Set to false for maximum performance
#define SHOW_DEBUG_INFO true

// Select the number of pwm boards you need. Please note that more than 16 pwm
// boards is currently not actively tested. 1 - 16 works fine.
#define PWM_BOARDS 16

// Analog servos usually use 50hz as frequency. This frequency can be altered
// for custom usecases.
#define PWM_LOW_REFRESH_RATE 50

// Digital servos often use a higher frequency.
// It is also recommended to drive LED's at the higher frequency to avoid
// flickering. If you plan on using camera equipment, you could try frequences
// that are not multiples of your recording frequences.
#define PWM_HIGH_REFRESH_RATE 200

// If complex group structures are required, the max recursion value can be
// increased to allow for a higher depth before showing an error message.
#define MAX_RECURSION 5
#define MAX_LERPING_CHANNELS 64
#define MAX_WAITING_CHANNELS 64

// Do not edit any of lines below this line
// -----------------------------------------------------------------------------
#define CHANNELS_PER_PWM_BOARD 16
#define MAX_TOTAL_CHANNELS PWM_BOARDS *CHANNELS_PER_PWM_BOARD

#define st Serial.print
#define sn Serial.println

#endif // MAIN
# Important technical Information about Bahnhofssteuerung 2000

## Disclaimer
DO NOT CONNECT MAINS VOLTAGE directly to any part of this Arduino project. Arduino boards and related components are designed for low-voltage DC circuits only and are not rated to handle high-voltage AC from mains power.

Connecting mains voltage to an Arduino or its peripherals can:

* Cause serious personal injury, including electric shock or burns.
* Damage or destroy your Arduino, components, and other equipment.
* Pose a fire hazard.
* Kill you or others!

By using this project, you acknowledge that you are solely responsible for following safe electrical practices and assume all risks associated with improper use.

When in doubt, consult a qualified electrician or expert.

## Libraries
This project requires a few libraries to run. Please install:
* CRC                               - by Rob Tillaart   - e.g. V 1.0.3
* WiFiNINA                          - by Arduino        - e.g. V 1.8.14
* Adafruit PWM Servo Driver Library	- by Adafruit       - e.g. V 3.0.2
* Adafruit BusIO                    - by Adafruit       - e.g. V 1.16.3

## Wifi
* Create a copy of _example_arduino_secrets.h 
* Rename the newly created copy it to arduino_secrets.h
* Fill in the credentials of your WiFi network

## Language
You can choose the language of the web interface. Only one language can be active at a time, and after changing the language the sketch needs to be rebuild and uploaded to your Arduino.
* Open bahnhfssteuerung2000.h
* Exclude english by adding // to the front of the line
* Include the language of your choice by removing the // in front of the language you like

## Number of Channels
 In the file bahnhfssteuerung2000.h you can define the max number of channels by altering `#define PWM_BOARDS 16`. For each pwm board, 16 additional channels will be available

## PWM frequency
In case you have special outputs or servos, you can define two pwm frequencies that are assignable to the pwm boards. A low and a higher frequency. For these, alter the values `PWM_LOW_REFRESH_RATE` and `PWM_HIGH_REFRESH_RATE` in the file bahnhfssteuerung2000.h


# Arduino-Browserbased-Many-LED-Controller

## Abstract
This project is a simple test platform I developed to control a bunch of LED's (on/off, dimming 0% - 100%) and the ability to save the state of the LED's. With the right hardware, up to 992 LED's can be controlled. I developed the project using an Arduino MKR WiFi 1010, multiple PCA9685 boards, an AT24C256 I2C 256k Bits EEPROM and a bunch of different LED's.

## Wiring
To wire this project up, you simply connect the SDA & SCL lines of the Arduino, the AT24C256 and the PCA9685 boards together as well as providing everything with the right voltage level. If you use more than 16 channels, you need an additional PCA9685 board for each set of 16 LED’s. You can chain a total of 62 boards. The LED's can be connected directly to the signal and ground lines of the PCA9685 boards since they come with integrated resistors.

## Features
* Web interface (provided through an Arduino based web server)
* A customizable total number of LED'S
* One slider for each LED's PWM
* A button to set random values to every channel
* A button to turn off every channel
* A button to save all the values to an EEPROM
* Automatic reload of the state from the EEPROM after powering up

## Note
* The Arduino will log its IP address to the serial terminal. But you can also check in your router for the device's IP. Just type it in the browser to get to the UI.
* If the Arduino was not yet ready, browsers tend to retry loading the page using https. Unfortunately, this is not obvious unless you click into the taskbar. Currently, this project only work using the http protocol

## Cavecasts
* Currently, using a lot of channels causes problems. I found on my Arduino ~350 LED's worked reliably but more resulted in crashes. Probably due to the use of String and searching for keys in a loop. Eventually I'll come back to this project and fix this but for now only ~100 LED's are enough for my needs

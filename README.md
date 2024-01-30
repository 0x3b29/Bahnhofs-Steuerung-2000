# Arduino-Browserbased-Many-LED-Controller

## Abstract
This project is a scalable, web based LED control platform I developed for my fathers model railway buildings. The main goal was to easily control a few hundred LEDs (on/off, dimming 0% - 100%), grouping some together and randomly turning some LEDs or groups on or off to create a lifelike simulation. 

## Required Components
* 1 x Arduino MKR 1010 WiFi
* 1 x 4 channel I2C logic level converter
* 1 x AT24C256 eeprom
* Up to 61* x PCA9685 Boards 
* Up to 976 single color LEDs
* Up to 305** RGB LEDs

(* Not 62 because one address in the usable address space of the PCA9685 boards is used by the eeprom)
(** 5 RGB LEDs per board * 61 boards)

## Features per channel
* Default state on or off after powering the entire system on
* A brightness value for the on state (0 … 4095)
* A flag to enabled random on events 
* Random on event frequency (0 … 255/h)
* A flag to enabled random off events
* Random off event frequency (0 … 255/h)
* A flag to set if the channel is linked
* A field for the other channels id that controls this channels on/off state
* A max 30 bytes long description. (Up to 30 characters)

## App Features
* A modern, mobile first web interface provided through a web server running locally on the Arduino
* A customizable total number of LED's
* A flag to globally enable or disable random events
* A flag to globally enable or disable channel links
* An extensive layout giving full detail for each channel
* A compact layout to give the name and buttons to turn the channel on and off
* Flags to globally force all LEDs permanently off or on (100% brightness)
* A few actions to momentarily switch all LEDs on, off, dim, only turn on odd or even channels and more
* Everything configured by the user is stored to the eeprom
* When loading data from eeprom, the integrity of the data is automatically checked
* I18n internationalization for all user facing texts. (Currently Luxembourgish, English, German, French, Dutch).
* Infinite loop detection and user notification on linked channels


## Hardware
After experimenting with different microcontrollers, the one that worked best for me was the Arduino MKR WiFi 1010. It is connected to all the other components via the I2C lines. To get the most brightness from your LED’s, you need to power the PCA9685 boards with 5V. Since the Arduino MKR WiFi 1010 is not 5V tolerant, you need to add an I2C signal converter that turns the 3.3V of the Arduino the the higher 5V for the PCA9685 boards. The eeprom can be placed on either side of the signal converter since it is 5V tolerant. The only thing to consider is that if you give it 5V Vcc, you also need to connect it to the 5V side of the I2C bus or it will fail to communicate.

## Wiring
To wire this project up, you connect the SDA & SCL lines of the Arduino through a logic level converter with an AT24C256 eeprom and the PCA9685 boards together as well as providing everything with the right voltage level.You need an additional PCA9685 board for each set of 16 LED’s. You can connect a total of 61 boards to the I2C bus. The bus can either be a chain or a star topology. The LED's can be connected directly to the signal and ground lines of the PCA9685 boards (Yellow = Signal, Black = GND) since they come with integrated 220 ohm resistors. Each channel can handle up to 25 mA, but at 5V the 220 ohm resistors allow only a maximum of ~23 mA to flow.

## Linked LEDs
For each channel, there is a flag that defines if the channel is linked or not. If specified that a link is active, the linked channel must be defined. This channel will be the channel that controls the current channel. At first, this might seem a bit counterintuitive to do it this way but it has a neat advantage. The other way would only allow for 1 to 1 channel relations (One channel could control only one other channel). By defining which channel controls the current channel, the same controlling channel can be defined for multiple other channels effectively forming a group or an 1 to n relation (One channel can control many other channels). Theoretically, one channel could control all the other channels. If one channel is controlling another channel, only the on / off command will be passed down, not the brightness. That way an LED will always light up with its predefined brightness. Also, a channel that is controlled can be controlling one or more other channels as well. This creates a relationship tree. To avoid locking up the microcontroller, a basic recursion detection algorithm is implemented that shows a warning if a circular link was detected.

## Installation
After wiring the project up, the Arduino can be programmed. First the example_arduino_secrets.h file needs to be copied and named to arduino_secrets.h. Then, the credentials for the WiFi need to get filled in. Also, in the file render.cpp, there is the possibility to define which language should be used for the frontend. Changing the language works by commenting the corresponding header file in. The user interface is accessible through any modern web browser. Simply type the Arduinos IP address in the address bar. After each startup, the Arduino will log its IP address to the serial terminal. But it is also possible to check the router for the device's IP address. Note that if the Arduino was not ready when navigating to the IP address, browsers tend to retry loading the page using https which will not work even if the Arduino is fully started. Unfortunately, this is not always obvious since browsers like chrome hide the protocol until the user navigates to the taskbar.

## Notes
### Note on the Arduino MKR 1010 WiFi
The official documentation says that the Arduino MKR 1010 WiFi can be powered using the VIN pin with 5V to 7V DC. However, I found during my tests that the WiFi did not connect reliably at 5V. After some experimentation, I found out that slightly increasing the voltage to 5.3V ensured that the WiFi connection worked much more consistently. I therefore opted to use two different power sources. One 5V Dc power source for the LEDs and the eeprom and one 6V DC power source for the Arduinos VIN. For my father, this was not necessary. His Arduino worked fine with 5V DC on the VIN pin.

### Note on Arduino Uno R4
During development of this project, the Arduino Uno R4 seemed to be a compelling option since it uses 5V on its pins. This eliminates the need for the I2C signal converter. But due to a limitation (or bug) in its firmware, the web serving capabilities were extremely slow. Although everything worked fine, the rendering of the webpage was taking several seconds which was a bad user experience. Therefore we opted for the Arduino MKR WiFi 1010 

### Note on TXS0108E
During development, I tried the TXS0108E logic level converter and I experienced lots of issues with ringing. This happens when the signals on the low and the high voltage sides start to reflect through the chip and the oscillation gets stronger and stronger. This breaks communication and only a complete reset was able to get everything back to a normal state. I was only able to get rid of this behavior by shortening the I2C bus cable lengths to very short  distances (< 5 cm). This chip might be suited if all the components are in close proximity. Otherwise I would strongly recommend a generic 4 channel I2C logic level converter or the spark fun logic level converter.

### Note on security
Currently, this project has no built in security. Every device in your WiFi can access the IP address and control all the channels. Furthermore, every setting can be modified. If a basic layer of security is necessary, use a separate password protected WiFi only for this purpose with no other devices connected.

### Note on electricity
Never connect any cable or circuit directly to mains voltage. Always use power supplies with the right voltages. Do not power your Arduino via the VIN pin when connecting it to a PC. There is a fair risk of damaging the PC and / or the Arduino.

## What's next
In the upcoming weeks, I'll try to add a few convenient features, find and fix the last remaining bugs and polish this project as much as I can. The planned features are 
* Memory banks for complete separate sets of settings (e.g. for daytime and nighttime)
* Optional password protection for settings
* Optional password protection for the entire interface 

## I need more
If there is anything else you want this project to do, you are free to write me a private message or an email using olivier.berlin@protonmail.com. 
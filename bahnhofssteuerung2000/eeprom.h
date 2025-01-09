#ifndef eeprom_h
#define eeprom_h

#include "bahnhofssteuerung2000.h"
#include <Arduino.h>

#define EEPROM_ADDRESS 0x50

// Memory Slots for general settings
#define MEM_SLOT_CHANNELS 0
#define MEM_SLOT_FORCE_ALL_ON 2
#define MEM_SLOT_FORCE_ALL_OFF 3
#define MEM_SLOT_RANDOM_CHAOS 4
#define MEM_SLOT_RANDOM_EVENTS 5
#define MEM_SLOT_PROPAGATE_EVENTS 6
#define MEM_SLOT_SHOW_OPTIONS 7
#define MEM_SLOT_SHOW_ACTIONS 8
#define MEM_SLOT_ONE_BASED_ADDRESSES 9
#define MEM_SLOT_COMPACT_DISPLAY 10
#define MEM_SLOT_RUNNING_LIGHTS 11
#define MEM_SLOT_HIGH_PWM 12

// Memory Slots for each channel
#define MEM_SLOT_OUTPUT_VALUE2 30
#define MEM_SLOT_DO_RANDOMLY_SET_VALUE2 32
#define MEM_SLOT_RANDOMLY_SET_VALUE2_FREQ 33
#define MEM_SLOT_DO_RANDOMLY_SET_VALUE1 34
#define MEM_SLOT_RANDOMLY_SET_VALUE1_FREQ 35
#define MEM_SLOT_IS_LINKED 36
#define MEM_SLOT_LINKED_CHANNEL 37
#define MEM_SLOT_IS_START_VALUE_OUTPUT_VALUE2 39
#define MEM_SLOT_HIDE_IN_COMPACT_VIEW 40
#define MEM_SLOT_SHOW_SLIDER 41
#define MEM_SLOT_USES_OUTPUT_VALUE1 42
#define MEM_SLOT_OUTPUT_VALUE1 43

#define MEM_SLOT_IS_LERPED 45
#define MEM_SLOT_LERP_TARGET_VALUE 46
#define MEM_SLOT_LERP_CURRENT_POS 48
#define MEM_SLOT_LERP_SPEED 52

#define MEM_SLOT_LINK_DELAY 56

// Memory Slot for general settings and each channel
#define MEM_SLOT_CRC 62

#define MAX_CHANNEL_NAME_LENGTH 30

// Each page is 64 Bytes long, we use one page per channel + 1, in page 0 we
// store general settings
#define EEPROM_PAGE_SIZE 64
#define MAX_EEPROM_RANGE EEPROM_PAGE_SIZE *(MAX_TOTAL_CHANNELS + 1)

extern uint8_t m_eepromBuffer[MAX_EEPROM_RANGE];
extern char m_channelNameBuffer[MAX_CHANNEL_NAME_LENGTH];

void clearEepromBuffer();
void clearEeprom();

void readFromEeprom(uint16_t readAddress, uint8_t *data, uint8_t length);
void writeToEeprom(uint16_t writeAddress, uint8_t *data, uint8_t length);
void writeToEepromBuffer(uint16_t writeAddress, uint8_t *data, uint8_t length);
void writeBoolToEepromBuffer(uint16_t writeAddress, bool value);
void writeUInt8ToEepromBuffer(uint16_t writeAddress, uint8_t value);
void writeUInt16ToEepromBuffer(uint16_t writeAddress, uint16_t value);
void writeFloatToEepromBuffer(uint16_t writeAddress, float value);

bool readBoolFromEepromBuffer(uint16_t readAddress);
void readFromEepromBuffer(uint16_t readAddress, uint8_t *data, uint8_t length);
float readFloatFromEepromBuffer(uint16_t readAddress);
uint16_t readUInt16FromEepromBuffer(uint16_t readAddress);
uint8_t readUInt8FromEepromBuffer(uint16_t readAddress);
void readChannelNameFromEepromBufferToChannelNameBuffer(int channel);

void writeChannelNameFromChannelNameBufferToEepromBuffer(int channel);
void writeUint8tForChannelToEepromBuffer(int channel, int memorySlot,
                                         uint8_t value);
void writeUint16tForChannelToEepromBuffer(int channel, int memorySlot,
                                          uint16_t value);
void writeFloatForChannelToEepromBuffer(int channel, int memorySlot,
                                        float value);

float readFloatForChannelFromEepromBuffer(int channel, int memorySlot);
uint16_t readUint16tForChannelFromEepromBuffer(int channel, int memorySlot);
bool readBoolForChannelFromEepromBuffer(int channel, int memorySlot);
uint8_t readUint8tForChannelFromEepromBuffer(int channel, int memorySlot);

uint8_t readUint8tForChannelFromEepromBuffer(int channel, int memorySlot);
void dumpEepromData(int startAddress, int endAddress);

void loadPageFromEepromToBuffer(int page);
void writePageFromBufferToEeprom(int page);
bool isPageIntegrityGood(uint16_t page);
void writePageIntegrity(int page);
void wipePage(int page);
void loadPageFromEepromToEepromBufferAndCheckIntegrity(int page);

#endif // eeprom_h
#ifndef eeprom_h
#define eeprom_h

#include "CRC.h"
#include "CRC16.h"
#include "main.h"
#include <Arduino.h>
#include <Wire.h>

#define EEPROM_ADDRESS 0x50

// Memory Slots for general settings
#define MEM_SLOT_CHANNELS 0
#define MEM_SLOT_FORCE_ALL_ON 2
#define MEM_SLOT_FORCE_ALL_OFF 3
#define MEM_SLOT_RANDOM_CHAOS 4
#define MEM_SLOT_RANDOM_EVENTS 5
#define MEM_SLOT_PROPAGATE_EVENTS 6

// Memory Slots for each channel
#define MEM_SLOT_BRIGHTNESS 30
#define MEM_SLOT_RANDOM_ON 32
#define MEM_SLOT_RANDOM_ON_FREQ 33
#define MEM_SLOT_RANDOM_OFF 34
#define MEM_SLOT_RANDOM_OFF_FREQ 35
#define MEM_SLOT_IS_LINKED 36
#define MEM_SLOT_LINKED_CHANNEL 37
#define MEM_SLOT_INITIAL_STATE 39
#define MEM_SLOT_ONE_BASED_ADDRESSES 40
#define MEM_SLOT_COMPACT_DISPLAY 41

#define MEM_SLOT_CRC 62

#define MAX_CHANNEL_NAME_LENGTH 30

// Each page is 64 Bytes long, we use one page per channel + 1, in page 0 we
// store general settings
#define EEPROM_PAGE_SIZE 64
#define MAX_EEPROM_RANGE EEPROM_PAGE_SIZE *(MAX_TOTAL_CHANNELS + 1)
#define PAGE_BUFFER_SIZE 8192

extern uint8_t m_eepromBuffer[MAX_EEPROM_RANGE];
extern char m_channelNameBuffer[MAX_CHANNEL_NAME_LENGTH];
extern CRC16 crc;

void clearEepromBuffer();
void clearEeprom();

void readFromEeprom(uint16_t readAddress, uint8_t *data, uint8_t length);
void writeToEeprom(uint16_t writeAddress, uint8_t *data, uint8_t length);
void writeToEepromBuffer(uint16_t writeAddress, uint8_t *data, uint8_t length);
void writeUInt16ToEepromBuffer(uint16_t writeAddress, uint16_t value);

void readFromEepromBuffer(uint16_t readAddress, uint8_t *data, uint8_t length);
uint16_t readUInt16FromEepromBuffer(uint16_t readAddress);
void readChannelNameFromEepromBufferToChannelNameBuffer(int channel);

void writeChannelNameFromChannelNameBufferToEepromBuffer(int channel);
void writeUint8tToEepromBuffer(int channel, int memorySlot, uint8_t value);
void writeUint16tForChannelToEepromBuffer(int channel, int memorySlot,
                                          uint16_t channelBrightness);

uint16_t readUint16tForChannelFromEepromBuffer(int channel, int memorySlot);
bool readBoolForChannelFromEepromBuffer(int channel, int memorySlot);
uint8_t readUint8tForChannelFromEepromBuffer(int channel, int memorySlot);

uint8_t readUint8tForChannelFromEepromBuffer(int channel, int memorySlot);
void dumpEepromData(int startAddress, int endAddress);

void loadPageFromEepromToBuffer(int page);
void writePageFromBufferToEeprom(int page);
bool isPageIntegrityGood(uint8_t page);
void writePageIntegrity(int page);
void wipePage(int page);
void loadPageAndCheckIntegrity(int page);

#endif // eeprom_h
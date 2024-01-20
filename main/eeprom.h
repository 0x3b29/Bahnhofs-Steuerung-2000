#ifndef eeprom_h
#define eeprom_h

#include "CRC.h"
#include "CRC16.h"
#include "main.h"
#include <Arduino.h>
#include <Wire.h>

#define EEPROM_ADDRESS 0x50

// Each page is 64 Bytes long, we use one page per channel + 1, in page 0 we
// store general settings
#define PAGE_SIZE 64
#define MAX_EEPROM_RANGE PAGE_SIZE *(MAX_TOTAL_CHANNELS + 1)
#define PAGE_BUFFER_SIZE 8192

extern uint8_t m_eepromBuffer[MAX_EEPROM_RANGE];
extern char m_channelNameBuffer[MAX_CHANNEL_NAME_LENGTH + 1];
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
                                          uint16_t channelValue);

uint16_t readUint16tForChannelFromEepromBuffer(int channel, int memorySlot);
bool readBoolForChannelFromEepromBuffer(int channel, int memorySlot);
uint8_t readUint8tForChannelFromEepromBuffer(int channel, int memorySlot);

uint8_t readUint8tForChannelFromEepromBuffer(int channel, int memorySlot);
void dumpEepromData(int startAddress, int endAddress);

void loadPageFromEepromToBuffer(int page);
void writePageFromBufferToEeprom(int page);
bool isPageIntegrityGood(uint8_t page);
void writePageIntegrity(int page);
void wipePage(int page) ;
void loadPageAndCheckIntegrity(int page);

#endif // eeprom_h
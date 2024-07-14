#include "eeprom.h"
#include "CRC.h"
#include "CRC16.h"
#include <Wire.h>
uint8_t m_eepromBuffer[MAX_EEPROM_RANGE];
char m_channelNameBuffer[MAX_CHANNEL_NAME_LENGTH];

CRC16 crc;

void readFromEeprom(uint16_t readAddress, uint8_t *data, uint8_t length) {
  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.write((byte)((readAddress & 0xFF00) >> 8));
  Wire.write((byte)(readAddress & 0x00FF));
  Wire.endTransmission();

  Wire.requestFrom(EEPROM_ADDRESS, length);
  int i;
  for (i = 0; i < length; i++) {
    if (Wire.available()) {
      data[i] = Wire.read();
    } else {
      data[i] = 0;
    }
  }
}

void writeToEeprom(uint16_t writeAddress, uint8_t *data, uint8_t length) {
  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.write((byte)((writeAddress & 0xFF00) >> 8));
  Wire.write((byte)(writeAddress & 0x00FF));
  uint8_t i;
  for (i = 0; i < length; i++) {
    Wire.write(data[i]);
  }
  Wire.endTransmission();
  delay(20);
}

void writeUInt16ToEepromBuffer(uint16_t writeAddress, uint16_t value) {
  uint8_t data[2]; // Create a byte array to hold the uint16_t value
  data[0] = (value >> 8) & 0xFF; // Extract the high byte
  data[1] = value & 0xFF;        // Extract the low byte

  writeToEepromBuffer(writeAddress, data, 2);
}

void writeBoolToEepromBuffer(uint16_t writeAddress, bool value) {
  uint8_t uint8tValue = 0;

  if (value) {
    uint8tValue = 1;
  }

  writeToEepromBuffer(writeAddress, &uint8tValue, 1);
}

void writeToEepromBuffer(uint16_t writeAddress, uint8_t *data, uint8_t length) {
  for (int i = 0; i < length; i++) {
    m_eepromBuffer[writeAddress + i] = data[i];
  }
}

boolean readBoolFromEepromBuffer(uint16_t readAddress) {
  uint8_t value = m_eepromBuffer[readAddress];
  bool valueAsBool = value != 0;
  return valueAsBool;
}

void readFromEepromBuffer(uint16_t readAddress, uint8_t *data, uint8_t length) {
  for (int i = 0; i < length; i++) {
    data[i] = m_eepromBuffer[readAddress + i];
  }
}

uint16_t readUInt16FromEepromBuffer(uint16_t readAddress) {
  uint8_t data[2];
  readFromEepromBuffer(readAddress, data, 2);

  uint16_t value = ((uint16_t)data[0] << 8) | data[1];
  return value;
}

void readChannelNameFromEepromBufferToChannelNameBuffer(int channel) {
  uint16_t startAddress = (channel + 1) * 64;
  readFromEepromBuffer(startAddress, (uint8_t *)m_channelNameBuffer,
                       MAX_CHANNEL_NAME_LENGTH);
  m_channelNameBuffer[MAX_CHANNEL_NAME_LENGTH - 1] = '\0';
}

void writeChannelNameFromChannelNameBufferToEepromBuffer(int channel) {
  uint16_t startAddress = (channel + 1) * 64;
  writeToEepromBuffer(startAddress, (uint8_t *)m_channelNameBuffer,
                      MAX_CHANNEL_NAME_LENGTH);
}

void writeUint8tToEepromBuffer(int channel, int memorySlot, uint8_t value) {
  uint16_t startAddress = (channel + 1) * 64 + memorySlot;
  writeToEepromBuffer(startAddress, &value, 1);
}

void writeUint16tForChannelToEepromBuffer(int channel, int memorySlot,
                                          uint16_t outputValue1) {
  int startAddress = (channel + 1) * 64 + memorySlot;
  writeUInt16ToEepromBuffer(startAddress, outputValue1);
}

uint16_t readUint16tForChannelFromEepromBuffer(int channel, int memorySlot) {
  int startAddress = (channel + 1) * 64 + memorySlot;
  return readUInt16FromEepromBuffer(startAddress);
}

bool readBoolForChannelFromEepromBuffer(int channel, int memorySlot) {
  int startAddress = (channel + 1) * 64 + memorySlot;
  uint8_t result = 0;

  readFromEepromBuffer(startAddress, &result, 1);

  if (result == 0) {
    return false;
  } else {
    return true;
  }
}

uint8_t readUint8tForChannelFromEepromBuffer(int channel, int memorySlot) {
  int startAddress = (channel + 1) * 64 + memorySlot;
  uint8_t result = 0;

  readFromEepromBuffer(startAddress, &result, 1);
  return result;
}

void dumpEepromData(int startAddress, int endAddress) {
  char lineBufferEEPROM[50]; // Buffer for EEPROM line data
  char lineBufferMirror[50]; // Buffer for m_eepromBuffer line data
  char asciiBufferEEPROM[9]; // Buffer for EEPROM ASCII characters
  char asciiBufferMirror[9]; // Buffer for m_eepromBuffer ASCII characters
  int bufferIndex = 0;

  for (int i = startAddress; i <= endAddress; i++) {
    // Handle the start of a new line
    if ((i - startAddress) % 8 == 0) {
      if (i != startAddress) {
        // Print the accumulated line data for EEPROM and m_eepromBuffer
        asciiBufferEEPROM[bufferIndex] = '\0';
        asciiBufferMirror[bufferIndex] = '\0';
        Serial.print(lineBufferEEPROM);
        Serial.print("  ");
        Serial.print(asciiBufferEEPROM);
        Serial.print("    ");
        Serial.print(lineBufferMirror);
        Serial.print("  ");
        Serial.println(asciiBufferMirror);
        bufferIndex = 0;
      }
      if (i != startAddress && (i - startAddress) % EEPROM_PAGE_SIZE == 0) {
        Serial.println(); // Newline to separate pages
      }
      snprintf(lineBufferEEPROM, sizeof(lineBufferEEPROM),
               "%04X: ", i);             // Start a new line for EEPROM data
      strcpy(lineBufferMirror, "     "); // Align m_eepromBuffer data
    }

    uint8_t byteValueEEPROM = 0;
    readFromEeprom(i, &byteValueEEPROM, 1);

    // Add HEX and ASCII for EEPROM data
    char hexBuffer[5];
    snprintf(hexBuffer, sizeof(hexBuffer), "%02X ", byteValueEEPROM);
    strcat(lineBufferEEPROM, hexBuffer);
    asciiBufferEEPROM[bufferIndex] =
        (byteValueEEPROM >= 32 && byteValueEEPROM <= 126) ? byteValueEEPROM
                                                          : '.';

    // Add HEX and ASCII for m_eepromBuffer data
    snprintf(hexBuffer, sizeof(hexBuffer), "%02X ", m_eepromBuffer[i]);
    strcat(lineBufferMirror, hexBuffer);
    asciiBufferMirror[bufferIndex] =
        (m_eepromBuffer[i] >= 32 && m_eepromBuffer[i] <= 126)
            ? m_eepromBuffer[i]
            : '.';

    bufferIndex++;

    // Group the bytes in sets of four, separated by two spaces
    if ((i - startAddress) % 4 == 3) {
      strcat(lineBufferEEPROM, "  ");
      strcat(lineBufferMirror, "  ");
    }
  }

  // Print any remaining data at the end
  if (bufferIndex > 0) {
    asciiBufferEEPROM[bufferIndex] = '\0';
    asciiBufferMirror[bufferIndex] = '\0';
    Serial.print(lineBufferEEPROM);
    Serial.print("  ");
    Serial.print(asciiBufferEEPROM);
    Serial.print("    ");
    Serial.print(lineBufferMirror);
    Serial.print("  ");
    Serial.println(asciiBufferMirror);
  }
}

void clearEeprom() {
  uint8_t resetChar = '\0';
  int updateInterval = 128;
  Serial.print("Erasing eeprom started");

  for (int i = 0; i < MAX_EEPROM_RANGE; i++) {
    writeToEeprom(i, &resetChar, 1);

    if (i % updateInterval == 0) {
      Serial.print("Erasing eeprom at ");
      Serial.print(i);
      Serial.print(" of ");
      Serial.print(MAX_EEPROM_RANGE);
      Serial.print(" (");
      Serial.print(((float)i / MAX_EEPROM_RANGE) * 100);
      Serial.println("%)");
    }
  }

  Serial.println("Erasing eeprom finished");
}

void loadPageFromEepromToBuffer(int page) {
  uint16_t readAddress = page * EEPROM_PAGE_SIZE;

  if (readAddress < MAX_EEPROM_RANGE) {
    readFromEeprom(readAddress, &m_eepromBuffer[readAddress], EEPROM_PAGE_SIZE);
  } else {
    Serial.println("Error: loadPageFromEepromToBuffer tried to load memory "
                   "outside MAX_EEPROM_RANGE");
  }
}

void writePageFromBufferToEeprom(int page) {
  uint16_t writeAddress = page * EEPROM_PAGE_SIZE;

  if (writeAddress < MAX_EEPROM_RANGE) {
    writeToEeprom(writeAddress, &m_eepromBuffer[writeAddress],
                  EEPROM_PAGE_SIZE);
  } else {
    Serial.println("Error: writePageFromBufferToEeprom tried to write memory "
                   "outside MAX_EEPROM_RANGE");
  }
}

bool isPageIntegrityGood(uint16_t page) {
  uint16_t pageStart = page * 64;
  uint16_t pageCrcHighByteAddress = page * EEPROM_PAGE_SIZE + 62;
  uint16_t pageCrcLowByteAddress = pageCrcHighByteAddress + 1;

  crc.reset();
  for (int i = pageStart; i < pageCrcHighByteAddress; i++) {
    crc.add(m_eepromBuffer[i]);
  }

  uint16_t savedCrc = ((uint16_t)m_eepromBuffer[pageCrcHighByteAddress] << 8) |
                      m_eepromBuffer[pageCrcLowByteAddress];
  uint16_t calculatedCrc = crc.getCRC();

  if (savedCrc == calculatedCrc) {
    return true;
  } else {
    return false;
  }
}

void writePageIntegrity(int page) {
  uint16_t pageStart = page * EEPROM_PAGE_SIZE;
  uint16_t pageCrcHighByteAddress = page * EEPROM_PAGE_SIZE + 62;
  uint16_t pageCrcLowByteAddress = pageCrcHighByteAddress + 1;

  crc.reset();
  for (int i = pageStart; i < pageCrcHighByteAddress; i++) {
    crc.add(m_eepromBuffer[i]);
  }

  uint16_t calculatedCrc = crc.getCRC();
  uint8_t calculatedCrcHighByte = (uint8_t)(calculatedCrc >> 8);
  uint8_t calculatedCrcLowByte = (uint8_t)(calculatedCrc & 0xFF);

  m_eepromBuffer[pageCrcHighByteAddress] = calculatedCrcHighByte;
  m_eepromBuffer[pageCrcLowByteAddress] = calculatedCrcLowByte;
}

void wipePage(int page) {
  uint16_t pageStart = page * EEPROM_PAGE_SIZE;
  uint16_t pageEnd = pageStart + EEPROM_PAGE_SIZE;

  for (int i = pageStart; i < pageEnd; i++) {
    m_eepromBuffer[i] = 0;
  }
}

void loadPageFromEepromToEepromBufferAndCheckIntegrity(int page) {
  loadPageFromEepromToBuffer(page);

  if (isPageIntegrityGood(page) == false) {
    st("Error: page ");
    st(page);
    sn(" integrety was bad. Wiping page...");

    wipePage(page);
    writePageIntegrity(page);

    if (isPageIntegrityGood(page) == false) {
      Serial.println(
          "Error: page integrety still bad. This is probably a bug.");
    } else {
      writePageFromBufferToEeprom(page);
    }
  }
}

void clearEepromBuffer() {
  for (int i = 0; i < MAX_EEPROM_RANGE; i++) {
    m_eepromBuffer[i] = '\0';
  }
}
#include <Adafruit_PWMServoDriver.h>
#include <ArduinoOTA.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiServer.h>
#include <Wire.h>

#include "CRC.h"
#include "CRC16.h"

// If you check out this project, this file does not exist.
// You need to create a copy of example_arduino_secrets.h and rename it to
// arduino_secrets.h and fill in your WiFI name and password
#include "arduino_secrets.h"

#define pt client.print
#define pn client.println

const int MAX_PWM_BOARDS = 4;
const int MAX_TOTAL_CHANNELS = MAX_PWM_BOARDS * 16;

const int PWM_REFRESH_RATE = 1042;

const int EEPROM_ADDRESS = 0x50;

// Each page is 64 Bytes long, we use one page per channel + 1, in page 0 we store general settings
const int MAX_EEPROM_RANGE = 64 * (MAX_TOTAL_CHANNELS + 1);
const int PAGE_BUFFER_SIZE = 8192;
const int MAX_CHANNEL_NAME_LENGTH = 20;

// Memory Slots for general settings
const int MEM_SLOT_CHANNELS = 0;
const int MEM_SLOT_FORCE_ALL_ON = 2;
const int MEM_SLOT_FORCE_ALL_OFF = 3;
const int MEM_SLOT_RANDOM = 4;

// Memory Slots for each channel
const int MEM_SLOT_BRIGHTNESS = 30;
const int MEM_SLOT_RANDOM_ON = 32;
const int MEM_SLOT_RANDOM_ON_FREQ = 33;
const int MEM_SLOT_RANDOM_OFF = 34;
const int MEM_SLOT_RANDOM_OFF_FREQ = 35;
const int MEM_SLOT_IS_LINKED = 36;
const int MEM_SLOT_LINKED_CHANNEL = 37;

const int MEM_SLOT_CRC = 62;

char m_pageBuffer[PAGE_BUFFER_SIZE];

WiFiServer server(80);

uint16_t m_numChannels = 0;
uint8_t m_toggleRandom = false;
uint8_t m_toggleForceAllOff = false;
uint8_t m_toggleForceAllOn = false;

char m_channelIdBuffer[4] = "0";
char m_channelNameBuffer[MAX_CHANNEL_NAME_LENGTH + 1] = "";
char m_channelValueBuffer[5] = "0";

char m_channelIdToEditBuffer[4] = "";
uint16_t m_channelIdToEdit = 0;
bool m_renderNextPageWithOptionsVisible = true;
bool m_renderNextPageWithChannelEditVisible = false;

Adafruit_PWMServoDriver m_pwmBoards[MAX_PWM_BOARDS];

// Function to extract value from form data using char arrays
// formData: The form data as a char array
// key: The key as a char array
// value: A char array buffer where the extracted value will be stored
// valueLen: The length of the value buffer
void getValueFromData(const char *formData, const char *key, char *value,
                      int valueLen) {
  const char *startPtr = strstr(formData, key);
  if (startPtr == NULL) {
    Serial.print("Unable to find key: ");
    Serial.println(key);
    return;
  }

  startPtr += strlen(key);  // Move pointer past the key

  const char *endPtr = strchr(startPtr, '&');
  if (endPtr == NULL) {
    endPtr =
      formData + strlen(formData);  // Set endPtr to the end of formData if '&' not found
  }

  int numCharsToCopy = endPtr - startPtr;
  if (numCharsToCopy >= valueLen) {
    numCharsToCopy = valueLen - 1;  // Ensure we don't exceed buffer size
  }

  strncpy(value, startPtr, numCharsToCopy);
  value[numCharsToCopy] = '\0';  // Null-terminate the string

  return;
}

bool isKeyInData(const char *formData, const char *key) {
  const char *startPtr = strstr(formData, key);
  if (startPtr == NULL) {
    return false;
  }

  return true;
}

int getBoardIndexForChannel(int channel) {
  return channel / 16;
}

int getBoardAddressForChannel(int channel) {
  return 0x40 + getBoardIndexForChannel(channel);
}

int getBoardSubAddressForChannel(int channel) {
  return channel % 16;
}

void renderWebPage(WiFiClient client) {
  // Send a standard HTTP response header
  pn("HTTP/1.1 200 OK");
  pn("Content-type:text/html");
  pn();

  // Output the HTML Web Page
  pn("<!DOCTYPE html>");

  pn("<html>");
  pn("<head>");
  pn("<meta charset='UTF-8'>");
  pn("<meta name='viewport' content='width=device-width, "
     "initial-scale=1'>");  // Responsive meta tag
  pn("    <link "
     "href='https://cdn.jsdelivr.net/npm/bootstrap@5.2.3/dist/css/"
     "bootstrap.min.css' rel='stylesheet' "
     "integrity='sha384-"
     "rbsA2VBKQhggwzxH7pPCaAqO46MgnOM80zW1RWuH61DGLwZJEdK2Kadq2F9CUG65' "
     "crossorigin='anonymous'>");  // Bootstrap CSS

  pn("</head>"
     "<body>"
     "<div class='container'>"
     "<br>"
     "<br>"
     "<h1>Bahnhofs Steuerung 2000</h1>"
     "<form action='/' method='POST' accept-charset='UTF-8'>"
     "<br>");

  if (m_renderNextPageWithOptionsVisible == true) {
    pn("<h3>Optionen</h3>");

    // Max value = Max number of boards (62 are max, but -1 because eeprom
    // address so 61) * number of pins => 61 * 16 = 976
    pt("Kanäle: <input type='number' name='numChannels' min='0' "
       "max='");
    pt(MAX_TOTAL_CHANNELS);
    pt("' value='");
    pt(m_numChannels);
    pt("'>");
    pt("<br><br>");

    // Alles Aus Switch
    pt("<div class='form-check form-switch'>");
    pt("<input class='form-check-input' type='checkbox' "
       "name='toggleForceAllOff' "
       "value='1'  id='toggleForceAllOff'");

    if (m_toggleForceAllOff == true) {
      pn(" checked>");
    } else {
      pt(">");
    }

    pt("<label class='form-check-label' for='toggleForceAllOff'>Alles "
       "0%</label>");
    pt("</div>");
    // /Alles Aus Switch

    // Alles An Switch
    pt("<div class='form-check form-switch'>");
    pt("<input class='form-check-input' type='checkbox' "
       "name='toggleForceAllOn' "
       "value='1'  id='toggleForceAllOn'");

    if (m_toggleForceAllOn == true) {
      pn(" checked>");
    } else {
      pt(">");
    }

    pt("<label class='form-check-label' for='toggleForceAllOn'>Alles "
       "100%</label>");
    pt("</div>");
    // /Alles An Switch

    // Zufalls Switch
    pt("<div class='form-check form-switch'>");
    pt("<input class='form-check-input' type='checkbox' name='toggleRandom'  "
       "value='1' role='switch' "
       "id='toggleRandom'");

    if (m_toggleRandom == true) {
      pn(" checked>");
    } else {
      pt(">");
    }

    pt("<label class='form-check-label' for='toggleRandom'>Zufall</label>");
    pt("</div>");
    // /Zufalls Switch

    pn("<br>");

    pt("<input type='hidden'  name='clearEeprom' value='0'>");

    pn("<button class='btn btn-primary' type='submit' "
       "name='updateSettings' value='Absenden'> Senden</button>");
    pn("<br>");
    pn("<br>");
  }

  if (m_renderNextPageWithChannelEditVisible == true) {
    pt("<h3>Kanal ");
    pt(m_channelIdToEdit);

    pn(" Bearbeiten</h3>");

    pt("<input type='hidden'  name='channelId' value='");
    pt(m_channelIdToEdit);
    pn("'>");

    pt("Name: <br> <input type='text' maxlength='20' size='20' "
       "name='channelName' value='");
    pt(m_channelNameBuffer);
    pn("'>");
    pn("<br><br>");

    pt("Helligkeit: <input type='range' min='0' max='4095' "
       "name='channelValue' value='");
    pt(readUint16tForChannelFromEeprom(m_channelIdToEdit, MEM_SLOT_BRIGHTNESS));
    pn("'>");
    pn("<br><br>");

    pt("Zufällig An: <input type='checkbox' name='randomOn' value='1' ");

    if (readBoolForChannelFromEeprom(m_channelIdToEdit, MEM_SLOT_RANDOM_ON)) {
      pt("checked");
    }

    pn(">");
    pn("<br>");
    pn("Häufigkeit An (/h): ");
    pt("<input type='number' name='frequencyOn' min='0' max='255' value='");
    pt(readUint8tForChannelFromEeprom(m_channelIdToEdit,
                                      MEM_SLOT_RANDOM_ON_FREQ));
    pn("'><br><br>");

    pt("Zufällig Aus: <input type='checkbox' name='randomOff' value='1' ");
    if (readBoolForChannelFromEeprom(m_channelIdToEdit, MEM_SLOT_RANDOM_OFF)) {
      pt("checked");
    }

    pn(">");
    pn("<br>");
    pt("Häufigkeit Aus (/h): <input type='number' "
       "name='frequencyOff' min='0' max='255' value='");
    pt(readUint8tForChannelFromEeprom(m_channelIdToEdit,
                                      MEM_SLOT_RANDOM_OFF_FREQ));
    pn("'><br><br>");

    pt("Verlinkt: <input type='checkbox' name='channelLinked' value='1' ");
    if (readBoolForChannelFromEeprom(m_channelIdToEdit, MEM_SLOT_IS_LINKED)) {
      pt("checked");
    }

    pn(">");
    pn("<br>");
    pt("Verlinkter Kanal: <input type='number' "
       "name='linkedChannelId' min='0' max='");
    pt(m_numChannels - 1);
    pt("' value='");
    pt(readUint16tForChannelFromEeprom(m_channelIdToEdit,
                                       MEM_SLOT_LINKED_CHANNEL));
    pn("'><br>");

    pn("<br><br>");
    pn("<input class='btn btn-success' type='submit' name='updateChannel' "
       "value='Speichern'/> &nbsp; <input class='btn btn-warning' "
       "type='submit' name='ignoreChannel' value='Verwerfen'/>");
    pn("<br>");
  }

  pn("<br>");

  pn("<h3>Übersicht der Kanäle</h3>");

  pn("<div>");

  for (int i = 0; i < m_numChannels; i++) {
    readNameForChannelFromEepromToBuffer(i);
    uint16_t brightness =
      readUint16tForChannelFromEeprom(i, MEM_SLOT_BRIGHTNESS);

    bool randomOn = readBoolForChannelFromEeprom(i, MEM_SLOT_RANDOM_ON);
    uint8_t randomOnFreq =
      readUint8tForChannelFromEeprom(i, MEM_SLOT_RANDOM_ON_FREQ);

    bool randomOff = readBoolForChannelFromEeprom(i, MEM_SLOT_RANDOM_OFF);
    uint8_t randomOffFreq =
      readUint8tForChannelFromEeprom(i, MEM_SLOT_RANDOM_OFF_FREQ);

    bool isLinked = readBoolForChannelFromEeprom(i, MEM_SLOT_IS_LINKED);
    uint16_t linkedChannel =
      readUint16tForChannelFromEeprom(i, MEM_SLOT_LINKED_CHANNEL);

    pn("<div class='pl-1 pr-1'>");

    // ROW START
    pn("  <div class='row'>");
    pn("      <div class='col-9'>");

    //  FIRST COL
    pt("<span class='h4'>");
    pt("Kanal ");
    pt(i);
    pt(" </span > ");
    pt("Board ");
    pt(getBoardIndexForChannel(i));
    pt(", Pin ");
    pt(getBoardSubAddressForChannel(i));
    //  / FIRST COL

    pn("      </div>");
    pn("      <div class='col'>");

    // SECOND COL
    pn("      <div class='d-flex justify-content-end'>");
    pt("<button class='btn p-0' type='submit' name='editChannel' value='");
    pt(i);
    pt("'>🖊</button >");
    pn("      </div>");
    // / SECOND COL

    pn("      </div>");
    pn("  </div>");
    // ROW END

    pn("  <div class='row'>");
    pn("    <div class='col'>");
    pn("<span class='h6'>Name</span>");
    pn("    </div>");
    pn("    <div class='col'>");
    pn(m_channelNameBuffer);
    pn("    </div>");
    pn("  </div>");

    pn("  <div class='row'>");
    pn("    <div class='col'>");
    pn("<span class='h6'>Helligkeit</span>");
    pn("    </div>");
    pn("    <div class='col'>");
    pt((int)(((float)brightness / 4095) * 100));
    pn("%");

    pn("    </div>");
    pn("  </div>");

    pn("  <div class='row'>");
    pn("    <div class='col'>");
    pn("      <span class='h6'>Zufällig An</span>");
    pn("    </div>");
    pn("    <div class='col'>");
    if (randomOn) {
      pn("      Ja");
    } else {
      pn("      Nein");
    }
    pn("    </div>");
    pn("  </div>");

    if (randomOn) {
      pn("  <div class='row'>");
      pn("    <div class='col'>");
      pn("      <span class='h6'>Häufigkeit</span>");
      pn("    </div>");
      pn("    <div class='col'>");
      pt(randomOnFreq);
      pn("/h");
      pn("    </div>");
      pn("  </div>");
    }

    pn("  <div class='row'>");
    pn("    <div class='col'>");
    pn("      <span class='h6'>Zufällig Aus</span>");
    pn("    </div>");
    pn("    <div class='col'>");
    if (randomOff) {
      pn("      Ja");
    } else {
      pn("      Nein");
    }
    pn("    </div>");
    pn("  </div>");

    if (randomOff) {
      pn("  <div class='row'>");
      pn("    <div class='col'>");
      pn("      <span class='h6'>Häufigkeit</span>");
      pn("    </div>");
      pn("    <div class='col'>");
      pt(randomOffFreq);
      pn("/h");
      pn("    </div>");
      pn("  </div>");
    }

    pn("  <div class='row'>");
    pn("    <div class='col'>");
    pn("      <span class='h6'>Verlinkt</span>");
    pn("    </div>");
    pn("    <div class='col'>");
    if (isLinked) {
      pn("      Ja");
    } else {
      pn("      Nein");
    }
    pn("    </div>");
    pn("  </div>");

    if (isLinked) {
      pn("  <div class='row'>");
      pn("    <div class='col'>");
      pn("      <span class='h6'>Kanal</span>");
      pn("    </div>");
      pn("    <div class='col'>");
      pn(linkedChannel);
      pn("    </div>");
      pn("  </div>");
    }

    pn("  </div>");
    pn("<hr class='mb-3 mt-3'/>");
  }

  pn("<br>");
  pn("</form>");
  pn("</div>");  // Close container div
  pn("</body></html>");
  pn();
}

void writeUInt16ToEeprom(uint16_t writeAddress, uint16_t value) {
  uint8_t data[2];                // Create a byte array to hold the uint16_t value
  data[0] = (value >> 8) & 0xFF;  // Extract the high byte
  data[1] = value & 0xFF;         // Extract the low byte

  writeToEeprom(writeAddress, data, 2);
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

uint16_t readUInt16FromEeprom(uint16_t readAddress) {
  uint8_t data[2];
  readFromEeprom(readAddress, data, 2);

  uint16_t value = ((uint16_t)data[0] << 8) | data[1];
  return value;
}

void readNameForChannelFromEepromToBuffer(int channel) {
  uint16_t startAddress = (channel + 1) * 64;
  readFromEeprom(startAddress, (uint8_t *)m_channelNameBuffer, 20);
  m_channelNameBuffer[21] = '\0';
}

void writeNameForChannelFromBufferToEeprom(int channel) {
  uint16_t startAddress = (channel + 1) * 64;
  writeToEeprom(startAddress, (uint8_t *)m_channelNameBuffer, 21);
}

void writeUint8tToEeprom(int channel, int memorySlot, uint8_t value) {
  uint16_t startAddress = (channel + 1) * 64 + memorySlot;
  writeToEeprom(startAddress, &value, 1);
}

void writeUint16tForChannelToEeprom(int channel, int memorySlot,
                                    uint16_t channelValue) {
  int startAddress = (channel + 1) * 64 + memorySlot;
  writeUInt16ToEeprom(startAddress, channelValue);
}

uint16_t readUint16tForChannelFromEeprom(int channel, int memorySlot) {
  int startAddress = (channel + 1) * 64 + memorySlot;
  return readUInt16FromEeprom(startAddress);
}

bool readBoolForChannelFromEeprom(int channel, int memorySlot) {
  int startAddress = (channel + 1) * 64 + memorySlot;
  uint8_t result = 0;

  readFromEeprom(startAddress, &result, 1);

  if (result == 0) {
    return false;
  } else {
    return true;
  }
}

uint8_t readUint8tForChannelFromEeprom(int channel, int memorySlot) {
  int startAddress = (channel + 1) * 64 + memorySlot;
  uint8_t result = 0;

  readFromEeprom(startAddress, &result, 1);
  return result;
}

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

void dumpEepromData(int startAddress, int endAddress) {

  char asciiBuffer[9];  // Buffer to store ASCII characters (8 characters + null
                        // terminator)
  int bufferIndex = 0;

  for (int i = startAddress; i <= endAddress; i++) {
    // Print the address at the start of each line
    if ((i - startAddress) % 8 == 0) {
      if (i != startAddress) {
        asciiBuffer[bufferIndex] = '\0';  // Null terminate the ASCII buffer
        Serial.print("  ");               // End the previous line
        Serial.println(asciiBuffer);
        bufferIndex = 0;
      }
      Serial.print(i, HEX);
      Serial.print(": ");
    }

    uint8_t byteValue = 0;
    readFromEeprom(i, &byteValue, 1);

    // Print the byte value in HEX
    if (byteValue < 0x10) {
      Serial.print('0');  // Print leading zero for single digit hex values
    }
    Serial.print(byteValue, HEX);
    Serial.print(" ");

    // Store the corresponding ASCII character or a placeholder
    asciiBuffer[bufferIndex++] =
      (byteValue >= 32 && byteValue <= 126) ? byteValue : '.';

    // Group the bytes in sets of four, separated by two spaces
    if ((i - startAddress) % 4 == 3) {
      Serial.print("  ");
    }
  }

  // Handle any remaining ASCII characters at the end
  asciiBuffer[bufferIndex] = '\0';  // Null terminate the buffer
  if (bufferIndex > 0) {
    Serial.print("  ");           // Two spaces before ASCII dump
    Serial.println(asciiBuffer);  // Print the ASCII characters
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

void applyValue(int channel, uint16_t brightness) {
  int boardIndex = getBoardIndexForChannel(channel);
  int subAddress = getBoardSubAddressForChannel(channel);

  if (m_toggleForceAllOff == true) {
    m_pwmBoards[boardIndex].setPWM(subAddress, 0, 0);
    return;
  }

  if (m_toggleForceAllOn) {
    m_pwmBoards[boardIndex].setPWM(subAddress, 0, 4095);
    return;
  }

  if (m_toggleRandom == true) {
    m_pwmBoards[boardIndex].setPWM(subAddress, 0, random(0, 4095));
    return;
  }

  m_pwmBoards[boardIndex].setPWM(subAddress, 0, brightness);
}

void applyValues() {
  for (int i = 0; i < m_numChannels; i++) {

    uint16_t brightness =
      readUint16tForChannelFromEeprom(i, MEM_SLOT_BRIGHTNESS);

    applyValue(i, brightness);
  }
}

void clearPageBuffer() {
  for (int i = 0; i < PAGE_BUFFER_SIZE; i++) {
    m_pageBuffer[i] = '\0';
  }
}

void checkPageBufferForPostData() {
  if (strstr(m_pageBuffer, "POST") != NULL) {
    if (isKeyInData(m_pageBuffer, "editChannel")) {
      Serial.println("PREPARE CHANNEL FORM");

      getValueFromData(m_pageBuffer, "editChannel=", m_channelIdToEditBuffer,
                       4);

      m_renderNextPageWithOptionsVisible = false;
      m_renderNextPageWithChannelEditVisible = true;

      m_channelIdToEdit = atoi(m_channelIdToEditBuffer);
      readNameForChannelFromEepromToBuffer(m_channelIdToEdit);
    }

    if (isKeyInData(m_pageBuffer, "updateSettings")) {
      char clearEepromBuffer[10] = "";
      getValueFromData(m_pageBuffer, "clearEeprom=", clearEepromBuffer, 10);
      Serial.print("clearEepromBuffer: ");
      Serial.println(clearEepromBuffer);
      if (strcmp(clearEepromBuffer, "reset2024") == 0) {
        Serial.println("Clearing Eeprom!!!");
        clearEeprom();
        return;
      }

      m_renderNextPageWithOptionsVisible = true;
      m_renderNextPageWithChannelEditVisible = false;

      // Initialize the toggles with 0 since they might not be present in the
      // post data if unchecked
      char toggleForceAllOffBuffer[2] = "0";
      char toggleForceAllOnBuffer[2] = "0";
      char toggleRandomBuffer[2] = "0";
      char numChannelsBuffer[4] = "0";

      // Find each value
      getValueFromData(m_pageBuffer,
                       "toggleForceAllOn=", toggleForceAllOnBuffer, 2);
      getValueFromData(m_pageBuffer,
                       "toggleForceAllOff=", toggleForceAllOffBuffer, 2);
      getValueFromData(m_pageBuffer, "toggleRandom=", toggleRandomBuffer, 2);
      getValueFromData(m_pageBuffer, "numChannels=", numChannelsBuffer, 4);

      m_toggleForceAllOn = atoi(toggleForceAllOnBuffer);
      m_toggleForceAllOff = atoi(toggleForceAllOffBuffer);
      m_toggleRandom = atoi(toggleRandomBuffer);
      m_numChannels = atoi(numChannelsBuffer);

      writeUInt16ToEeprom(MEM_SLOT_CHANNELS, m_numChannels);
      writeToEeprom(MEM_SLOT_FORCE_ALL_ON, &m_toggleForceAllOn, 1);
      writeToEeprom(MEM_SLOT_FORCE_ALL_OFF, &m_toggleForceAllOff, 1);
      writeToEeprom(MEM_SLOT_RANDOM, &m_toggleRandom, 1);
    }

    if (isKeyInData(m_pageBuffer, "updateChannel")) {
      Serial.println("UPDATE CHANNEL");

      m_renderNextPageWithOptionsVisible = true;
      m_renderNextPageWithChannelEditVisible = false;

      getValueFromData(m_pageBuffer, "channelId=", m_channelIdBuffer, 5);
      getValueFromData(m_pageBuffer, "channelName=", m_channelNameBuffer, 21);
      getValueFromData(m_pageBuffer, "channelValue=", m_channelValueBuffer, 5);

      char randomOnBuffer[2] = "0";
      char randomOnFreqBuffer[4] = "0";

      char randomOffBuffer[2] = "0";
      char randomOffFreqBuffer[4] = "0";

      char isLinkedBuffer[2] = "0";
      char linkedChannelIdBuffer[4] = "0";

      getValueFromData(m_pageBuffer, "randomOn=", randomOnBuffer, 2);
      getValueFromData(m_pageBuffer, "frequencyOn=", randomOnFreqBuffer, 4);
      getValueFromData(m_pageBuffer, "randomOff=", randomOffBuffer, 2);
      getValueFromData(m_pageBuffer, "frequencyOff=", randomOffFreqBuffer, 4);
      getValueFromData(m_pageBuffer, "channelLinked=", isLinkedBuffer, 2);
      getValueFromData(m_pageBuffer, "linkedChannelId=", linkedChannelIdBuffer,
                       2);

      uint8_t randomOn = atoi(randomOnBuffer);
      uint8_t randomOnFreq = atoi(randomOnFreqBuffer);

      uint8_t randomOff = atoi(randomOffBuffer);
      uint8_t randomOffFreq = atoi(randomOffFreqBuffer);

      uint8_t isLinked = atoi(isLinkedBuffer);
      uint16_t linkedChannelId = atoi(linkedChannelIdBuffer);

      uint16_t channelIdAsNumber = atoi(m_channelIdBuffer);
      Serial.print("Channel ");
      Serial.print(channelIdAsNumber);

      uint16_t startAddress = 64 + channelIdAsNumber * 64;
      Serial.print(" got startAddress ");
      Serial.println(startAddress);

      uint16_t channelValue = atoi(m_channelValueBuffer);

      writeNameForChannelFromBufferToEeprom(channelIdAsNumber);
      writeUint16tForChannelToEeprom(channelIdAsNumber, MEM_SLOT_BRIGHTNESS,
                                     channelValue);

      writeUint8tToEeprom(channelIdAsNumber, MEM_SLOT_RANDOM_ON, randomOn);
      if (randomOn == 1) {
        writeUint8tToEeprom(channelIdAsNumber, MEM_SLOT_RANDOM_ON_FREQ, randomOnFreq);
      }

      writeUint8tToEeprom(channelIdAsNumber, MEM_SLOT_RANDOM_OFF, randomOff);
      if (randomOff == 1) {
        writeUint8tToEeprom(channelIdAsNumber, MEM_SLOT_RANDOM_OFF_FREQ, randomOffFreq);
      }

      writeUint8tToEeprom(channelIdAsNumber, MEM_SLOT_IS_LINKED, isLinked);
      if (isLinked == 1) {
        writeUint16tForChannelToEeprom(channelIdAsNumber, MEM_SLOT_LINKED_CHANNEL, linkedChannelId);
      }

      applyValue(channelIdAsNumber, channelValue);
    }

    // dumpEepromData(0, (m_numChannels + 1) * 64 - 1);

    // After each post request, we apply all the values
    applyValues();
  }
}

void loadOptionsToMemberVariables() {
  m_numChannels = readUInt16FromEeprom(MEM_SLOT_CHANNELS);
  readFromEeprom(MEM_SLOT_FORCE_ALL_OFF, &m_toggleForceAllOff, 1);
  readFromEeprom(MEM_SLOT_FORCE_ALL_ON, &m_toggleForceAllOn, 1);
  readFromEeprom(MEM_SLOT_RANDOM, &m_toggleRandom, 1);
}

uint8_t eepromPageBuffer[64];
CRC16 crc;

void checkPageIntegrity(uint8_t page) {
  readFromEeprom(page * 64, eepromPageBuffer, 64);

  crc.reset();
  for (int i = 0; i < 62; i++) {
    crc.add(eepromPageBuffer[i]);
  }

  uint16_t savedCrc = ((uint16_t)eepromPageBuffer[62] << 8) | eepromPageBuffer[63];
  uint16_t caclulatedCrc = crc.getCRC();

  Serial.print("savedCrc: ");
  Serial.print(savedCrc);
  Serial.print(" caclulatedCrc: ");
  Serial.print(caclulatedCrc);

  if (savedCrc == caclulatedCrc) {
    Serial.println(" OK");
  } else {
    Serial.println(" NOT OK");
  }
}

void writePageIntegrity(uint8_t page) {
  readFromEeprom(page * 64, eepromPageBuffer, 64);

  crc.reset();
  for (int i = 0; i < 62; i++) {
    crc.add(eepromPageBuffer[i]);
  }

  uint16_t caclulatedCrc = crc.getCRC();
  uint16_t address = page * 64 + MEM_SLOT_CRC;

  Serial.print("Writing caclulatedCrc: ");
  Serial.print(caclulatedCrc);
  Serial.print(" to address: ");
  Serial.println(address);
  writeUInt16ToEeprom(address, caclulatedCrc);
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting");
  Wire.begin();

  checkPageIntegrity(0);
  writePageIntegrity(0);

  // clearEeprom();

  clearPageBuffer();
  // Initializte pwm boards
  for (int i = 0; i < MAX_PWM_BOARDS; i++) {
    int pwmAddress = 0x40 + i;

    m_pwmBoards[i] = Adafruit_PWMServoDriver(pwmAddress);
    m_pwmBoards[i].begin();
    m_pwmBoards[i].setOscillatorFrequency(27000000);
    m_pwmBoards[i].setPWMFreq(PWM_REFRESH_RATE);

    Serial.print("Board added: ");
    Serial.println(pwmAddress);
  }

  /*   ArduinoOTA.onStart([]() {
    Serial.println("Start OTA");
  }); */

  WiFi.begin(SECRET_SSID, SECRET_PASS);
  Serial.println("Attempting to connect to WiFi network...");
  delay(500);
  WiFi.begin(SECRET_SSID, SECRET_PASS);

  int tries = 0;
  // Attempt to connect to WiFi network
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Waiting for WiFi network...");
    //

    if (tries > 10) {
      Serial.println("Re-attempting to connect to WiFi network...");
      WiFi.begin(SECRET_SSID, SECRET_PASS);
      tries = 0;
    }

    delay(1000);
    tries++;
  }

  Serial.println("Connected to WiFi");

  // Print the IP address
  Serial.print("Server IP Address: ");
  Serial.println(WiFi.localIP());

  // Start the server
  server.begin();

  /*   // start the WiFi OTA library with internal (flash) based storage
  ArduinoOTA.begin(WiFi.localIP(), "Arduino_MKR_WiFi_1010", SECRET_OTA,
  InternalStorage); */

  Serial.println("Server started");
  loadOptionsToMemberVariables();
  dumpEepromData(0, MAX_EEPROM_RANGE);
  applyValues();
}



void loop() {
  /*   // check for WiFi OTA updates
  ArduinoOTA.poll(); */

  WiFiClient client = server.available();  // Listen for incoming clients

  if (client) {
    boolean currentLineIsBlank = true;

    clearPageBuffer();
    int pageIndex = 0;

    if (client.connected()) {
      while (client.available()) {
        char c = client.read();
        // Serial.write(c);

        m_pageBuffer[pageIndex] = c;
        pageIndex++;

        if (pageIndex >= PAGE_BUFFER_SIZE) {
          Serial.println("WARNING: PAGE EXCEEDED BUFFER SIZE! DANGER!!!");
        }
      }

      checkPageBufferForPostData();
      renderWebPage(client);
      client.stop();
    }
  }
}
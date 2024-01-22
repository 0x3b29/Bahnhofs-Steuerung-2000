#include <Adafruit_PWMServoDriver.h>
#include <ArduinoOTA.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiServer.h>

#include "eeprom.h"
#include "main.h"

// If you check out this project, this file does not exist.
// You need to create a copy of example_arduino_secrets.h and rename it to
// arduino_secrets.h and fill in your WiFI name and password
#include "arduino_secrets.h"

char m_pageBuffer[PAGE_BUFFER_SIZE];

WiFiServer server(80);

uint16_t m_numChannels = 0;
uint8_t m_toggleRandom = false;
uint8_t m_toggleForceAllOff = false;
uint8_t m_toggleForceAllOn = false;

char m_channelIdBuffer[4] = "0";

char m_channelIdToEditBuffer[4] = "";
uint16_t m_channelIdToEdit = 0;
bool m_renderNextPageWithOptionsVisible = true;
bool m_renderNextPageWithChannelEditVisible = false;

#define RANDOM_INTERVAL 200
#define RANDOM_EVENT_INTERVAL 1000

long m_lastRandom = 0;
long m_lastRandomEvent = 0;

Adafruit_PWMServoDriver m_pwmBoards[PWM_BOARDS];

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

  startPtr += strlen(key); // Move pointer past the key

  const char *endPtr = strchr(startPtr, '&');
  if (endPtr == NULL) {
    endPtr =
        formData +
        strlen(formData); // Set endPtr to the end of formData if '&' not found
  }

  int numCharsToCopy = endPtr - startPtr;
  if (numCharsToCopy >= valueLen) {
    numCharsToCopy = valueLen - 1; // Ensure we don't exceed buffer size
  }

  strncpy(value, startPtr, numCharsToCopy);
  value[numCharsToCopy] = '\0'; // Null-terminate the string

  return;
}

bool isKeyInData(const char *formData, const char *key) {
  const char *startPtr = strstr(formData, key);
  if (startPtr == NULL) {
    return false;
  }

  return true;
}

int getBoardIndexForChannel(int channel) { return channel / 16; }

int getBoardAddressForChannel(int channel) {
  return 0x40 + getBoardIndexForChannel(channel);
}

int getBoardSubAddressForChannel(int channel) { return channel % 16; }

void renderWebPage(WiFiClient client) {
  // Send a standard HTTP response header
  pn("HTTP/1.1 200 OK");
  pn("Content-type:text/html");
  pn();

  // Output the HTML Web Page
  pn("<!DOCTYPE html>");

  pn("<html>"
     "<head>"
     "<meta charset='UTF-8'>"
     "<meta name='viewport' content='width=device-width, "
     "initial-scale=1'>"
     "<link "
     "href='https://cdn.jsdelivr.net/npm/bootstrap@5.2.3/dist/css/"
     "bootstrap.min.css' rel='stylesheet' "
     "integrity='sha384-"
     "rbsA2VBKQhggwzxH7pPCaAqO46MgnOM80zW1RWuH61DGLwZJEdK2Kadq2F9CUG65' "
     "crossorigin='anonymous'/>"
     "<link href='https://fonts.googleapis.com/css2?family=Grape+Nuts&display=swap' rel='stylesheet'>"
     "</head>"
     "<body>"
     "<div class='container'>"
     "<div class='row justify-content-md-center'>"
     "<div class='col col-md-8 col-lg-5'>"
     "<div class='h1 mt-4 mb-3' style=\"font-family: 'Grape Nuts', bold; font-size: xx-large;\">Bahnhofs Steuerung 2000</div>"
     "<form id='myForm' action='/' method='POST' accept-charset='UTF-8'>");

  if (m_renderNextPageWithOptionsVisible == true) {
    pn("<h3>Optionen</h3>");

    // Max value = Max number of boards (62 are max, but -1 because eeprom
    // address so 61) * number of pins => 61 * 16 = 976
    pt("<div class='row'>"
       "<div class='col-3 d-flex align-items-center'> Kanäle: </div> <div "
       "class='col-5'> <input type='number' class='form-control w-100' "
       "name='numChannels' min='0' "
       "max='");
    pt(MAX_TOTAL_CHANNELS);
    pt("' value='");
    pt(m_numChannels);
    pt("'> </div>");
    pn("<div class='col-4 d-flex justify-content-end'><button class='btn "
       "btn-primary' type='submit' "
       "name='updateSettings' value='Absenden'>Senden</button> </div></div>"
       "<br>");

    // Alles Aus Switch
    pt("<div class='form-check form-switch'>");
    pt("<input class='form-check-input' type='checkbox' "
       "name='toggleForceAllOff' "
       "value='1'  id='toggleForceAllOff' onchange='sendCheckbox(this)'");

    if (m_toggleForceAllOff == true) {
      pn(" checked>");
    } else {
      pt(">");
    }

    pt("<label class='form-check-label' for='toggleForceAllOff'>Alles "
       "dauerhaft "
       "0%</label>");
    pt("</div>");
    // /Alles Aus Switch

    // Alles An Switch
    pt("<div class='form-check form-switch'>");
    pt("<input class='form-check-input' type='checkbox' "
       "name='toggleForceAllOn' "
       "value='1' id='toggleForceAllOn' onchange='sendCheckbox(this)'");

    if (m_toggleForceAllOn == true) {
      pn(" checked>");
    } else {
      pt(">");
    }

    pt("<label class='form-check-label' for='toggleForceAllOn'>Alles dauerhaft "
       "100%</label>"
       "</div>");
    // /Alles An Switch

    // Zufalls Switch
    pt("<div class='form-check form-switch'>"
       "<input class='form-check-input' type='checkbox' name='toggleRandom'  "
       "value='1' role='switch' "
       "id='toggleRandom' onchange='sendCheckbox(this)'");

    if (m_toggleRandom == true) {
      pn(" checked>");
    } else {
      pt(">");
    }

    pt("<label class='form-check-label' for='toggleRandom'>Zufällig blinken</label>"
       "</div>"
       // /Zufalls Switch

       "<input type='hidden'  name='clearEeprom' value='0'>");
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
    pt(readUint16tForChannelFromEepromBuffer(m_channelIdToEdit,
                                             MEM_SLOT_BRIGHTNESS));
    pn("'>");
    pn("<br><br>");

    pt("<div class='form-check form-switch'>"
       "<input class='form-check-input' type='checkbox' "
       "name='initialState' value='1' role='switch' id='initialState' "
       "value='1'");

    if (readBoolForChannelFromEepromBuffer(m_channelIdToEdit,
                                           MEM_SLOT_INITIAL_STATE)) {
      pn(" checked>");
    } else {
      pt(">");
    }

    pt("<label class='form-check-label' for='toggleRandom'>Startzustand</label>"
       "</div>");

    pt("Zufällig An: <input type='checkbox' name='randomOn' value='1' ");

    if (readBoolForChannelFromEepromBuffer(m_channelIdToEdit,
                                           MEM_SLOT_RANDOM_ON)) {
      pt("checked");
    }

    pn(">");
    pn("<br>");
    pn("Häufigkeit An (/h): ");
    pt("<input type='number' name='frequencyOn' min='0' max='255' value='");
    pt(readUint8tForChannelFromEepromBuffer(m_channelIdToEdit,
                                            MEM_SLOT_RANDOM_ON_FREQ));
    pn("'><br><br>");

    pt("Zufällig Aus: <input type='checkbox' name='randomOff' value='1' ");
    if (readBoolForChannelFromEepromBuffer(m_channelIdToEdit,
                                           MEM_SLOT_RANDOM_OFF)) {
      pt("checked");
    }

    pn(">");
    pn("<br>");
    pt("Häufigkeit Aus (/h): <input type='number' "
       "name='frequencyOff' min='0' max='255' value='");
    pt(readUint8tForChannelFromEepromBuffer(m_channelIdToEdit,
                                            MEM_SLOT_RANDOM_OFF_FREQ));
    pn("'><br><br>");

    pt("Verlinkt: <input type='checkbox' name='channelLinked' value='1' ");
    if (readBoolForChannelFromEepromBuffer(m_channelIdToEdit,
                                           MEM_SLOT_IS_LINKED)) {
      pt("checked");
    }

    pn(">");
    pn("<br>");
    pt("Steuert Kanal: <input type='number' "
       "name='linkedChannelId' min='0' max='");
    pt(m_numChannels - 1);
    pt("' value='");
    pt(readUint16tForChannelFromEepromBuffer(m_channelIdToEdit,
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
    readChannelNameFromEepromBufferToChannelNameBuffer(i);
    uint16_t brightness =
        readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_BRIGHTNESS);

    bool initialState =
        readBoolForChannelFromEepromBuffer(i, MEM_SLOT_INITIAL_STATE);

    bool randomOn = readBoolForChannelFromEepromBuffer(i, MEM_SLOT_RANDOM_ON);
    uint8_t randomOnFreq =
        readUint8tForChannelFromEepromBuffer(i, MEM_SLOT_RANDOM_ON_FREQ);

    bool randomOff = readBoolForChannelFromEepromBuffer(i, MEM_SLOT_RANDOM_OFF);
    uint8_t randomOffFreq =
        readUint8tForChannelFromEepromBuffer(i, MEM_SLOT_RANDOM_OFF_FREQ);

    bool isLinked = readBoolForChannelFromEepromBuffer(i, MEM_SLOT_IS_LINKED);
    uint16_t linkedChannel =
        readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_LINKED_CHANNEL);

    pn("<div class='pl-1 pr-1'>"
       // ROW START
       "  <div class='row'>"
       "      <div class='col-9'>"

       //  FIRST COL
       "<span class='h4'>"
       "Kanal ");
    pt(i);
    pt(" </span > "
       "Board ");
    pt(getBoardIndexForChannel(i));
    pt(", Pin ");
    pt(getBoardSubAddressForChannel(i));
    //  / FIRST COL

    pt("      </div>"
       "      <div class='col-3'>"

       // SECOND COL
       "      <div class='d-flex justify-content-end'>"
       "<button class='btn text-warning'  onclick=\"sendValue('turnChannelOn', "
       "'");
    pt(i);
    pt("')\" >⛭</button >"
       "<button class='btn'  onclick=\"sendValue('turnChannelOff', '");
    pt(i);
    pt("')\" >⛭</button >"

       "<button class='btn' type='submit' name='editChannel' value='");
    pt(i);
    pt("'>🖊</button >"
       "      </div>"
       // / SECOND COL

       "      </div>"
       "  </div>"
       // ROW END

       "  <div class='row'>"
       "    <div class='col'>"
       "<span class='h6'>Name</span>"
       "    </div>"
       "    <div class='col font-weight-bold'> <b>");
    pt(m_channelNameBuffer);
    pt("    </b></div>"
       "  </div>"

       "  <div class='row'>"
       "    <div class='col'>"
       "<span class='h6'>Startzustand</span>"
       "    </div>"
       "    <div class='col'>");

    if (initialState) {
      pt("An");
    } else {
      pt("Aus");
    }

    pt("    </div>"
       "  </div>"

       "  <div class='row'>"
       "    <div class='col'>"
       "<span class='h6'>Helligkeit</span>"
       "    </div>"
       "    <div class='col'>");
    pt((int)(((float)brightness / 4095) * 100));
    pt("%"

       "    </div>"
       "  </div>"

       "  <div class='row'>"
       "    <div class='col'>"
       "      <span class='h6'>Zufällig An</span>"
       "    </div>"
       "    <div class='col'>");
    if (randomOn) {
      pn("      Ja");
    } else {
      pn("      Nein");
    }
    pn("    </div>"
       "  </div>");

    if (randomOn) {
      pn("  <div class='row'>"
         "    <div class='col'>"
         "      <span class='h6'>Häufigkeit</span>"
         "    </div>"
         "    <div class='col'>");
      pt(randomOnFreq);
      pn("/h"
         "    </div>"
         "  </div>");
    }

    pn("  <div class='row'>"
       "    <div class='col'>"
       "      <span class='h6'>Zufällig Aus</span>"
       "    </div>"
       "    <div class='col'>");
    if (randomOff) {
      pn("      Ja");
    } else {
      pn("      Nein");
    }
    pn("    </div>"
       "  </div>");

    if (randomOff) {
      pn("  <div class='row'>"
         "    <div class='col'>"
         "      <span class='h6'>Häufigkeit</span>"
         "    </div>"
         "    <div class='col'>");
      pt(randomOffFreq);
      pn("/h"
         "    </div>"
         "  </div>");
    }

    pn("  <div class='row'>"
       "    <div class='col'>"
       "      <span class='h6'>Verlinkt</span>"
       "    </div>"
       "    <div class='col'>");
    if (isLinked) {
      pn("      Ja");
    } else {
      pn("      Nein");
    }
    pn("    </div>"
       "  </div>");

    if (isLinked) {
      pn("  <div class='row'>"
         "    <div class='col'>"
         "      <span class='h6'>Steuert Kanal</span>"
         "    </div>"
         "    <div class='col'>");
      pn(linkedChannel);
      pn("    </div>"
         "  </div>");
    }

    pn("  </div>"
       "<hr class='mb-3 mt-3'/>");
  }

  pn("<br>"
     "</form>"
     "</div></div></div>"
     "<script>"
     "function sendValue(buttonName, buttonValue) {"
     "    event.preventDefault();"
     "    var dataString = encodeURIComponent(buttonName) + '=' + "
     "encodeURIComponent(buttonValue);"
     "    fetch('/', {"
     "       method: 'POST',"
     "       headers: {"
     "          'Content-Type': 'application/x-www-form-urlencoded'"
     "       },"
     "       body: dataString"
     "   });"
     "}"
     "function sendCheckbox(checkbox) {"
     "var dataString = encodeURIComponent(checkbox.name) + '=' + "
     "encodeURIComponent(checkbox.checked ? 1 : 0);"
     "fetch('/', {"
     "    method: 'POST',"
     "    headers: {"
     "        'Content-Type': 'application/x-www-form-urlencoded'"
     "    },"
     "    body: dataString"
     "})}"
     "</script>"
     "</body></html>");
}

void replyToClient(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/plain");
  client.println();
  client.println("Settings updated successfully.");
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

  st("applyValue ");
  st(brightness);
  st(" to boardIndex ");
  st(boardIndex);
  st(" and subAddress ");
  sn(subAddress);

  m_pwmBoards[boardIndex].setPWM(subAddress, 0, brightness);
}

void applyValues() {
  for (int i = 0; i < m_numChannels; i++) {

    bool initialState =
        readBoolForChannelFromEepromBuffer(i, MEM_SLOT_INITIAL_STATE);

    uint16_t brightness = 0;

    if (initialState == true) {
      brightness =
          readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_BRIGHTNESS);
    }

    applyValue(i, brightness);
  }
}

void clearPageBuffer() {
  for (int i = 0; i < PAGE_BUFFER_SIZE; i++) {
    m_pageBuffer[i] = '\0';
  }
}

void urlDecode(const char *urlEncoded, char *decoded, int maxLen) {
  int len = strlen(urlEncoded);
  int decodedIndex = 0;

  for (int i = 0; i < len && decodedIndex < maxLen - 1; ++i) {
    if (urlEncoded[i] == '+') {
      decoded[decodedIndex++] = ' ';
    } else if (urlEncoded[i] == '%' && i + 2 < len) {
      // Get the next two characters after '%'
      char hex[3];
      hex[0] = urlEncoded[++i];
      hex[1] = urlEncoded[++i];
      hex[2] = '\0';

      // Convert it into a character
      decoded[decodedIndex++] = (char)strtol(hex, NULL, 16);
    } else {
      decoded[decodedIndex++] = urlEncoded[i];
    }
  }

  // Null-terminate the decoded string
  decoded[decodedIndex] = '\0';
}

bool processRequestAndReturnRerenderNeed() {
  bool shouldRerender = false;

  if (strstr(m_pageBuffer, "POST") != NULL) {

    if (isKeyInData(m_pageBuffer, "ignoreChannel")) {
      // User clicked on "Verwerfen" button
      m_renderNextPageWithOptionsVisible = true;
      m_renderNextPageWithChannelEditVisible = false;

      shouldRerender = true;
    }

    if (isKeyInData(m_pageBuffer, "turnChannelOff")) {
      char turnChannelOffIdBuffer[4];
      uint16_t turnChannelOffId;
      getValueFromData(m_pageBuffer, "turnChannelOff=", turnChannelOffIdBuffer,
                       4);
      turnChannelOffId = atoi(turnChannelOffIdBuffer);

      st("Turning channel ");
      st(turnChannelOffId);
      sn(" off");
      applyValue(turnChannelOffId, 0);
    }

    if (isKeyInData(m_pageBuffer, "turnChannelOn")) {
      char turnChannelOnIdBuffer[4];
      uint16_t turnChannelOnId;
      getValueFromData(m_pageBuffer, "turnChannelOn=", turnChannelOnIdBuffer,
                       4);
      turnChannelOnId = atoi(turnChannelOnIdBuffer);

      uint16_t turnOnBrightness = readUint16tForChannelFromEepromBuffer(
          turnChannelOnId, MEM_SLOT_BRIGHTNESS);

      st("Turning channel ");
      st(turnChannelOnId);
      st(" on to ");
      sn(turnOnBrightness);
      applyValue(turnChannelOnId, turnOnBrightness);
    }

    if (isKeyInData(m_pageBuffer, "editChannel")) {
      Serial.println("PREPARE CHANNEL FORM");

      getValueFromData(m_pageBuffer, "editChannel=", m_channelIdToEditBuffer,
                       4);

      m_renderNextPageWithOptionsVisible = false;
      m_renderNextPageWithChannelEditVisible = true;

      m_channelIdToEdit = atoi(m_channelIdToEditBuffer);
      readChannelNameFromEepromBufferToChannelNameBuffer(m_channelIdToEdit);

      shouldRerender = true;
    }

    if (isKeyInData(m_pageBuffer, "toggleForceAllOff")) {
      char toggleForceAllOffBuffer[2] = "0";
      getValueFromData(m_pageBuffer,
                       "toggleForceAllOff=", toggleForceAllOffBuffer, 2);
      m_toggleForceAllOff = atoi(toggleForceAllOffBuffer);
      writeToEepromBuffer(MEM_SLOT_FORCE_ALL_OFF, &m_toggleForceAllOff, 1);

      writePageIntegrity(0);
      writePageFromBufferToEeprom(0);
      applyValues();
    }

    if (isKeyInData(m_pageBuffer, "toggleForceAllOn")) {
      char toggleForceAllOnBuffer[2] = "0";
      getValueFromData(m_pageBuffer,
                       "toggleForceAllOn=", toggleForceAllOnBuffer, 2);
      m_toggleForceAllOn = atoi(toggleForceAllOnBuffer);
      writeToEepromBuffer(MEM_SLOT_FORCE_ALL_ON, &m_toggleForceAllOn, 1);

      writePageIntegrity(0);
      writePageFromBufferToEeprom(0);
      applyValues();
    }

    if (isKeyInData(m_pageBuffer, "toggleRandom")) {
      char toggleRandomBuffer[2] = "0";
      getValueFromData(m_pageBuffer, "toggleRandom=", toggleRandomBuffer, 2);
      m_toggleRandom = atoi(toggleRandomBuffer);
      writeToEepromBuffer(MEM_SLOT_RANDOM, &m_toggleRandom, 1);

      writePageIntegrity(0);
      writePageFromBufferToEeprom(0);
      applyValues();
    }

    if (isKeyInData(m_pageBuffer, "updateSettings")) {
      char clearEepromBuffer[10] = "";
      getValueFromData(m_pageBuffer, "clearEeprom=", clearEepromBuffer, 10);

      // TODO: move reset2024 to secrets file
      if (strcmp(clearEepromBuffer, "reset2024") == 0) {
        Serial.println("Clearing Eeprom!!!");
        clearEeprom();
        return true;
      }

      uint16_t oldNumChannels = m_numChannels;

      m_renderNextPageWithOptionsVisible = true;
      m_renderNextPageWithChannelEditVisible = false;

      char numChannelsBuffer[4] = "0";
      getValueFromData(m_pageBuffer, "numChannels=", numChannelsBuffer, 4);
      m_numChannels = atoi(numChannelsBuffer);

      writeUInt16ToEepromBuffer(MEM_SLOT_CHANNELS, m_numChannels);

      writePageIntegrity(0);
      writePageFromBufferToEeprom(0);

      // If we have new channels, we load them, check them and wipe them if
      // nessesary
      if (oldNumChannels < m_numChannels) {
        for (int i = oldNumChannels; i < m_numChannels; i++) {
          Serial.print("Checking page for channel: ");
          Serial.println(i);
          loadPageAndCheckIntegrity(i + 1);
        }
      }

      applyValues();

      shouldRerender = true;
    }

    if (isKeyInData(m_pageBuffer, "updateChannel")) {
      Serial.println("UPDATE CHANNEL");

      m_renderNextPageWithOptionsVisible = true;
      m_renderNextPageWithChannelEditVisible = false;

      getValueFromData(m_pageBuffer, "channelId=", m_channelIdBuffer, 5);
      uint16_t channelIdAsNumber = atoi(m_channelIdBuffer);

      char urlEncodedNameBuffer[21];
      getValueFromData(m_pageBuffer, "channelName=", urlEncodedNameBuffer, 21);
      urlDecode(urlEncodedNameBuffer, m_channelNameBuffer, 20);

      char channelValueBuffer[5] = "0";
      getValueFromData(m_pageBuffer, "channelValue=", channelValueBuffer, 5);

      char initialStateBuffer[2] = "0";
      getValueFromData(m_pageBuffer, "initialState=", initialStateBuffer, 2);
      uint8_t initialState = atoi(initialStateBuffer);
      writeUint8tToEepromBuffer(channelIdAsNumber, MEM_SLOT_INITIAL_STATE,
                                initialState);

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
                       4);

      uint8_t randomOn = atoi(randomOnBuffer);
      uint8_t randomOnFreq = atoi(randomOnFreqBuffer);

      uint8_t randomOff = atoi(randomOffBuffer);
      uint8_t randomOffFreq = atoi(randomOffFreqBuffer);

      uint8_t isLinked = atoi(isLinkedBuffer);
      uint16_t linkedChannelId = atoi(linkedChannelIdBuffer);

      Serial.print("Channel ");
      Serial.print(channelIdAsNumber);

      uint16_t startAddress = 64 + channelIdAsNumber * 64;
      Serial.print(" got startAddress ");
      Serial.println(startAddress);

      uint16_t channelValue = atoi(channelValueBuffer);

      writeChannelNameFromChannelNameBufferToEepromBuffer(channelIdAsNumber);
      writeUint16tForChannelToEepromBuffer(channelIdAsNumber,
                                           MEM_SLOT_BRIGHTNESS, channelValue);

      writeUint8tToEepromBuffer(channelIdAsNumber, MEM_SLOT_RANDOM_ON,
                                randomOn);
      if (randomOn == 1) {
        writeUint8tToEepromBuffer(channelIdAsNumber, MEM_SLOT_RANDOM_ON_FREQ,
                                  randomOnFreq);
      }

      writeUint8tToEepromBuffer(channelIdAsNumber, MEM_SLOT_RANDOM_OFF,
                                randomOff);
      if (randomOff == 1) {
        writeUint8tToEepromBuffer(channelIdAsNumber, MEM_SLOT_RANDOM_OFF_FREQ,
                                  randomOffFreq);
      }

      writeUint8tToEepromBuffer(channelIdAsNumber, MEM_SLOT_IS_LINKED,
                                isLinked);
      if (isLinked == 1) {
        writeUint16tForChannelToEepromBuffer(
            channelIdAsNumber, MEM_SLOT_LINKED_CHANNEL, linkedChannelId);
      }

      applyValue(channelIdAsNumber, channelValue);

      writePageIntegrity(channelIdAsNumber + 1);
      writePageFromBufferToEeprom(channelIdAsNumber + 1);

      shouldRerender = true;
    }

    // dumpEepromData(0, MAX_EEPROM_RANGE - 1);
  } else {
    // For get requests, we always want to render the page to the client if its
    // not for the favicon

    if (!isKeyInData(m_pageBuffer, "updateChannel")) {
      shouldRerender = true;
    }
  }

  return shouldRerender;
}

void loadOptionsToMemberVariables() {
  m_numChannels = readUInt16FromEepromBuffer(MEM_SLOT_CHANNELS);
  readFromEepromBuffer(MEM_SLOT_FORCE_ALL_OFF, &m_toggleForceAllOff, 1);
  readFromEepromBuffer(MEM_SLOT_FORCE_ALL_ON, &m_toggleForceAllOn, 1);
  readFromEepromBuffer(MEM_SLOT_RANDOM, &m_toggleRandom, 1);
}

bool shouldInvokeEvent(uint8_t freq) {
  // Generate a random number between 0 and 3599
  uint16_t randNumber = random(3600);

  // Check if the random number is less than the threshold
  return randNumber < freq;
}

void calculateRandomEvents() {
  for (int i = 0; i < m_numChannels; i++) {
    bool randomOn = readBoolForChannelFromEepromBuffer(i, MEM_SLOT_RANDOM_ON);
    bool isLinked = readBoolForChannelFromEepromBuffer(i, MEM_SLOT_RANDOM_ON);
    uint16_t linkedChannel =
        readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_LINKED_CHANNEL);

    // No need for further checks if channel has no random events
    if (!randomOn) {
      continue;
    }

    uint8_t randomOnFreq =
        readUint8tForChannelFromEepromBuffer(i, MEM_SLOT_RANDOM_ON_FREQ);

    uint8_t randomOffFreq =
        readUint8tForChannelFromEepromBuffer(i, MEM_SLOT_RANDOM_OFF_FREQ);

    bool turnOn = shouldInvokeEvent(randomOnFreq);
    bool turnOff = shouldInvokeEvent(randomOffFreq);

    if (turnOn) {
      st("Got random on event for channel ");
      sn(i);

      uint16_t brightness =
          readUint16tForChannelFromEepromBuffer(i, MEM_SLOT_BRIGHTNESS);

      applyValue(i, brightness);

      if (isLinked) {
        uint16_t linkedBrightness = readUint16tForChannelFromEepromBuffer(
            linkedChannel, MEM_SLOT_BRIGHTNESS);

        applyValue(linkedChannel, linkedBrightness);
      }
    }

    if (turnOff) {
      st("Got random off event for channel ");
      sn(i);

      applyValue(i, 0);

      if (isLinked) {
        applyValue(linkedChannel, 0);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting");
  Wire.begin();

  // Comment in for HARD RESET
  // clearEeprom();

  clearEepromBuffer();
  clearPageBuffer();

  // Load first page which contains information such as channel count etc...
  loadPageAndCheckIntegrity(0);

  loadOptionsToMemberVariables();

  for (int i = 0; i < m_numChannels; i++) {
    // Page for channel is always channelId + 1 since page 0 = general config;
    loadPageAndCheckIntegrity(i + 1);
  }

  // dumpEepromData(0, MAX_EEPROM_RANGE - 1);

  // Initializte pwm boards
  for (int i = 0; i < PWM_BOARDS; i++) {
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

  applyValues();

  float analogValue = analogRead(0);

  st("Priming randomSeed with ");
  sn(analogValue);
  randomSeed(analogValue);
}

void loop() {
  /*   // check for WiFi OTA updates
  ArduinoOTA.poll(); */

  WiFiClient client = server.available(); // Listen for incoming clients

  if ((m_toggleRandom == 1) && (millis() > (m_lastRandom + RANDOM_INTERVAL))) {
    m_lastRandom = millis();
    applyValues();
  }

  if (millis() > (m_lastRandomEvent + RANDOM_EVENT_INTERVAL)) {
    m_lastRandomEvent = millis();
    calculateRandomEvents();
  }

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

      bool shouldRerender = processRequestAndReturnRerenderNeed();

      if (shouldRerender) {
        sn("Rerender");
        renderWebPage(client);
      } else {
        replyToClient(client);
      }

      client.stop();
    }
  }
}
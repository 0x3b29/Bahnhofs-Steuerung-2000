#include <WiFiNINA.h>
#include <WiFiServer.h>
#include <Adafruit_PWMServoDriver.h>
#include <ArduinoOTA.h>
#include <SPI.h>
#include <Wire.h>

// If you check out this project, this file does not exist.
// You need to create a copy of example_arduino_secrets.h and rename it to
// arduino_secrets.h and fill in your WiFI name and password
#include "arduino_secrets.h"

const int MAX_PWM_BOARDS = 4;
const int servoFrequency = 1042;
// Starting address for writing data
// Offset by 4 to accomodate flags at 0 & 1 & channels (2 byte)
const int startAddress = 0x0004;
const int EEPROM_ADDRESS = 0x50;
const int MAX_EEPROM_RANGE = 1024;

const int MAX_TOTAL_CHANNELS = MAX_PWM_BOARDS * 16;

const int PAGE_BUFFER_SIZE = 8192;

const int CHANNEL_NAME_LENGTH = 20;


char pageBuffer[PAGE_BUFFER_SIZE];

WiFiServer server(80);

char toggleEnergyBuffer[2] = "1";
char toggleRandomBuffer[2] = "0";
char toggleSaveBuffer[2] = "0";

char numChannels[4] = "0";
char channelIdBuffer[4] = "0";
char channelNameBuffer[CHANNEL_NAME_LENGTH + 1] = "";
char channelValueBuffer[5] = "0";

char editChannelIdBuffer[4] = "";

uint16_t channels = 10;
Adafruit_PWMServoDriver pwmBoards[MAX_PWM_BOARDS];

// Function to extract value from form data using char arrays
// formData: The form data as a char array
// key: The key as a char array
// value: A char array buffer where the extracted value will be stored
// valueLen: The length of the value buffer
void getValueFromData(const char* formData, const char* key, char* value, int valueLen) {
  const char* startPtr = strstr(formData, key);
  if (startPtr == NULL) {
    Serial.print("Unable to find key: ");
    Serial.println(key);
    return;
  }

  startPtr += strlen(key);  // Move pointer past the key

  const char* endPtr = strchr(startPtr, '&');
  if (endPtr == NULL) {
    endPtr = formData + strlen(formData);  // Set endPtr to the end of formData if '&' not found
  }

  int numCharsToCopy = endPtr - startPtr;
  if (numCharsToCopy >= valueLen) {
    numCharsToCopy = valueLen - 1;  // Ensure we don't exceed buffer size
  }

  strncpy(value, startPtr, numCharsToCopy);
  value[numCharsToCopy] = '\0';  // Null-terminate the string

  return;
}

bool isKeyInData(const char* formData, const char* key) {
  const char* startPtr = strstr(formData, key);
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

void saveDataToEeprom() {
  Serial.println("Save Data");

  uint8_t toggleEnergy = 0;
  if (toggleEnergyBuffer == "1") {
    toggleEnergy = 1;
  }

  uint8_t toggleRandom = 0;
  if (toggleRandomBuffer == "1") {
    toggleRandom = 1;
  }
  Serial.print("Had toggleEnergyBuffer: '");
  Serial.print(toggleEnergyBuffer);
  Serial.println("'");

  Serial.print("Writing toggleEnergy: ");
  Serial.println(toggleEnergy);
  Serial.print("Writing toggleRandom: ");
  Serial.println(toggleRandom);

  writeToEeprom(0, &toggleEnergy, 1);
  writeToEeprom(1, &toggleRandom, 1);
  saveUInt16ToEeprom(2, channels);

}

void renderWebPage(WiFiClient client) {
  // Send a standard HTTP response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();

  // Output the HTML Web Page
  client.println("<!DOCTYPE html>");

  client.println("<html>");
  client.println("<head>");
  client.println("<meta charset='UTF-8'>");
  client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");                                  // Responsive meta tag
  client.println("<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css'>");  // Bootstrap CSS
  client.println("<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js'></script>");              // jQuery (optional, for Bootstrap JS)
  client.println("<script src='https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.16.0/umd/popper.min.js'></script>");     // Popper JS (optional, for Bootstrap JS)
  client.println("<script src='https://maxcdn.bootstrapcdn.com/bootstrap/4.5.2/js/bootstrap.min.js'></script>");           // Bootstrap JS
  client.println("</head>");
  client.println("<body>");
  client.println("<div class='container'>");
  client.println("<br>");
  client.println("<br>");
  client.println("<h1>Bahnhofs Steuerung 2000</h1>");
  client.println("<form action='/' method='POST' accept-charset='UTF-8'>");
  client.println("<br>");
  client.println("<h3>Optionen</h3>");
  client.print("Kanäle: <input type='number' name='numChannels' min='1' max='512' value='");
  client.print(channels);
  client.println("'><br><br>");
  client.print("Strom: <input type='checkbox' name='toggleEnergy' value='1'");

  if ((strcmp(toggleEnergyBuffer, "1") == 0)) {
    client.print(" checked");
  }

  client.println(">");
  client.println("<br>");

  client.print("Zufall: <input type='checkbox' name='toggleRandom' value='1'");

  if ((strcmp( toggleRandomBuffer, "1") == 0)) {
    client.print(" checked");
  }

  client.println(">");
  client.println("<br>");

  client.println("<input type='submit' name='action' value='Absenden' value='Absenden'/>");
  client.println("<br>");
  client.println("<br>");

  if (editChannelIdBuffer[0] != '\0') {
    client.print("<h3>Kanal ");
    client.print(editChannelIdBuffer);
    client.println(" Bearbeiten</h3>");

    client.print("<input type='hidden'  name='channelId' value='");
    client.print(editChannelIdBuffer);
    client.println("'>");

    client.print("Name: <input type='text' maxlength='20' size='20' name='channelName' value='");
    client.print(channelNameBuffer);
    client.println("'>");
    client.println("<br>");

    client.print("Helligkeit: <input type='range' min='0' max='4095' name='channelValue' value='");
    client.print(readBrightnessForChannelFromEeprom(atoi(editChannelIdBuffer)));
    client.println("'>");
    client.println("<br>");

    client.println("<input type='submit' name='updateChannel' value='Speichern'/> &nbsp; <input type='submit' name='ignoreChannel' value='Verwerfen'/>");

    client.println("<br>");
  }

  client.println("<br>");

  client.println("<h3>Übersicht der Kanäle</h3>");
  client.println("<table class='table table-striped'>");

  client.println(
    "<tr>"
    " <th>ID) Karte : Pin</th>"
    " <th>Name</th>"
    " <th>Helligkeit</th>"
    " <th>Zufall</th>"
    " <th>Bearbeiten</th>"
    "</tr>");

  for (int i = 0; i < channels; i++) {
    readNameForChannelFromEepromToBuffer(i);
    uint16_t brightness = readBrightnessForChannelFromEeprom(i);

    client.print("  <tr>");

    client.println("    <td>");
    
    client.print(i);
    client.print(")  ");
    client.print(getBoardIndexForChannel(i));
    client.print(" : ");
    client.print(getBoardSubAddressForChannel(i));
    client.println("    </td>");

    client.println("    <td>");
    client.println(channelNameBuffer);
    client.println("    </td>");

    client.println("    <td>");
    client.println(brightness);
    client.println("    </td>");

    client.println("    <td>");
    client.println("Ja");
    client.println("    </td>");

    client.println("    <td>");
    client.print("<button  type='submit' name='editChannel' value='");
    client.print(i);
    client.print("'>🖊</button >");
    client.println("    </td>");

    client.println("  </tr>");
  }
  client.println("</table>");

  client.println("<br>");
  client.println("</form>");
  client.println("</div>");  // Close container div
  client.println("</body></html>");
  client.println();
}

void readAllDataFromEeprom() {
  Serial.println("Read Data");

  uint8_t toggleEnergy = 0;
  uint8_t toggleRandom = 0;

  readFromEeprom(0, &toggleEnergy, 1);
  readFromEeprom(1, &toggleRandom, 1);
  uint16_t readchannels = readUInt16FromEeprom(2);

  Serial.println("readchannels: ");
  Serial.println(readchannels);

  if (channels > MAX_TOTAL_CHANNELS) {
    channels = MAX_TOTAL_CHANNELS;
  }

  Serial.print("Reading toggleEnergy: ");
  Serial.println(toggleEnergy);
  Serial.print("Reading toggleRandom: ");
  Serial.println(toggleRandom);

  toggleEnergyBuffer[0] = '0';
  if (toggleEnergy == 1) {
    toggleEnergyBuffer[0] = '1';
  }

  toggleRandomBuffer[0] = 0;
  if (toggleRandom == 1) {
    toggleRandomBuffer[0] = 1;
  }
}

void saveUInt16ToEeprom(uint16_t writeAddress, uint16_t value) {
  uint8_t data[2];                // Create a byte array to hold the uint16_t value
  data[0] = (value >> 8) & 0xFF;  // Extract the high byte
  data[1] = value & 0xFF;         // Extract the low byte

  writeToEeprom(writeAddress, data, 2);
}

void writeToEeprom(uint16_t writeAddress, uint8_t* data, uint8_t len) {
  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.write((byte)((writeAddress & 0xFF00) >> 8));
  Wire.write((byte)(writeAddress & 0x00FF));
  uint8_t i;
  for (i = 0; i < len; i++) {
    Wire.write(data[i]);
  }
  Wire.endTransmission();
  delay(10);
}

uint16_t readUInt16FromEeprom(uint16_t readAddress) {
  uint8_t data[2];
  readFromEeprom(readAddress, data, 2);

  uint16_t value = ((uint16_t)data[0] << 8) | data[1];
  return value;
}

void readNameForChannelFromEepromToBuffer(int channel) {
  uint16_t startAddress = 64 + channel * 64;
  readFromEeprom(startAddress, (uint8_t*)channelNameBuffer, 21);
}

void writeNameForChannelFromBufferToEeprom(int channel) {
  uint16_t startAddress = 64 + channel * 64;
  writeToEeprom(startAddress, (uint8_t*)channelNameBuffer, 21);
}

void writeBrightnessForChannelToEeprom(int channel, uint16_t channelValue) {
  int startAddress = 64 + channel * 64 + 30;
  saveUInt16ToEeprom(startAddress, channelValue);
}

uint16_t readBrightnessForChannelFromEeprom(int channel) {
  int startAddress = 64 + channel * 64 + 30;
  return readUInt16FromEeprom(startAddress);
}

void readFromEeprom(uint16_t readAddress, uint8_t* data, uint8_t len) {
  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.write((byte)((readAddress & 0xFF00) >> 8));
  Wire.write((byte)(readAddress & 0x00FF));
  Wire.endTransmission();

  Wire.requestFrom(EEPROM_ADDRESS, len);
  int i;
  for (i = 0; i < len; i++) {
    if (Wire.available()) {
      data[i] = Wire.read();
    } else {
      data[i] = 0;
    }
  }
}

void dumpEepromData( int startAddress, int endAddress) {

  char asciiBuffer[9];  // Buffer to store ASCII characters (8 characters + null terminator)
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
    asciiBuffer[bufferIndex++] = (byteValue >= 32 && byteValue <= 126) ? byteValue : '.';

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

void resetEeprom() {
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

  Serial.print("Erasing eeprom finished");
}

void applyValue(int channel, uint16_t brightness) {
    int boardIndex = getBoardIndexForChannel(channel);
    int subAddress = getBoardSubAddressForChannel(channel);

    Serial.print("Apply channel ");
    Serial.print(channel);
    Serial.print(" to board ");
    Serial.print(boardIndex);
    Serial.print(" and sub ");
    Serial.print(subAddress);
    Serial.print(": ");

    int value = 0;
    if (strcmp( toggleEnergyBuffer, "1") == 0) {
      if (strcmp(toggleRandomBuffer, "1") == 0) {
        value = random(0, 4096);
        pwmBoards[boardIndex].setPWM(subAddress, 0, value);
      } else {
        value = brightness;
        pwmBoards[boardIndex].setPWM(subAddress, 0, value);
      }
    } else {
      Serial.println("OFF");
      value = 0;
      pwmBoards[boardIndex].setPWM(subAddress, 0, value);
    }

    Serial.println(value);
}

void applyValues() {
  for (int i = 0; i < channels; i++) {
    // TODO
  }
}

void clearPageBuffer() {
  for (int i = 0; i < PAGE_BUFFER_SIZE; i++) {
    pageBuffer[i] = '\0';
  }
}

void checkPageBufferForPostData() {
  if (strstr(pageBuffer, "POST") != NULL) {
    Serial.println("Was post request");

    // Initialize the toggles with 0 since they might not be present in the post data if unchecked
    strcpy(toggleEnergyBuffer, "0");
    strcpy(toggleRandomBuffer, "0");
    strcpy(toggleSaveBuffer, "0");

    // Find each value
    getValueFromData(pageBuffer, "toggleEnergy=", toggleEnergyBuffer, 2);
    getValueFromData(pageBuffer, "toggleRandom=", toggleRandomBuffer, 2);
    getValueFromData(pageBuffer, "toggleSave=", toggleSaveBuffer, 2);

    getValueFromData(pageBuffer, "channelId=", channelIdBuffer, 5);
    getValueFromData(pageBuffer, "channelName=", channelNameBuffer, 21);
    getValueFromData(pageBuffer, "channelValue=", channelValueBuffer, 5);

    getValueFromData(pageBuffer, "numChannels=", numChannels, 4);

    strcpy(editChannelIdBuffer, "");
    getValueFromData(pageBuffer, "editChannel=", editChannelIdBuffer, 4);

    if (isKeyInData(pageBuffer, "editChannel")) {
      uint16_t channelIdAsNumber = atoi(editChannelIdBuffer);
      readNameForChannelFromEepromToBuffer(channelIdAsNumber);
    }

    if (isKeyInData(pageBuffer, "updateChannel")) {

      uint16_t channelIdAsNumber = atoi(channelIdBuffer);
      Serial.print("Channel ");
      Serial.print(channelIdAsNumber);

      uint16_t startAddress = 64 + channelIdAsNumber * 64;
      Serial.print(" got startAddress ");
      Serial.println(startAddress);

      uint16_t channelValue = atoi(channelValueBuffer);

      writeNameForChannelFromBufferToEeprom(channelIdAsNumber);
      writeBrightnessForChannelToEeprom(channelIdAsNumber, channelValue);

      Serial.print("toggleEnergyBuffer: ");
      Serial.println(toggleEnergyBuffer);
      applyValue(channelIdAsNumber, channelValue);

      // dumpEepromData(0, 1023);
    }


    /*       if (toggleSaveBuffer == "1") {
              saveDataToEeprom();
              readAllDataFromEeprom();
            }

            applyValues(); */
  } else {
    Serial.println("Was not a post request");
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting");
  Wire.begin();

  // resetEeprom();

  clearPageBuffer();
  // Initializte pwm boards
  for (int i = 0; i < MAX_PWM_BOARDS; i++) {
    int pwmAddress = 0x40 + i;

    pwmBoards[i] = Adafruit_PWMServoDriver(pwmAddress);
    pwmBoards[i].begin();
    pwmBoards[i].setOscillatorFrequency(27000000);
    pwmBoards[i].setPWMFreq(servoFrequency);

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
  ArduinoOTA.begin(WiFi.localIP(), "Arduino_MKR_WiFi_1010", SECRET_OTA, InternalStorage); */

  Serial.println("Server started");

  dumpEepromData(0, 1023);

  readAllDataFromEeprom();
  applyValues();

  for (int i = 0; i < channels; i++) {
    Serial.print("Channel ");
    Serial.print(i);
    Serial.print(" is on board index ");
    Serial.print(getBoardIndexForChannel(i));
    Serial.print(" with board address ");
    Serial.print(getBoardAddressForChannel(i));
    Serial.print(" with subaddress for channel ");
    Serial.println(getBoardSubAddressForChannel(i));
  }
}

void loop() {
  /*   // check for WiFi OTA updates
  ArduinoOTA.poll(); */

  WiFiClient client = server.available();  // Listen for incoming clients

  if (client) {
    Serial.println("New client");
    boolean currentLineIsBlank = true;

    clearPageBuffer();
    int pageIndex = 0;

    if (client.connected()) {
      while (client.available()) {
        char c = client.read();
        Serial.write(c);

        pageBuffer[pageIndex] = c;
        pageIndex++;

        if (pageIndex >= PAGE_BUFFER_SIZE) {
          Serial.println("WARNING: PAGE EXCEEDED BUFFER SIZE! DANGER!!!");
        }
      }

      checkPageBufferForPostData();

      renderWebPage(client);

      // Close the connection
      client.stop();
    }

    Serial.println("Client disconnected");
  }
}
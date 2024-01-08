#include "Arduino.h"
#include <WiFiServer.h>
#include <SPI.h>
#include <Wire.h>
#include "Adafruit_PWMServoDriver.h"
#include "WiFi.h"
#include "EEPROM.h"
#include "arduino_secrets.h"

#define EEPROM_SIZE 4095

const int MAX_PWM_BOARDS = 32;
const int servoFrequency = 1042;
// Starting address for writing data
// Offset by 3 to accomodate flags at 0 & 1 & channels
const int startAddress = 0x0003;
const int MAX_TOTAL_CHANNELS = MAX_PWM_BOARDS * 16;

WiFiServer server(80);

String toggleAllValue = "1";
String toggleRandomValue = "0";
String toggleSaveValue = "0";

uint8_t channels = 48;
uint16_t channelValues[16 * 32];
Adafruit_PWMServoDriver pwmBoards[MAX_PWM_BOARDS];

void writeToEeprom(int writeAddress, uint8_t* data, uint8_t len) {
  for (int i = 0; i < len; i++) {
    EEPROM.write(writeAddress + i, data[i]);
  }
}

void readFromEeprom(int readAddress, uint8_t* data, uint8_t len) {
  for (int i = 0; i < len; i++) {
    data[i] = EEPROM.read(readAddress + i);
  }
}


// Helper function to get values from the form data
String getValueFromFormData(String formData, String key) {
  int start = formData.indexOf(key);
  if (start == -1) return "";  // If the key isn't found, return empty string
  start += key.length();
  int end = formData.indexOf('&', start);  // Find the end of the value
  if (end == -1) end = formData.length();
  return formData.substring(start, end);
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

void printAllChannelsWithValue() {
  for (int i = 0; i < channels; i++) {
    Serial.print("Channel ");
    Serial.print(i + 1);
    Serial.print(" has value ");
    Serial.println(channelValues[i]);
  }
}

void saveDataToEeprom() {
  Serial.println("Save Data");

  uint8_t toggleAll = 0;
  if (toggleAllValue == "1") {
    toggleAll = 1;
  }

  uint8_t toggleRandom = 0;
  if (toggleRandomValue == "1") {
    toggleRandom = 1;
  }
  Serial.print("Had toggleAllValue: '");
  Serial.print(toggleAllValue);
  Serial.println("'");

  Serial.print("Writing toggleAll: ");
  Serial.println(toggleAll);
  Serial.print("Writing toggleRandom: ");
  Serial.println(toggleRandom);

  Serial.println("Writing Channels");
  printAllChannelsWithValue();

  writeToEeprom(0, &toggleAll, 1);
  writeToEeprom(1, &toggleRandom, 1);
  writeToEeprom(2, &channels, 1);

  for (int i = 0; i < channels; i++) {
    int address = startAddress + i * 2;  // Calculate address for each channel
    uint16_t data = channelValues[i];
    uint8_t highByte = data >> 8;   // High byte
    uint8_t lowByte = data & 0xFF;  // Low byte
    uint8_t buffer[2] = { highByte, lowByte };
    writeToEeprom(address, buffer, 2);  // Write both bytes to EEPROM
  }
}

void renderWebPage(WiFiClient client) {
  // Send a standard HTTP response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();

  // Output the HTML Web Page
  client.println("<!DOCTYPE html>");  // Add DOCTYPE declaration for HTML5

  client.println("<html>");
  client.println("<head>");
  client.println("<meta charset=\"UTF-8\">");
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");                                  // Responsive meta tag
  client.println("<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css\">");  // Bootstrap CSS
  client.println("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js\"></script>");                // jQuery (optional, for Bootstrap JS)
  client.println("<script src=\"https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.16.0/umd/popper.min.js\"></script>");       // Popper JS (optional, for Bootstrap JS)
  client.println("<script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.5.2/js/bootstrap.min.js\"></script>");             // Bootstrap JS
  client.println("</head>");
  client.println("<body>");
  client.println("<div class='container'>");  // Bootstrap container class
  client.println("<br>");
  client.println("<br>");
  client.println("<h1>Bahnhofs Steuerung 2000</h1>");
  client.println("<form action=\"/\" method=\"POST\" accept-charset=\"UTF-8\">");

  client.println("<input type=\"submit\" value=\"Absenden\">");
  client.println("<br>");
  client.println("<br>");
  client.println("<h3>Optionen</h3>");
  client.println("Kanäle: <input type='number' name='numChannels' min='1' max='512' value='" + String(channels) + "'><br><br>");
  client.print("Strom: <input type=\"checkbox\" name=\"toggleAll\" value=\"1\"");
  if (toggleAllValue == "1") {  // Assuming toggleAllValue is "1" when the checkbox should be checked
    client.print(" checked");
  }
  client.println(">");
  client.println("<br>");

  client.print("Zufall: <input type=\"checkbox\" name=\"toggleRandom\" value=\"1\"");
  if (toggleRandomValue == "1") {  // Assuming toggleAllValue is "1" when the checkbox should be checked
    client.print(" checked");
  }
  client.println(">");
  client.println("<br>");

  client.print("Speichern?: <input type=\"checkbox\" name=\"toggleSave\" value=\"1\">");
  client.println("<br>");
  client.println("<br>");

  client.println("<h3>Kanäle</h3>");

  for (int i = 0; i < channels; i++) {
    client.print("Channel ");
    client.print(i + 1);
    client.print(": <input type=\"range\" min=\"0\" max=\"4095\" name=\"channel");
    client.print(i + 1);
    client.print("\" value=\"");
    client.print(channelValues[i]);
    client.println("\"><br>");
  }
  client.println("<br>");
  client.println("<input type=\"submit\" value=\"Absenden\">");
  client.println("</form>");
  client.println("</div>");  // Close container div
  client.println("</body></html>");
  client.println();
}


void readAllDataFromEeprom() {
  Serial.println("Read Data");

  uint8_t toggleAll = 0;
  uint8_t toggleRandom = 0;

  readFromEeprom(0, &toggleAll, 1);
  readFromEeprom(1, &toggleRandom, 1);
  readFromEeprom(2, &channels, 1);

  for (int i = 0; i < channels; i++) {
    int address = startAddress + i * 2;  // Calculate address for each channel
    uint8_t buffer[2];
    readFromEeprom(address, buffer, 2);                // Read two bytes from EEPROM
    uint16_t data = ((uint16_t)buffer[0] << 8) | buffer[1];  // Combine the high and low bytes
    channelValues[i] = data;
  }

  Serial.print("Reading toggleAll: ");
  Serial.println(toggleAll);
  Serial.print("Reading toggleRandom: ");
  Serial.println(toggleRandom);

  toggleAllValue = "0";
  if (toggleAll == 1) {
    toggleAllValue = "1";
  }

  toggleRandomValue = "0";
  if (toggleRandom == 1) {
    toggleRandomValue = "1";
  }

  Serial.print("Reading Channels: ");
  Serial.println(channels);
  // printAllChannelsWithValue();
}



void applyValues() {
  for (int i = 0; i < channels; i++) {
    int boardIndex = getBoardIndexForChannel(i);
    int subAddress = getBoardSubAddressForChannel(i);

    Serial.print("Apply channel ");
    Serial.print(i);
    Serial.print(" to board ");
    Serial.print(boardIndex);
    Serial.print(" and sub ");
    Serial.print(subAddress);
    Serial.print(": ");

    int value = 0;
    if (toggleAllValue == "1") {
      if (toggleRandomValue == "1") {
        value = random(0, 4096);
        pwmBoards[boardIndex].setPWM(subAddress, 0, value);
      } else {
        value = channelValues[i];
        pwmBoards[boardIndex].setPWM(subAddress, 0, value);
      }
    } else {
      value = 0;
      pwmBoards[boardIndex].setPWM(subAddress, 0, value);
    }

    Serial.println(value);
  }
}

void setup() {
  Serial.begin(115200);

  delay(2000);

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

  Serial.println("Starting");

  // Attempt to connect to WiFi network
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Attempting to connect to WiFi network...");
    WiFi.begin(SECRET_SSID, SECRET_PASS);
    delay(5000);
  }

  Serial.println("Connected to WiFi");

  // Print the IP address
  Serial.print("Server IP Address: ");
  Serial.println(WiFi.localIP());

  // Start the server
  server.begin();

  Serial.println("Server started");

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
  WiFiClient client = server.available();  // Listen for incoming clients

  if (client) {
    Serial.println("New client");
    boolean currentLineIsBlank = true;
    String httpReq = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        httpReq += c;
        if (c == '\n' && currentLineIsBlank) {

          // Handle the POST request
          if (httpReq.indexOf("POST") != -1) {
            // Wait for all data from the client to be received
            while (client.available()) {
              char c = client.read();
              // Serial.write(c);
              httpReq += c;
            }

            // Find the start of the data (after the headers)
            int startIdx = httpReq.indexOf("\r\n\r\n");
            if (startIdx != -1) {
              // Extract the data
              String formData = httpReq.substring(startIdx + 4);  // +4 to skip over the "\r\n\r\n"

              // Find each value
              toggleAllValue = getValueFromFormData(formData, "toggleAll=");
              toggleRandomValue = getValueFromFormData(formData, "toggleRandom=");
              toggleSaveValue = getValueFromFormData(formData, "toggleSave=");

              String numChannelsString = getValueFromFormData(formData, "numChannels=");
              int newChannelCount = numChannelsString.toInt();

              // Print values to Serial (for debugging)
              Serial.print("ToggleAllValue: ");
              Serial.println(toggleAllValue);
              Serial.print("ToggleRandValue: ");
              Serial.println(toggleRandomValue);
              Serial.print("ToggleSaveValue: ");
              Serial.println(toggleSaveValue);

              for (int i = 0; i < channels; i++) {
                String key = "channel" + String(i + 1) + "=";
                channelValues[i] = getValueFromFormData(formData, key).toInt();
                Serial.print(key);
                Serial.print(": ");
                Serial.println(channelValues[i]);
              }

              // Validation of newChannelCount
              if (newChannelCount > 0 && newChannelCount <= MAX_TOTAL_CHANNELS && channels != newChannelCount) {  // Define MAX_TOTAL_CHANNELS as per your limit
                channels = newChannelCount;
              }

              if (toggleSaveValue == "1") {
                saveDataToEeprom();
                readAllDataFromEeprom();
              }

              applyValues();
            }
          }

          renderWebPage(client);
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }

    // Close the connection
    client.stop();
    Serial.println("Client disconnected");
  }
}

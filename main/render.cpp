#include "render.h"
#include "eeprom.h"
#include "helpers.h"
#include <WiFiNINA.h>

void renderOptions(WiFiClient client, uint16_t numChannels,
                   bool toggleOneBasedAddresses, bool toggleCompactDisplay,
                   bool toggleForceAllOff, bool toggleForceAllOn,
                   bool toggleRandom, uint16_t channelIdToEdit) {
  pn("<div class='h3'>Optionen</div>");

  // Max value = Max number of boards (62 are max, but -1 because eeprom
  // address so 61) * number of pins => 61 * 16 = 976
  pt("<div class='row'>"
     "<div class='col-3 d-flex align-items-center'> Kanäle: </div> <div "
     "class='col-5'> <input type='number' class='form-control w-100' "
     "name='numChannels' min='0' "
     "max='");
  pt(MAX_TOTAL_CHANNELS);
  pt("' value='");
  pt(numChannels);
  pt("'> </div>");
  pn("<div class='col-4 d-flex justify-content-end'><button class='btn "
     "btn-primary' type='submit' "
     "name='updateSettings' value='Absenden'>Senden</button> </div></div>"
     "<br>");

  // 1 Indexierte Adressen
  pt("<div class='form-check form-switch'>");
  pt("<input class='form-check-input' type='checkbox' "
     "name='toggleOneBasedAddresses' "
     "value='1'  id='toggleOneBasedAddresses' onchange='sendCheckbox(this, "
     "true)'");

  if (toggleOneBasedAddresses == true) {
    pn(" checked>");
  } else {
    pt(">");
  }

  pt("<label class='form-check-label' "
     "for='toggleOneBasedAddresses'>Adressierung "
     "startet bei 1</label>");
  pt("</div>");
  // /1 Indexierte Adressen

  // Kompakte Übersicht
  pt("<div class='form-check form-switch'>");
  pt("<input class='form-check-input' type='checkbox' "
     "name='toggleCompactDisplay' "
     "value='1'  id='toggleCompactDisplay' onchange='sendCheckbox(this, "
     "true)'");

  if (toggleCompactDisplay == true) {
    pn(" checked>");
  } else {
    pt(">");
  }

  pt("<label class='form-check-label' "
     "for='toggleCompactDisplay'>Kompakte Übersicht</label>");
  pt("</div>");
  // / Kompakte Übersicht

  // Alles Aus Switch
  pt("<div class='form-check form-switch'>");
  pt("<input class='form-check-input' type='checkbox' "
     "name='toggleForceAllOff' "
     "value='1'  id='toggleForceAllOff' onchange='sendCheckbox(this, "
     "false)'");

  if (toggleForceAllOff == true) {
    pn(" checked>");
  } else {
    pt(">");
  }

  pt("<label class='form-check-label' for='toggleForceAllOff'>Alle Kanäle "
     "dauerhaft auf "
     "0%</label>");
  pt("</div>");
  // /Alles Aus Switch

  // Alles An Switch
  pt("<div class='form-check form-switch'>");
  pt("<input class='form-check-input' type='checkbox' "
     "name='toggleForceAllOn' "
     "value='1' id='toggleForceAllOn' onchange='sendCheckbox(this, false)'");

  if (toggleForceAllOn == true) {
    pn(" checked>");
  } else {
    pt(">");
  }

  pt("<label class='form-check-label' for='toggleForceAllOn'>Alle Kanäle "
     "dauerhaft "
     "auf "
     "100%</label>"
     "</div>");
  // /Alles An Switch

  // Zufalls Switch
  pt("<div class='form-check form-switch'>"
     "<input class='form-check-input' type='checkbox' name='toggleRandom'  "
     "value='1' role='switch' "
     "id='toggleRandom' onchange='sendCheckbox(this, false)'");

  if (toggleRandom == true) {
    pn(" checked>");
  } else {
    pt(">");
  }

  pt("<label class='form-check-label' for='toggleRandom'>Verrücktes "
     "Blinken</label>"
     "</div>"
     // /Zufalls Switch

     "<input type='hidden'  name='clearEeprom' value='0'>");
}

void renderEditChannel(WiFiClient client, bool renderAnchor,
                       uint16_t anchorChannelId, uint16_t numChannels,
                       bool toggleOneBasedAddresses, uint16_t channelIdToEdit) {
  pt("<h3>Kanal ");
  pt(toggleOneBasedAddresses ? channelIdToEdit + 1 : channelIdToEdit);
  pn(" Bearbeiten</h3>");

  pt("<input type='hidden'  name='channelId' value='");
  pt(channelIdToEdit);
  pn("'>");

  pt("<div class='row'>");
  pt("  <div class='col'>");
  pt("Beschreibung");
  pt("  </div>");
  pt("  <div class='col d-flex justify-content-end'>");
  pt("<input type='text' maxlength='20' size='20' "
     "name='channelName' value='");
  pt(m_channelNameBuffer);
  pn("'>");
  pt("  </div>");
  pt("</div>");

  pt("<br>");

  pt("<div class='form-check form-switch'>"
     "<input class='form-check-input' type='checkbox' "
     "name='initialState' value='1' role='switch' id='initialState' "
     "value='1'");

  if (readBoolForChannelFromEepromBuffer(channelIdToEdit,
                                         MEM_SLOT_INITIAL_STATE)) {
    pn(" checked>");
  } else {
    pt(">");
  }

  pt("<label class='form-check-label' for='toggleRandom'>Startzustand</label>"
     "</div>");

  pt("<div class='row'>");
  pt("  <div class='col'>");
  pt("Helligkeit");
  pt("  </div>");
  pt("  <div class='col d-flex justify-content-end'>");
  pt("<input type='range' min='0' max='4095' "
     "name='channelValue' value='");
  pt(readUint16tForChannelFromEepromBuffer(channelIdToEdit,
                                           MEM_SLOT_BRIGHTNESS));
  pn("'>");
  pt("  </div>");
  pt("</div>");

  pt("<div class='form-check form-switch pt-3'>"
     "<input class='form-check-input' type='checkbox' "
     "name='randomOn' value='1' role='switch' id='randomOn' "
     "value='1'");

  if (readBoolForChannelFromEepromBuffer(channelIdToEdit, MEM_SLOT_RANDOM_ON)) {
    pn(" checked>");
  } else {
    pt(">");
  }

  pt("<label class='form-check-label' for='randomOn'>Zufälliges "
     "Einschalten</label>"
     "</div>");

  pt("<div class='row'>");
  pt("  <div class='col'>");
  pt("~ Zufälle/h");
  pt("  </div>");
  pt("  <div class='col d-flex justify-content-end'>");
  pt("<input type='number' name='frequencyOn' min='0' max='255' value='");
  pt(readUint8tForChannelFromEepromBuffer(channelIdToEdit,
                                          MEM_SLOT_RANDOM_ON_FREQ));
  pn("'>");
  pt("  </div>");
  pt("</div>");

  pt("<div class='form-check form-switch pt-3'>"
     "<input class='form-check-input' type='checkbox' "
     "name='randomOff' value='1' role='switch' id='randomOff' "
     "value='1'");

  if (readBoolForChannelFromEepromBuffer(channelIdToEdit,
                                         MEM_SLOT_RANDOM_OFF)) {
    pn(" checked>");
  } else {
    pt(">");
  }

  pt("<label class='form-check-label' for='randomOff'>Zufälliges "
     "Ausschalten</label>"
     "</div>");

  pt("<div class='row'>");
  pt("  <div class='col'>");
  pt("~ Zufälle/h");
  pt("  </div>");
  pt("  <div class='col d-flex justify-content-end'>");
  pt("<input type='number' name='frequencyOff' min='0' max='255' value='");
  pt(readUint8tForChannelFromEepromBuffer(channelIdToEdit,
                                          MEM_SLOT_RANDOM_OFF_FREQ));
  pn("'>");
  pt("  </div>");
  pt("</div>");

  pt("<div class='form-check form-switch pt-3'>"
     "<input class='form-check-input' type='checkbox' "
     "name='channelLinked' value='1' role='switch' id='channelLinked' "
     "value='1'");

  if (readBoolForChannelFromEepromBuffer(channelIdToEdit, MEM_SLOT_IS_LINKED)) {
    pn(" checked>");
  } else {
    pt(">");
  }

  pt("<label class='form-check-label' for='channelLinked'>Verknüpft</label>"
     "</div>");

  pt("<div class='row'>");
  pt("  <div class='col'>");
  pt("Gesteuert durch Kanal");
  pt("  </div>");
  pt("  <div class='col d-flex justify-content-end'>");

  pt("<input type='number' "
     "name='linkedChannelId' min='");
  pt(toggleOneBasedAddresses ? 1 : 0);
  pt("' max='");

  pt(toggleOneBasedAddresses ? numChannels : numChannels - 1);
  pt("' value='");

  uint16_t linkedChannelId = readUint16tForChannelFromEepromBuffer(
      channelIdToEdit, MEM_SLOT_LINKED_CHANNEL);

  pt(toggleOneBasedAddresses ? linkedChannelId + 1 : linkedChannelId);

  pn("'>");

  pt("  </div>");
  pt("</div>");

  pt("<br>");

  pt("<div class='row'>");
  pt("  <div class='col'>");
  pt("    <input class='btn btn-warning' type='submit' name='ignoreChannel' "
     "value='Verwerfen'/>");
  pt("  </div>");
  pt("  <div class='col d-flex justify-content-end'>");
  pn("    <input class='btn btn-primary ' type='submit' name='updateChannel' "
     "value='Speichern'/> &nbsp; ");
  pt("  </div>");
  pt("</div>");
}

void renderChannelDetail(WiFiClient client, bool toggleOneBasedAddresses,
                         uint16_t channelId) {
  readChannelNameFromEepromBufferToChannelNameBuffer(channelId);
  uint16_t brightness =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_BRIGHTNESS);

  bool initialState =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_INITIAL_STATE);

  bool randomOn =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_ON);
  uint8_t randomOnFreq =
      readUint8tForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_ON_FREQ);

  bool randomOff =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_OFF);
  uint8_t randomOffFreq =
      readUint8tForChannelFromEepromBuffer(channelId, MEM_SLOT_RANDOM_OFF_FREQ);

  bool isLinked =
      readBoolForChannelFromEepromBuffer(channelId, MEM_SLOT_IS_LINKED);
  uint16_t linkedChannel =
      readUint16tForChannelFromEepromBuffer(channelId, MEM_SLOT_LINKED_CHANNEL);

  if (toggleOneBasedAddresses) {
    linkedChannel++;
  }

  int boardIndex = getBoardIndexForChannel(channelId);
  int subAddress = getBoardSubAddressForChannel(channelId);

  pt("<div id='channel-");
  pt(channelId);
  pn("' class='pl-1 pr-1'>"
     // ROW START
     "  <div class='row'>"
     "      <div class='col-9'>"

     //  FIRST COL
     "<span class='h4'>"
     "Kanal ");

  pt(toggleOneBasedAddresses ? channelId + 1 : channelId);
  pt(" </span > "
     "Board ");
  pt(toggleOneBasedAddresses ? boardIndex + 1 : boardIndex);
  pt(", Pin ");
  pt(toggleOneBasedAddresses ? subAddress + 1 : subAddress);
  //  / FIRST COL

  pt("      </div>"
     "      <div class='col-3'>"

     // SECOND COL
     "      <div class='d-flex justify-content-end'>"
     "<button class='btn text-warning'  onclick=\"sendValue('turnChannelOn', "
     "'");
  pt(channelId);
  pt("')\" >⛭</button >"
     "<button class='btn'  onclick=\"sendValue('turnChannelOff', '");
  pt(channelId);
  pt("')\" >⛭</button >"

     "<button class='btn' type='submit' name='editChannel' value='");
  pt(channelId);
  pt("'>🖊</button >"
     "      </div>"
     // / SECOND COL

     "      </div>"
     "  </div>"
     // ROW END

     "  <div class='row'>"
     "    <div class='col'>"
     "<span class='h6'>Beschreibung</span>"
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
     "      <span class='h6'>Zufälliges Einschalten</span>"
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
       "      <span class='h6'>~Zufälle/h</span>"
       "    </div>"
       "    <div class='col'>");
    pt(randomOnFreq);
    pn("/h"
       "    </div>"
       "  </div>");
  }

  pn("  <div class='row'>"
     "    <div class='col'>"
     "      <span class='h6'>Zufälliges Ausschalten</span>"
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
       "      <span class='h6'>~Zufälle/h</span>"
       "    </div>"
       "    <div class='col'>");
    pt(randomOffFreq);
    pn("/h"
       "    </div>"
       "  </div>");
  }

  pn("  <div class='row'>"
     "    <div class='col'>"
     "      <span class='h6'>Verknüpft</span>"
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
       "      <span class='h6'>Gesteuert durch Kanal</span>"
       "    </div>"
       "    <div class='col'>");
    pn(linkedChannel);
    pn("    </div>"
       "  </div>");
  }

  pn("  </div>"
     "<hr class='mb-3 mt-3'/>");
}

void renderChannelDetailCompact(WiFiClient client, bool toggleOneBasedAddresses,
                                uint16_t channelId) {
  readChannelNameFromEepromBufferToChannelNameBuffer(channelId);

  int boardIndex = getBoardIndexForChannel(channelId);
  int subAddress = getBoardSubAddressForChannel(channelId);

  pt("<div id='channel-");
  pt(channelId);
  pn("' class='pl-1 pr-1'>"
     // ROW START
     "  <div class='row'>"
     "      <div class='col-9 d-flex align-items-center'>"

     //  FIRST COL

  );
  if (strcmp(m_channelNameBuffer, "") == false) {
    pt("Kanal ");
    pt(toggleOneBasedAddresses ? channelId + 1 : channelId);
    pt(", "
       "Board ");
    pt(toggleOneBasedAddresses ? boardIndex + 1 : boardIndex);
    pt(", Pin ");
    pt(toggleOneBasedAddresses ? subAddress + 1 : subAddress);
  } else {
    pt("<span>");
    pt(m_channelNameBuffer);
    pt("</span >");
  }
  //  / FIRST COL

  pt("  </div>"
     "    <div class='col-3'>"

     // SECOND COL
     "      <div class='d-flex justify-content-end'>"
     "        <button class='btn text-warning'  "
     "onclick=\"sendValue('turnChannelOn', "
     "'");
  pt(channelId);
  pt("')\" >⛭</button >"
     "        <button class='btn'  onclick=\"sendValue('turnChannelOff', '");
  pt(channelId);
  pt("')\" >⛭</button >"
     "        <button class='btn' type='submit' name='editChannel' value='");
  pt(channelId);
  pt("'>🖊</button >"
     "     </div>"
     // / SECOND COL

     "   </div>"
     " </div>");
  // ROW END

  pn("  </div>"
     "<hr class='mb-1 mt-1'/>");
}

void renderHeadJavascript(WiFiClient client) {
  pn("<script>"
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
     "function sendCheckbox(checkbox, reloadAfterRequest) {"
     "var dataString = encodeURIComponent(checkbox.name) + '=' + "
     "encodeURIComponent(checkbox.checked ? 1 : 0);"
     "console.log(dataString);"
     "fetch('/', {"
     "    method: 'POST',"
     "    headers: {"
     "        'Content-Type': 'application/x-www-form-urlencoded'"
     "    },"
     "    body: dataString"
     "})"
     ".then(response => {"
     " if (reloadAfterRequest) {"
     " window.location.href = '/'; "
     " }"
     "})"
     "}"
     "</script>");
}

void renderWebPage(WiFiClient client, bool foundRecursion,
                   bool renderWithOptionsVisible,
                   bool renderWithEditChannelVisible, bool renderAnchor,
                   uint16_t anchorChannelId, uint16_t numChannels,
                   bool toggleOneBasedAddresses, bool toggleCompactDisplay,
                   bool toggleForceAllOff, bool toggleForceAllOn,
                   bool toggleRandom, uint16_t channelIdToEdit) {

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
     "<link "
     "href='https://fonts.googleapis.com/css2?family=Grape+Nuts&display=swap' "
     "rel='stylesheet'>"
     "<link rel='icon' href='data:;base64,iVBORw0KGgo='>");

  renderHeadJavascript(client);

  pn("</head>"
     "<body>"
     "<div class='container'>"
     "<div class='row justify-content-center'>"
     "<div class='col col-12 col-sm-10 col-md-8 col-lg-6'>"

     "<div class='h1 mt-4 mb-3' style=\"font-family: 'Grape Nuts', bold; "
     "font-size: xx-large;\">Bahnhofs Steuerung 2000</div>"

     "<form id='myForm' action='/' method='POST' accept-charset='UTF-8'>");

  if (foundRecursion) {
    pt("<div class='pb-3'><span class='text-danger'>Achtung: Schleife oder zu "
       "tiefe Verschachtelung (> ");
    pt(MAX_RECURSION);
    pn(") in "
       "verknüpften Kanälen "
       "entdeckt. Bitte überprüfe alle Verknüpfungen auf Schleifen oder erhöhe "
       "die maximale Verschachtelungstiefe!</span></div>");
  }

  if (renderWithEditChannelVisible == true) {
    renderEditChannel(client, renderAnchor, anchorChannelId, numChannels,
                      toggleOneBasedAddresses, channelIdToEdit);
  } else {
    renderOptions(client, numChannels, toggleOneBasedAddresses,
                  toggleCompactDisplay, toggleForceAllOff, toggleForceAllOn,
                  toggleRandom, channelIdToEdit);

    pn("<br>");

    pn("<h3>Übersicht der Kanäle</h3>");

    if (renderAnchor) {
      pt("<div id='navigateTo' data-anchor='#channel-");
      pt(anchorChannelId);
      pn("' "
         "style='display:none;'></div>");
    }

    pn("<div>");
    for (int channelId = 0; channelId < numChannels; channelId++) {
      if (toggleCompactDisplay) {
        renderChannelDetailCompact(client, toggleOneBasedAddresses, channelId);
      } else {
        renderChannelDetail(client, toggleOneBasedAddresses, channelId);
      }
    }
    pn("</div>");
  }

  pn("</form>");

  pn("</div></div></div>"
     "<script>"
     "document.addEventListener('DOMContentLoaded', function() {"
     "  var navigationTag = document.getElementById('navigateTo');"
     "  if (navigationTag) {"
     "    var anchor = navigationTag.getAttribute('data-anchor');"
     "    if (anchor) {"
     "      window.location.hash = anchor;"
     "    }"
     "  }"
     "});"
     "</script>"
     "</body></html>");
}
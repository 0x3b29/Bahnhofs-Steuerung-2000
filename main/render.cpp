#include "render.h"

char Renderer::m_checkedBuffer[] = "checked";
char Renderer::m_emptyBuffer[] = "";
char Renderer::m_renderHiddenBuffer[] = "style='display: none;'";
char Renderer::m_textMutedBuffer[] = "text-muted";

Renderer::Renderer(StateManager *stateManager) {
  this->m_stateManager = stateManager;
}

void Renderer::pn(WiFiClient client, char *buffer) {
  if (client.connected() == 0) {
    return;
  }

  client.flush();
  client.println(buffer);
}

void Renderer::renderHeadJavascript(WiFiClient client) {
  pn(client, R"html(
<script>
function openEditChannelPage(channel) {
  const data = {
    channel
  };
  const queryString = new URLSearchParams(data).toString();
  window.location.href = `update?${queryString}`
}

function sendValue(key, value, reloadAfterRequest) {
  var dataString =
    encodeURIComponent(key) + "=" + encodeURIComponent(value);

  fetch("/", {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: dataString,
  }).then((response) => {
    if (reloadAfterRequest || !response.ok  || response.status !== 200) {
      window.location.href = "/";
    }
  });
}
)html");

  pn(client, R"html(
function sendCheckbox(checkbox, reloadAfterRequest) {
  var dataString =
    encodeURIComponent(checkbox.name) +
    "=" +
    encodeURIComponent(checkbox.checked ? 1 : 0);

  if (checkbox.id === "toggleShowOptions") {
    var optionsDiv = document.getElementById("options");
    var optionsHeading = document.getElementById("options-heading");
    optionsDiv.style.display = checkbox.checked ? "block" : "none";
    if (checkbox.checked) {
      optionsHeading.classList.remove("text-muted");
      optionsHeading.classList.add("text-body");
    } else {
      optionsHeading.classList.remove("text-body");
      optionsHeading.classList.add("text-muted");
    }
  }

  if (checkbox.id === "toggleShowActions") {
    var actionsDiv = document.getElementById("actions");
    var actionsHeading = document.getElementById("actions-heading");
    actionsDiv.style.display = checkbox.checked ? "block" : "none";
    if (checkbox.checked) {
      actionsHeading.classList.remove("text-muted");
      actionsHeading.classList.add("text-body");
    } else {
      actionsHeading.classList.remove("text-body");
      actionsHeading.classList.add("text-muted");
    }
  }

  fetch("/", {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: dataString,
  }).then((response) => {
    if (reloadAfterRequest || !response.ok  || response.status !== 200) {
      window.location.href = "/";
    }
  });
}

function onBrightnessValueChanged(value, channelId) {
  var brightnessAsPercentage = Math.floor((value / 4095) * 100);

  var percentDisplay = document.getElementById("rangeAsPercentage");

  if (percentDisplay) 
  {
    percentDisplay.textContent =
    "(" + brightnessAsPercentage + "%)";
  }

  var dataString =
    "testBrightness=1&" +
    encodeURIComponent("channelBrightness") +
    "=" +
    encodeURIComponent(value) +
    "&" +
    encodeURIComponent("channelId") +
    "=" +
    encodeURIComponent(channelId);
  fetch("/", {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: dataString,
  }).then((response) => {
    if (!response.ok || response.status !== 200) {
      window.location.href = "/";
    }
  });
}
</script>
)html");
}

void Renderer::renderHeadCss(WiFiClient client) {
  pn(client, R"html(
<style>
  .mtba {
    margin-top: auto;
    margin-bottom: auto;
  }
</style>
)html");
}

void Renderer::renderWebPage(WiFiClient client, bool foundRecursion) {
  uint16_t numChannels = m_stateManager->getNumChannels();
  bool toggleCompactDisplay = m_stateManager->getToggleCompactDisplay();
  // Send a standard HTTP response header
  pn(client, "HTTP/1.1 200 OK");
  pn(client, "Content-type:text/html");
  pn(client, "Connection: close");
  pn(client, m_emptyBuffer);

  pn(client, R"html(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8" />
    <meta
      name="viewport"
      content="width=device-width, initial-scale=1"
    />
    <link
      href="https://cdn.jsdelivr.net/npm/bootstrap@5.2.3/dist/css/bootstrap.min.css"
      rel="stylesheet"
      integrity="sha384-rbsA2VBKQhggwzxH7pPCaAqO46MgnOM80zW1RWuH61DGLwZJEdK2Kadq2F9CUG65"
      crossorigin="anonymous"
    />
    <link
      href="https://fonts.googleapis.com/css2?family=Grape+Nuts&display=swap"
      rel="stylesheet"
    />
    <link rel="icon" href="data:;base64,iVBORw0KGgo=" />
  )html");

  renderHeadJavascript(client);

  if (m_stateManager->getRenderEditChannel() == true) {
    renderEditChannelJavascript(client);
  } else {
    renderOptionsJavascript(client);
  }

  renderHeadCss(client);

  pn(client, R"html(
</head>
<body style="min-height: 100vh">
  <div class="container" style="min-height: 93vh">
    <div class="row justify-content-center">
      <div class="col col-12 col-sm-10 col-md-8 col-lg-6">
        <div class='h1 mt-4 mb-3' style="font-family: 'Grape Nuts', bold; font-size: 195%;"> 
        Bahnhofs Steuerung 2000
      </div>
      <div>
)html");

  if (foundRecursion) {
    char renderRecursionWarningBuffer[512];
    snprintf(renderRecursionWarningBuffer, sizeof(renderRecursionWarningBuffer),
             R"html(
        <div class='pb-3'>
          <span class='text-danger'>
          %s %d %s
          </span>
        </div>
    )html",
             I18N_WARNING_RECURSION_BEFORE_MAX, MAX_RECURSION,
             I18N_WARNING_RECURSION_AFTER_MAX);

    pn(client, renderRecursionWarningBuffer);
  }

  if (m_stateManager->getRenderEditChannel() == true) {
    renderEditChannel(client);
  } else {
    renderOptionsHeading(client);
    renderOptions(client);
    renderActionsHeading(client);
    renderActions(client);

    pn(client, "<div>");
    if (toggleCompactDisplay) {
      for (int channelId = 0; channelId < numChannels; channelId++) {
        renderChannelDetailCompact(client, channelId);
      }
    } else {
      for (int channelId = 0; channelId < numChannels; channelId++) {
        bool renderHorizontalRule = true;

        if (channelId == numChannels - 1) {
          renderHorizontalRule = false;
        }

        renderChannelDetail(client, channelId, renderHorizontalRule);
      }
    }
    pn(client, "</div>");
  }

  pn(client, R"html(
          </div>
        </div>
      </div>
    </div>
  <div class='h6 mt-3 mb-3 d-flex justify-content-center' 
       style="font-family: 'Grape Nuts', bold; font-size: large;">
    made with ♥ by olivier.berlin
  </div>
</body>
</html>
)html");
}

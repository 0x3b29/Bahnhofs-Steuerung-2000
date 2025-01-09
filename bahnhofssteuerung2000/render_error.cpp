#include "render.h"

void Renderer::renderHttp413ErrorPage(WiFiClient client) {
  client.println("HTTP/1.1 413 Payload Too Large");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println(R"html(
    <html>
        <body>
            <h1>413 Payload Too Large</h1>
            <p>Your request is too large for this server.</p>
            <p><a href='/'>Return to Home</a></p>
        </body>
    </html>
  )html");
}

void Renderer::renderHttp400ErrorPage(WiFiClient client) {
  client.println("HTTP/1.1 400 Bad Request");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println(R"html(
    <html>
        <body>
            <h1>400 Bad Request</h1>
            <p>Invalid input parameters provided. The request did not contain the expected parameters</p>
            <p><a href='/'>Return to Home</a></p>
        </body>
    </html>
  )html");
}

void Renderer::renderHttp404ErrorPage(WiFiClient client) {
  client.println("HTTP/1.1 404 Not Found");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println(R"html(
    <html>
        <body>
            <h1>404 Not Found</h1>
            <p>The requested resource was not found on this server.</p>
            <p><a href='/'>Return to Home</a></p>
        </body>
    </html>
  )html");
}
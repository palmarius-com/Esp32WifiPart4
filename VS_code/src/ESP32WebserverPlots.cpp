// ---------------------------------------------------------------------------------------
//
// Demonstration of how to show plots on an ESP32 with a webserver (device used for tests: ESP32-WROOM-32D).
// The code emulates sensor data that is shown on a chart in the webserver and automatically updated through websockets.
//
// For installation, the following library needs to be installed under Sketch -> Include Library -> Manage Libraries:
// * ArduinoJson by Benoit Blanchon
//
// The following to libraries need to be downloaded, unpacked and copied to the "Arduino\libraries" folder
// (Required for ESPAsyncWebServer)
// https://github.com/me-no-dev/ESPAsyncWebServer
// https://github.com/me-no-dev/AsyncTCP
//
// Required to make SPIFFS.h work:
// https://github.com/me-no-dev/arduino-esp32fs-plugin/releases/
// see as well: https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
//
// 20230121 see this for VisualStudio Code:
// https://randomnerdtutorials.com/esp32-vs-code-platformio-spiffs/
//
// Favicon downloaded from: https://icons8.com/icons/set/favicon
//
// Written by mo thunderz (last update: 11.09.2022)
// ---------------------------------------------------------------------------------------

#include "declarations.h"
#include <Streaming.h>

// SSID and password of Wifi connection:
const char* ssid = "Arduino_ssid";
const char* password = "password";

// Configure IP addresses of the local access point
IPAddress local_IP(192,168,1,1);
IPAddress gateway(192,168,1,0);
IPAddress subnet(255,255,255,0);

// global variables of the LED selected and the intensity of that LED
int random_intensity = 5;

const int ARRAY_LENGTH = 10;
int sens_vals[ARRAY_LENGTH];

// We want to periodically send values to the clients, so we need to define an "interval" and remember the last time we sent data to the client (with "previousMillis")
int interval = 1000;                                  // send data to the client every 1000ms -> 1s
unsigned long previousMillis = 0;                     // we use the "millis()" command for time reference and this will output an unsigned long

// Initialization of webserver and websocket
AsyncWebServer server(80);                            // the server uses port 80 (standard port for websites
WebSocketsServer webSocket = WebSocketsServer(81);    // the websocket uses port 81 (standard port for websockets

void setup() {
  Serial.begin(115200);                               // init serial port for debugging

  if (!SPIFFS.begin()) Serial << "SPIFFS could not initialize" << endl;
  
  Serial << "Setting up Access Point ... " << (WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!") << endl;
  Serial << "Starting Access Point ...   " << (WiFi.softAP(ssid, password) ?                  "Ready" : "Failed!") << endl;
  Serial << "IP address = "                <<  WiFi.softAPIP() << endl;
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {    // define here wat the webserver needs to do
    request->send(SPIFFS, "/webpage.html", "text/html");           
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "File not found");
  });

  server.serveStatic("/", SPIFFS, "/");
  
  webSocket.begin();                                  // start websocket
  webSocket.onEvent(webSocketEvent);                  // define a callback function -> what does the ESP32 need to do when an event from the websocket is received? -> run function "webSocketEvent()"

  server.begin();                                     // start server -> best practise is to start the server after the websocket
}

void loop() {
  webSocket.loop();                                   // Update function for the webSockets 
  unsigned long now = millis();                       // read out the current "time" ("millis()" gives the time in ms since the Arduino started)
  if ((unsigned long)(now - previousMillis) > interval) { // check if "interval" ms has passed since last time the clients were updated
    previousMillis = now;                             // reset previousMillis

    for(int i=0; i < ARRAY_LENGTH - 1; i++) {
      sens_vals[i] = sens_vals[i+1];
    }
    sens_vals[ARRAY_LENGTH - 1] = random_intensity;

    sendJsonArray("graph_update", sens_vals);
  }
}

void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length) {      // the parameters of this callback function are always the same -> num: id of the client who send the event, type: type of message, payload: actual data sent and length: length of payload
  switch (type) {                                     // switch on the type of information sent
    case WStype_DISCONNECTED:                         // if a client is disconnected, then type == WStype_DISCONNECTED
      Serial << "Client " << String(num) << " disconnected" << endl;
      break;
    case WStype_CONNECTED:                            // if a client is connected, then type == WStype_CONNECTED
      Serial << "Client " << String(num) << " connected" << endl;
      
      // send variables to newly connected web client -> as optimization step one could send it just to the new client "num", but for simplicity I left that out here
      sendJson("random_intensity", String(random_intensity));
      sendJsonArray("graph_update", sens_vals);

      break;
    case WStype_TEXT:                                 // if a client has sent data, then type == WStype_TEXT
      // try to decipher the JSON string received
      StaticJsonDocument<200> doc;                    // create JSON container 
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial << F("deserializeJson() failed: ") << error.f_str() << endl;
        return;
      }
      else {
        // JSON string was received correctly, so information can be retrieved:
        const char* l_type = doc["type"];
        const int l_value = doc["value"];
        Serial << "Type:  " << String(l_type)  << endl;
        Serial << "Value: " << String(l_value) << endl;

        // if random_intensity value is received -> update and write to all web clients
        if(String(l_type) == "random_intensity") {
          random_intensity = int(l_value);
          sendJson("random_intensity", String(l_value));
        }
      }
      Serial << endl;
      break;
  }
}

// Simple function to send information to the web clients
void sendJson(String l_type, String l_value) {
    String jsonString = "";                           // create a JSON string for sending data to the client
    StaticJsonDocument<200> doc;                      // create JSON container
    JsonObject object = doc.to<JsonObject>();         // create a JSON Object
    object["type"] = l_type;                          // write data into the JSON object
    object["value"] = l_value;
    serializeJson(doc, jsonString);                   // convert JSON object to string
    webSocket.broadcastTXT(jsonString);               // send JSON string to all clients
}

// Simple function to send information to the web clients
void sendJsonArray(String l_type, int l_array_values[]) 
{
    String jsonString = "";                           // create a JSON string for sending data to the client
    const size_t CAPACITY = JSON_ARRAY_SIZE(ARRAY_LENGTH) + 100;
    StaticJsonDocument<CAPACITY> doc;                      // create JSON container
    
    JsonObject object = doc.to<JsonObject>();         // create a JSON Object
    object["type"] = l_type;                          // write data into the JSON object
    JsonArray value = object.createNestedArray("value");
    for(int i=0; i<ARRAY_LENGTH; i++) {
      value.add(l_array_values[i]);
    }
    serializeJson(doc, jsonString);                   // convert JSON object to string
    webSocket.broadcastTXT(jsonString);               // send JSON string to all clients
}

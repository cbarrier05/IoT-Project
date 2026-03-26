
// Include WiFi library for network connection
#include <WiFi.h>

// Include WebServer library to create a small web server
// running directly on the ESP32
#include <WebServer.h>

#include <ESPAsyncWebServer.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>



// ---------------- Wi-Fi credentials ----------------
// SSID and password of the wireless network.
// These must match the Wi-Fi network you want the ESP32 to connect to.
const char* ssid = "Christian's_S23";
const char* password = "Christian"; // CHANGE THIS TO YOUR WIFI PASSWORD

// ---------------- TMP36 Sensor Pin ----------------
// GPIO36 is an analog input pin on the ESP32
// where the TMP36 temperature sensor output is connected.
#define LED1_PIN 13
#define LED2_PIN 12
#define LED3_PIN 11

std::map<String, int> pin_map;
// Create a web server object listening on port 80
// Port 80 is the default HTTP port used by web browsers
AsyncWebServer server(80);

void toggle_pin(uint8_t pin) {
  digitalWrite(pin, !digitalRead(pin));
}

// ---------------------------------------------------
// Function: setup()es
// Runs once when the ESP32 starts
// ---------------------------------------------------
void setup()
{
  // Start serial communication for debugging
  Serial.begin(115200);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);

  pin_map["1"] = LED1_PIN;
  pin_map["2"] = LED2_PIN;
  pin_map["3"] = LED3_PIN;

  // Start connecting to WiFi
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  // Wait until connection is established
  while(WiFi.status()!=WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // Once connected, print IP address
  Serial.println(WiFi.localIP());


  // Serve the HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    // serves the contents of index.html
    String file_path = "index.html";
    std::ifstream file(file_path.c_str());
    std::stringstream buffer;

    buffer << file.rdbuf();
    String file_string = String(buffer.str().c_str());

    request->send(200, "text/html", file_string);
  });

  // Handle toggle requests
  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request) {
    String ledParam;
    if (request->hasParam("led")) {
      ledParam = request->getParam("led")->value();
      int pin_number = pin_map[ledParam];
      toggle_pin(pin_number);
      request->send(200, "text/plain", "OK");
    }

    request->send(400, "text/plain", "Bad Request"); 
  });

  // Start the web server
  server.begin();
}




// ---------------------------------------------------
// Function: loop()
// Runs continuously after setup()
// ---------------------------------------------------
void loop()
{
  // Handle incoming client requests
  // (e.g., browser requests for webpage or temperature)
}
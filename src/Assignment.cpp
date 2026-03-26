
// Include WiFi library for network connection
#include <WiFi.h>

// Include WebServer library to create a small web server
// running directly on the ESP32
#include <WebServer.h>

#include <ESPAsyncWebServer.h>



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



// Create a web server object listening on port 80
// Port 80 is the default HTTP port used by web browsers
AsyncWebServer server(80);





// ---------------------------------------------------
// Function: setup()
// Runs once when the ESP32 starts
// ---------------------------------------------------
void setup()
{
  // Start serial communication for debugging
  Serial.begin(115200);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);


  // Start connecting to WiFi
  WiFi.begin(ssid,password);

  Serial.print("Connecting to WiFi");

  // Wait until connection is established
  while(WiFi.status()!=WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // Once connected, print IP address
  Serial.println("");
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());


  // Serve the HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body><h1>LED Control</h1>";
    html += "<button onclick=\"toggleLED(1)\">Toggle LED 1</button><br><br>";
    html += "<button onclick=\"toggleLED(2)\">Toggle LED 2</button><br><br>";
    html += "<button onclick=\"toggleLED(3)\">Toggle LED 3</button><br><br>";
    html += "<script>";
    html += "function toggleLED(led) {";
    html += "  var xhr = new XMLHttpRequest();";
    html += "  xhr.open('GET', '/toggle?led=' + led, true);";
    html += "  xhr.send();";
    html += "}";
    html += "</script>";
    html += "</body></html>";
    
    request->send(200, "text/html", html);
  });

  // Handle toggle requests
  server.on("/toggle", HTTP_GET, [](AsyncWebServerRequest *request){
    String ledParam;
    if (request->hasParam("led")) {
      ledParam = request->getParam("led")->value();
    }

    if (ledParam == "1") {
      digitalWrite(LED1_PIN, !digitalRead(LED1_PIN)); // Toggle LED 1
    } else if (ledParam == "2") {
      digitalWrite(LED2_PIN, !digitalRead(LED2_PIN)); // Toggle LED 2
    } else if (ledParam == "3") {
      digitalWrite(LED3_PIN, !digitalRead(LED3_PIN)); // Toggle LED 3
    }

    request->send(200, "text/plain", "OK");
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
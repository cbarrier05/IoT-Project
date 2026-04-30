
#include <WiFi.h>
#include <HTTPClient.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

// Change to required WiFi network
const char* ssid = "Christian's_S23";
const char* password = "Christian";

// Change to required server address
const char* server_address = "http://192.168.134.40:5000";

#define TEMP_PIN 8
#define TEMP_LED1_PIN 5
#define TEMP_LED2_PIN 4
#define TEMP_LED3_PIN 3
#define LED1_PIN 13
#define LED2_PIN 12
#define LED3_PIN 11
#define LED4_PIN 10
#define LED5_PIN 9
#define LED6_PIN 6

#define ADC_MAX 4095.0
#define VREF 3.3


typedef struct {
  int pin_num;
  String colour;
  int state;
} PinInfo;

// The buffers for storing the pattern LED and temperature LED data
std::map<int, PinInfo > led_pin_map;
std::map<int, PinInfo> temp_pin_map;

bool custom_pattern[6 * 12];
int custom_frame_delay = 0;
int custom_frame_count = 0;

int frame;
int pattern;

long lastUpdate = 0;

float temperatureC;

std::map<String, int> temp_led_boundaries;

// How often the ESP32 sends the temperature data and requests LED data
const int update_period = 2000;

// Sets the LEDs to match the function argument
void setLeds(int new_states[6]) {
  for (int i = 0; i < 6; i++) {
    PinInfo current_pin = led_pin_map[i+1];
    // Only updates changed LEDs
    if (new_states[i] != current_pin.state) {
      digitalWrite(current_pin.pin_num, new_states[i]);
      current_pin.state = new_states[i];
      led_pin_map[i+1] = current_pin;
    }
  }
}

// Sets the temperature LEDs to match the function argument
void setTempLeds(int new_states[3]) {
  for (int i = 0; i < 3; i++) {
    PinInfo current_pin = temp_pin_map[i+1];
    // Only updates changed LEDs
    if (new_states[i] != current_pin.state) {
      digitalWrite(current_pin.pin_num, new_states[i]);
      current_pin.state = new_states[i];
      temp_pin_map[i+1] = current_pin;
    }
  }
}

void blinkPattern() {
  int first_state[6] = {1,1,1,1,1,1};
  setLeds(first_state);
  delay(300);
  int second_state[6] = {0,0,0,0,0,0};
  setLeds(second_state);
  delay(300);
}

void chasePattern() {
  for (int i = 0; i < 6; i++) {
      int new_state[6] = {0,0,0,0,0,0};
      new_state[i] = 1;
      setLeds(new_state);
      delay(100);
  }
}

void randomPattern() {
  int new_state[6] = {0,0,0,0,0,0};
  // For each LED, random chance of being on or off
  for (int i = 0; i < 6; i++) {
    if (random(0,2) == 0) {
      new_state[i] = 1;
    }
  }
  setLeds(new_state);
  delay(300);
}

void rainbowPattern() {
  String colours[] = {"green", "yellow", "red"};
  for (String colour : colours) {
    int new_state[6] = {0,0,0,0,0,0};
    for (int i = 0; i < 6; i++) {
      if (led_pin_map[i+1].colour == colour) {
        new_state[i] = 1;
      }
    }
    setLeds(new_state);
    delay(300);
  }
}

void getCustomPattern() {
  // Requests the custom LED pattern from the web server
  HTTPClient http;
  String url = String(server_address) + "/get_custom";
  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
    // Splits into [delay,data]
    int split = response.indexOf(',');

    if (split != -1) {
      custom_frame_delay = response.substring(0, split).toInt();
      String ledData = response.substring(split + 1, response.length());
      custom_frame_count = ledData.length() / 6;
      // Decodes python flask format back into bools
      for (int i = 0; i < ledData.length(); i++) {
        custom_pattern[i] = ledData[i] - '0';
      }
    }
  }
  http.end();
}

void setCustomPattern() {
  if (custom_frame_count == 0) return;
  // Splits the custom pattern data into separate frames
  for (int frame = 0; frame < custom_frame_count; frame++) {
    int new_states[6] = {0,0,0,0,0,0};
    for (int i = 0; i < 6; i++) {
      if (custom_pattern[frame * 6 + i]) {
       new_states[i] = 1;
      }
    }
    setLeds(new_states);
    delay(custom_frame_delay);
  }
}

void noPattern() {
  int new_state[6] = {0,0,0,0,0,0};
  setLeds(new_state);
}

void runPattern(int pattern) {
  switch (pattern) {
    case 0: noPattern(); break;
    case 1: blinkPattern(); break;
    case 2: chasePattern(); break;
    case 3: randomPattern(); break;
    case 4: rainbowPattern(); break;
    case 5: setCustomPattern(); break;
    default: break;
  }
}

float readTemperature() {
  // Translates the voltage output of the TMP36 to a celsius temperature
  int adcValue = analogRead(TEMP_PIN);
  float voltage = adcValue * VREF / ADC_MAX;
  float tempC = (voltage - 0.5) * 100.0;
  return tempC;
}

void getTempLedValues() {
  // Requests the temperature LED boundaries from the web server
  HTTPClient http;
  String url = String(server_address) + "/get_temp_leds";
  http.begin(url);
  int httpResponseCode = http.GET();

  // Updates the temperature LED boundaries with the received values
  if (httpResponseCode > 0) {
    String response = http.getString();
    int first_split = response.indexOf(',');
    temp_led_boundaries["Low"] = response.substring(0, first_split).toInt();
    String sub_string = response.substring(first_split + 1, response.length());
    int second_split = sub_string.indexOf(",");
    temp_led_boundaries["Med"] = sub_string.substring(0, second_split).toInt();
    temp_led_boundaries["High"] = sub_string.substring(second_split + 1, sub_string.length()).toInt();
  } 
}

void tempLedPattern() {
  // Treats the temperature LEDs as visual thermometer
  // Temperature boundaries set which LEDs are on/off
  int new_states[3] = {0,0,0};
  if (temperatureC > temp_led_boundaries["Low"]) {
    new_states[0] = 1;
  }
  if (temperatureC > temp_led_boundaries["Med"]) {
    new_states[1] = 1;
  }
  if (temperatureC > temp_led_boundaries["High"]) {
    new_states[2] = 1;
  }

  setTempLeds(new_states);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
  pinMode(LED5_PIN, OUTPUT);
  pinMode(LED6_PIN, OUTPUT);
  pinMode(TEMP_LED1_PIN, OUTPUT);
  pinMode(TEMP_LED2_PIN, OUTPUT);
  pinMode(TEMP_LED3_PIN, OUTPUT);

  // Intialising the LED buffers
  led_pin_map[1].pin_num = LED1_PIN;
  led_pin_map[1].colour = "green";
  led_pin_map[1].state = 0;

  led_pin_map[2].pin_num = LED2_PIN;
  led_pin_map[2].colour = "yellow";
  led_pin_map[2].state = 0;

  led_pin_map[3].pin_num = LED3_PIN;
  led_pin_map[3].colour = "red";
  led_pin_map[3].state = 0;

  led_pin_map[4].pin_num = LED4_PIN;
  led_pin_map[4].colour = "green";
  led_pin_map[4].state = 0;

  led_pin_map[5].pin_num = LED5_PIN;
  led_pin_map[5].colour = "yellow";
  led_pin_map[5].state = 0;

  led_pin_map[6].pin_num = LED6_PIN;
  led_pin_map[6].colour = "red";
  led_pin_map[6].state = 0;

  temp_pin_map[1].pin_num = TEMP_LED1_PIN;
  temp_pin_map[1].colour = "green";
  temp_pin_map[1].state = 0;

  temp_pin_map[2].pin_num = TEMP_LED2_PIN;
  temp_pin_map[2].colour = "yellow";
  temp_pin_map[2].state = 0;

  temp_pin_map[3].pin_num = TEMP_LED3_PIN;
  temp_pin_map[3].colour = "red";
  temp_pin_map[3].state = 0;

  // Temperature Sensor Tuning
  analogReadResolution(12);
  analogSetPinAttenuation(TEMP_PIN, ADC_11db);

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
  Serial.println("\nWiFi Connected");
  Serial.println(WiFi.localIP());

}


void loop() {
  // Sends/Receives update every time the update period passes
  if (millis() - lastUpdate >= update_period) {
    if (WiFi.status() == WL_CONNECTED) {
      // Sends the latest temperature reading to the web server
      // Receives the required LED pattern and whether the custom pattern or temperature boundaries have been updated
      HTTPClient http;
      temperatureC = readTemperature();
      String url = String(server_address) + "/update" + "?temp=" + temperatureC;
      http.begin(url);

      int httpResponseCode = http.GET();
      int temp_leds_updated;
      int custom_pattern_updated;

      if (httpResponseCode > 0) {
        String response = http.getString();
        pattern = response[0] - '0';
        temp_leds_updated = response[1] - '0';  
        custom_pattern_updated = response[2] - '0';
      }

      http.end();

      // Only requests the custom pattern and temperature boundary values if they have changed and are needed
      // Reduces unnecessary server requests 
      if ((pattern == 5) && (custom_pattern_updated == 1)) {
        getCustomPattern();
      }
      if (temp_leds_updated == 1) {
        getTempLedValues();
      }
    }
    lastUpdate = millis();
  }

  // Refreshes the LED patterns 
  runPattern(pattern);
  tempLedPattern();
}


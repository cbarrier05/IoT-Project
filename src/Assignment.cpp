
#include <WiFi.h>
#include <HTTPClient.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

// Update to the desired WiFi network
const char* ssid = "Christian's_S23";
const char* password = "Christian";

const char* server_address = "http://192.168.0.15:5000";

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
} Pin_info;

std::map<int, Pin_info > led_pin_map;
std::map<int, Pin_info> temp_pin_map;

bool custom_pattern[6 * 12];
int custom_frame_delay = 0;
int custom_frame_count = 0;

int frame;
int pattern;

long lastUpdate = 0;

float temperatureC;

const float low_temp = 15;
const float mid_temp = 19;
const float high_temp = 21;

const int update_period = 2000;

void set_leds(int new_states[6]) {
  for (int i = 0; i < 6; i++) {
    Pin_info current_pin = led_pin_map[i+1];
    if (new_states[i] != current_pin.state) {
      digitalWrite(current_pin.pin_num, new_states[i]);
      current_pin.state = new_states[i];
      led_pin_map[i+1] = current_pin;
    }
  }
}

void set_temp_leds(int new_states[3]) {
  for (int i = 0; i < 3; i++) {
    Pin_info current_pin = temp_pin_map[i+1];
    if (new_states[i] != current_pin.state) {
      digitalWrite(current_pin.pin_num, new_states[i]);
      current_pin.state = new_states[i];
      temp_pin_map[i+1] = current_pin;
    }
  }
}

void blink_pattern() {
  int first_state[6] = {1,1,1,1,1,1};
  set_leds(first_state);
  delay(300);
  int second_state[6] = {0,0,0,0,0,0};
  set_leds(second_state);
  delay(300);
}

void chase_pattern() {
  for (int i = 0; i < 6; i++) {
      int new_state[6] = {0,0,0,0,0,0};
      new_state[i] = 1;
      set_leds(new_state);
      delay(100);
  }
}

void random_pattern() {
  int new_state[6] = {0,0,0,0,0,0};
  for (int i = 0; i < 6; i++) {
    if (random(0,2) == 0) {
      new_state[i] = 1;
    }
  }
  set_leds(new_state);
  delay(300);
}

void rainbow_pattern() {
  String colours[] = {"green", "yellow", "red"};
  for (String colour : colours) {
    int new_state[6] = {0,0,0,0,0,0};
    for (int i = 0; i < 6; i++) {
      if (led_pin_map[i+1].colour == colour) {
        new_state[i] = 1;
      }
    }
    set_leds(new_state);
    delay(300);
  }
}

void get_custom_pattern() {
  HTTPClient http;
  String url = String(server_address) + "/get_custom";
  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String response = http.getString();
    // splits into [delay,data]
    int split = response.indexOf(',');

    if (split != -1) {
      custom_frame_delay = response.substring(0, split).toInt();
      String ledData = response.substring(split + 1, response.length());
      custom_frame_count = ledData.length() / 6;
      // decodes python flask format back into bools
      for (int i = 0; i < ledData.length(); i++) {
        custom_pattern[i] = ledData[i] - '0';
      }
    }
  }
  http.end();
}

void set_custom_pattern() {
  if (custom_frame_count == 0) return;

  for (int frame = 0; frame < custom_frame_count; frame++) {
    int new_states[6] = {0,0,0,0,0,0};
    for (int i = 0; i < 6; i++) {
      if (custom_pattern[frame * 6 + i]) {
       new_states[i] = 1;
      }
    }
    set_leds(new_states);
    delay(custom_frame_delay);
  }
}


void run_pattern(int pattern) {
  switch (pattern) {
    case 0: blink_pattern(); break;
    case 1: chase_pattern(); break;
    case 2: random_pattern(); break;
    case 3: rainbow_pattern(); break;
    case 4: set_custom_pattern(); break;
    default: break;
  }
}

float readTemperature() {
  int adcValue = analogRead(TEMP_PIN);
  float voltage = adcValue * VREF / ADC_MAX;
  float tempC = (voltage - 0.5) * 100.0;
  return tempC;
}

void temp_led_pattern() {
  int new_states[3] = {0,0,0};
  if (temperatureC > low_temp) {
    new_states[0] = 1;
  }
  if (temperatureC > mid_temp) {
    new_states[1] = 1;
  }
  if (temperatureC > high_temp) {
    new_states[2] = 1;
  }

  set_temp_leds(new_states);
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
  // Sends/Receives update every 2 seconds
  if (millis() - lastUpdate >= update_period) {
    if (WiFi.status() == WL_CONNECTED) {
      // Sends the latest temperature reading to the web server and receives the desired LED pattern
      HTTPClient http;
      temperatureC = readTemperature();
      String url = String(server_address) + "/update" + "?temp=" + temperatureC;
      http.begin(url);

      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        String response = http.getString();
        pattern = response.toInt();  
      }

      http.end();

      if (pattern == 4) {
        get_custom_pattern();
      }
    }
    lastUpdate = millis();
  }

  // Refreshes the LED patterns 
  run_pattern(pattern);
  temp_led_pattern();
}


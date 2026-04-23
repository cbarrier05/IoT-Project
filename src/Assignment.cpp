
#include <WiFi.h>
#include <HTTPClient.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>


const char* ssid = "Christian's_S23";
const char* password = "Christian";

const char* server_address = "http://192.168.0.15:5000/update";


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

std::map<int, int> led_pin_map;
std::map<int, int> temp_pin_map;

int pattern;

long lastUpdate = 0;

float temperatureC;

const float low_temp = 15;
const float mid_temp = 19;
const float high_temp = 21;

void reset_leds() {
  for (auto led : led_pin_map) {
    digitalWrite(led.second, LOW);
  }
}

void reset_temp_leds() {
  for (auto led : temp_pin_map) {
    digitalWrite(led.second, LOW);
  }
}

void toggle_pin(uint8_t pin) {
  digitalWrite(pin, !digitalRead(pin));
}

void blink_pattern() {
  for (int i = 0; i < 2; i++) {
    for (auto led : led_pin_map) {
      toggle_pin(led.second);
    }
    delay(300);
  }
}

void chase_pattern() {
  for (auto led : led_pin_map) {
      toggle_pin(led.second);
      delay(100);
      toggle_pin(led.second);
    }
}

void random_pattern() {
  reset_leds();
  for (auto led : led_pin_map) {
    if (random(0,2) == 0) {
      toggle_pin(led.second);
    }
  }
  delay(300);
}

void rainbow_pattern() {
  for (int i = 1; i <= 3; i++) {
    for (auto led : led_pin_map) {
      if ((led.first - i) % 3 == 0) {
        toggle_pin(led.second);
      }
    }
    delay(300);
    reset_leds();
  }
}

void run_pattern(int pattern) {
  switch (pattern) {
    case 0: blink_pattern(); break;
    case 1: chase_pattern(); break;
    case 2: random_pattern(); break;
    case 3: rainbow_pattern(); break;
  }
}

float readTemperature() {
  int adcValue = analogRead(TEMP_PIN);
  float voltage = adcValue * VREF / ADC_MAX;
  float tempC = (voltage - 0.5) * 100.0;
  return tempC;
}

void temp_led_pattern() {
  reset_temp_leds();
  if (temperatureC > low_temp) {
    toggle_pin(temp_pin_map[1]);
  }
  if (temperatureC > mid_temp) {
    toggle_pin(temp_pin_map[2]);
  }
  if (temperatureC > high_temp) {
    toggle_pin(temp_pin_map[3]);
  }
}

void setup() {
  // Start serial communication for debugging
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

  led_pin_map[1] = LED1_PIN;
  led_pin_map[2] = LED2_PIN;
  led_pin_map[3] = LED3_PIN;
  led_pin_map[4] = LED4_PIN;
  led_pin_map[5] = LED5_PIN;
  led_pin_map[6] = LED6_PIN;

  temp_pin_map[1] = TEMP_LED1_PIN;
  temp_pin_map[2] = TEMP_LED2_PIN;
  temp_pin_map[3] = TEMP_LED3_PIN;

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
  // 
  if (millis() - lastUpdate > 2000) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      temperatureC = readTemperature();
      String url = String(server_address) + "?temp=" + temperatureC;
      http.begin(url);

      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        String response = http.getString();
        pattern = response.toInt();  
      }

      http.end();
    }
    lastUpdate = millis();
  }
  run_pattern(pattern);
  temp_led_pattern();
}


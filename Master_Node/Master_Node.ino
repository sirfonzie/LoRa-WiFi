// MASTER
#include <WiFi.h>

//ESPNOW
#include <esp_now.h>
#include <PubSubClient.h>

// LoRa
#include <RadioLib.h>
#include "config/boards.h"

#include "lib/message.h"
#include "lib/lora_impl.h"
#include "lib/esp_now_impl.h"

/**
 * @brief Sets up the WiFi in AP_STA mode, prints the MAC address, and then disconnects.
 */
void setupWiFi() {
  WiFi.mode(WIFI_AP_STA);
  Serial.println("ESP-NOW Broadcast Demo");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  WiFi.disconnect();
}

/**
 * @brief Initializes ESP-NOW, registers a callback function for receiving data.
 * If initialization fails, the ESP is restarted.
 */
void initESPNow() {
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESP-NOW Init Success");
    esp_now_register_recv_cb(receiveCallback);
    // esp_now_register_send_cb(sentCallback);
  } else {
    Serial.println("ESP-NOW Init Failed");
    delay(3000);
    ESP.restart();
  }
}

/**
 * @brief Initializes the board and sets up LoRa.
 */
void initLoRa() {
  initBoard();
  setupLoRa();
}

/**
 * @brief Sets up the Serial Monitor, WiFi, ESP-NOW, and LoRa.
 * Also sets the initial time.
 */
void setup() {
  // Set up Serial Monitor
  Serial.begin(115200);
  delay(1000);

  // Set up WiFi
  setupWiFi();

  // Initialize ESP-NOW
  initESPNow();

  // Initialize LoRa
  initLoRa();

  prev_time = millis();
  Serial.println("Finish Setup");
}

/**
 * @brief Main loop function, runs the LoRa loop.
 */
void loop() {
  loRaLoop();
}
// ESP-NOW Mesh library for ESP32 / LoRa
// Author: Benjamin Loh Choon How (2201590)
// Co-Author: Woon JunWei (2200624)
// Date: 2/4/2024y

#include "ESPNowCommunication.h"
#include "ProtocolManager.h"
#include "MACaddr.h"

esp_now_peer_info_t peerInfo = {};
esp_now_peer_num_t peer_num;
uint8_t isConnectedToMaster = 0;
uint8_t numberOfHopsToMaster = 0;
SensorData sensorData;
Handshake msg;

// Function to add a peer node to the peer list
void addPeerToPeerList(const uint8_t *macAddr)
{
  char macStr[18];
  // Format the MAC address and store in sensor data struct
  formatMacAddress(macAddr, macStr, 18);

  Serial.printf("Received peer address: %s\n", macStr);

  memcpy(peerInfo.peer_addr, macAddr, 6); 
  peerInfo.channel = 0;                   
  peerInfo.encrypt = false;
  // If peer does not exist in the list, add it          
  if (!esp_now_is_peer_exist(macAddr))
  {
    esp_now_add_peer(&peerInfo);
    Serial.println("Added peer to the list");
  }
  else
  {
    Serial.println("Peer already exists in the list");
  }
}

// Function to send sensor data to all peer nodes in mesh network
void sendToAllPeers(const SensorData &sensorData)
{
  esp_now_get_peer_num(&peer_num);
  Serial.println("Sending....");
  for (int i = 0; i < peer_num.total_num; i++)
  {
    // Format the MAC address
    char macStr[18];
    formatMacAddress(peerInfo.peer_addr, macStr, 18);
    Serial.printf("Peer address to send: %s\n", macStr);
    // If peer exists in the list, send the sensor data
    if (esp_now_fetch_peer(1, &peerInfo) == ESP_OK)
    {
      esp_err_t result = esp_now_send(peerInfo.peer_addr, (const uint8_t *)&sensorData, sizeof(SensorData));

      // Print results to serial monitor when data sent successfully
      if (result == ESP_OK)
      {
        Serial.printf("Forwarded message to: %02X:%02X:%02X:%02X:%02X:%02X\n",
                      peerInfo.peer_addr[0], peerInfo.peer_addr[1], peerInfo.peer_addr[2],
                      peerInfo.peer_addr[3], peerInfo.peer_addr[4], peerInfo.peer_addr[5]);
      }
      else
      {
        Serial.println("Error sending message to peer");
      }
    }
    else
    {
      Serial.println("Peer not found");
    }
  }
}

// Function to handle received data
void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen)
{
  // Format the MAC address
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);

  // when sensor data received match the size of SensorData struct
  if (dataLen == sizeof(SensorData))
  {
    Serial.println("Sensor Data received");
    SensorData receivedData;
    memcpy(&receivedData, data, sizeof(SensorData));

    // Print the received sensor data
    Serial.printf("Received sensor data from: %s\n", macStr);
    // Serial.printf("Root Node Address: %d\n", receivedData.rootNodeAddress);
    Serial.printf("CO2 Data: %.2f\n", receivedData.c02Data);
    Serial.printf("Temperature Data: %.2f\n", receivedData.temperatureData);
    Serial.printf("Humidity Data: %.2f\n", receivedData.humidityData);

    sendToAllPeers(receivedData);
  }
  // when handshake message received match the size of Handshake struct
  else if (dataLen == sizeof(Handshake))
  {
    Serial.println("Handshake received");
    Handshake receivedMsg;
    memcpy(&receivedMsg, data, sizeof(Handshake));

    // handle requests
    if (receivedMsg.requestType == 0)
    {
      Serial.println("Request received");
      // Request type is 0, it's a request for connection status
      // Reply with the current connection status
      Handshake replyMsg;
      replyMsg.requestType = 1;
      replyMsg.isConnectedToMaster = isConnectedToMaster;
      replyMsg.numberOfHopsToMaster = numberOfHopsToMaster;

      memcpy(peerInfo.peer_addr, macAddr, 6);
      if (!esp_now_is_peer_exist(macAddr))
      {
        esp_now_add_peer(&peerInfo);
      }

      esp_err_t result = esp_now_send(macAddr, (const uint8_t *)&replyMsg, sizeof(Handshake));

      // Remove the broadcast address from the peer list
      esp_now_del_peer(macAddr);

      // Print results to serial monitor
      if (result == ESP_OK)
      {
        Serial.println("Reply message success");
      }
      else if (result == ESP_ERR_ESPNOW_NOT_INIT)
      {
        Serial.println("ESP-NOW not Init.");
      }
      else if (result == ESP_ERR_ESPNOW_ARG)
      {
        Serial.println("Invalid Argument");
      }
      else if (result == ESP_ERR_ESPNOW_INTERNAL)
      {
        Serial.println("Internal Error");
      }
      else if (result == ESP_ERR_ESPNOW_NO_MEM)
      {
        Serial.println("ESP_ERR_ESPNOW_NO_MEM");
      }
      else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
      {
        Serial.println("Peer not found.");
      }
      else
      {
        Serial.println("Unknown error");
      }
    }
    // handle replies from nearby nodes
    else if (receivedMsg.requestType == 1)
    {
      Serial.println("Reply received");
      // Reply type is 1, it's a reply containing connection status
      if (receivedMsg.isConnectedToMaster)
      {
        Serial.printf("Node %s is CONNECTED to the master\n", macStr);
        // Add this node to peer list
        addPeerToPeerList(macAddr);
        isConnectedToMaster = receivedMsg.isConnectedToMaster;
        numberOfHopsToMaster = receivedMsg.numberOfHopsToMaster + 1;

        Serial.printf("Hop Count: %d\n", numberOfHopsToMaster);
      }
      else
      {
        Serial.printf("Node %s is NOT CONNECTED to master\n", macStr);
      }
    }
  }
  else
  {
    Serial.println("Received data length does not match expected formats");
  }
}

// Function to handle sent data
void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Message sent successfully");
    failedMessageCount = 0; // Reset on success
    // failedMessageCount++; // Reset on success
  } else {
    Serial.println("Failed to send message");
    failedMessageCount++; // Increment on failure
  }
}

// Function to send a broadcast message for peer discovery
void broadcast(const Handshake &msg)
{
  // Broadcast address
  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  // Add broadcast address to peer list
  memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
  if (!esp_now_is_peer_exist(broadcastAddress))
  {
    esp_now_add_peer(&peerInfo);
  }

  // Send message
  esp_err_t result = esp_now_send(broadcastAddress, (const uint8_t *)&msg, sizeof(Handshake));

  // Remove the broadcast address from the peer list
  esp_now_del_peer(broadcastAddress);

  // Print results to serial monitor
  if (result == ESP_OK)
  {
    Serial.println("Broadcast message success");
  }
  else if (result == ESP_ERR_ESPNOW_NOT_INIT)
  {
    Serial.println("ESP-NOW not Init.");
  }
  else if (result == ESP_ERR_ESPNOW_ARG)
  {
    Serial.println("Invalid Argument");
  }
  else if (result == ESP_ERR_ESPNOW_INTERNAL)
  {
    Serial.println("Internal Error");
  }
  else if (result == ESP_ERR_ESPNOW_NO_MEM)
  {
    Serial.println("ESP_ERR_ESPNOW_NO_MEM");
  }
  else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
  {
    Serial.println("Peer not found.");
  }
  else
  {
    Serial.println("Unknown error");
  }
}

// Initialize ESP-NOW
void espnowSetup()
{
  if (esp_now_init() == ESP_OK)
  {
    Serial.println("ESP-NOW Init Success");
    esp_now_register_recv_cb(receiveCallback);
    esp_now_register_send_cb(sentCallback);
    // esp_now_register_send_cb(onDataSent);
  }
  else
  {
    Serial.println("ESP-NOW Init Failed");
    delay(3000);
    ESP.restart();
  }

  // Set up SCD4x sensor
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(13, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

   Wire.begin(I2C_SDA, I2C_SCL);
  
   uint16_t error;
   char errorMessage[256];
  
   scd4x.begin(Wire);
  
   // stop potentially previously started measurement
   error = scd4x.stopPeriodicMeasurement();
   if (error) {
     Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
     errorToString(error, errorMessage, 256);
     Serial.println(errorMessage);
   }
  
   uint16_t serial0;
   uint16_t serial1;
   uint16_t serial2;
   error = scd4x.getSerialNumber(serial0, serial1, serial2);
   if (error) {
     Serial.print("Error trying to execute getSerialNumber(): ");
     errorToString(error, errorMessage, 256);
     Serial.println(errorMessage);
   } else {
     printSerialNumber(serial0, serial1, serial2);
   }
  
   // Start Measurement
   error = scd4x.startPeriodicMeasurement();
   if (error) {
     Serial.print("Error trying to execute startPeriodicMeasurement(): ");
     errorToString(error, errorMessage, 256);
     Serial.println(errorMessage);
   }
}

// Uninitialize ESP-NOW
void espnowUninit()
{
  esp_now_deinit();
  esp_now_unregister_recv_cb();
  esp_now_unregister_send_cb();
}

// Polling loop for ESP-NOW communication
void espnowLoop()
{
  SensorData sensorData;

  // Read sensor data
  uint16_t error;
  char errorMessage[256];
  uint16_t co2 = 0;
  float temperature = 0.0f;
  float humidity = 0.0f;
  bool isDataReady = false;

  uint8_t macAddressBytes[6];
  parseMacAddress(WiFi.macAddress(), macAddressBytes);

  // Format the MAC address and store it in sensorData.MACaddr
  formatMacAddress(macAddressBytes, sensorData.MACaddr, MAX_MAC_LENGTH);

  error = scd4x.getDataReadyFlag(isDataReady);
  if (error)
  {
    Serial.print("Error trying to execute getDataReadyFlag(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
    // If error, use random data
    sensorData.c02Data = getRandomFloat(100.0, 1000.0);
    sensorData.temperatureData = getRandomFloat(0.0, 40.0);
    sensorData.humidityData = getRandomFloat(90.0, 1030.0);
  }
  else if (!isDataReady)
  {
    // If data not ready, return
    return;
  }
  else
  {
    // Read sensor measurement
    error = scd4x.readMeasurement(co2, temperature, humidity);
    if (error)
    {
      Serial.print("Error trying to execute readMeasurement(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
      // If error, use random data
      sensorData.c02Data = getRandomFloat(100.0, 1000.0);
      sensorData.temperatureData = getRandomFloat(0.0, 40.0);
      sensorData.humidityData = getRandomFloat(90.0, 1030.0);
    }
    else if (co2 == 0)
    {
      Serial.println("Invalid sample detected, skipping.");
      // If invalid sample, use random data
      sensorData.c02Data = getRandomFloat(100.0, 1000.0);
      sensorData.temperatureData = getRandomFloat(0.0, 40.0);
      sensorData.humidityData = getRandomFloat(90.0, 1030.0);
    }
    else
    {
      // Assign sensor data to sensorData structure
      sensorData.c02Data = co2;
      sensorData.temperatureData = temperature;
      sensorData.humidityData = humidity;
    }
  }

  // Handle ESP-NOW communication
  // uint8_t peer_num;
  esp_now_get_peer_num(&peer_num);
  Serial.printf("Peer num: %d\n", peer_num);

  if (peer_num.total_num == 0)
  {
    Serial.println("Commencing route discovery");
    // Set message header to request
    msg.requestType = 0;
    broadcast(msg);
  }
  else
  {
    Serial.println("Sending data to all peers");
    sendToAllPeers(sensorData);
  }

  delay(1000);
}
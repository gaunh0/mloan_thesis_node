/**
 * ESP32 Gateway - Nhận dữ liệu từ node và gửi lên MQTT
 * Hỗ trợ điều khiển polling interval của nodes
 */

#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include <map>

// Định nghĩa chân cho nút cấu hình
int pushButton = 23;

// Cấu hình MQTT
const char* mqtt_server = "";
const int mqtt_port = 1883;
const char* mqtt_client_id = "ESP32_Gateway";
const char* mqtt_username = "";
const char* mqtt_password = "";

// MQTT Topics
const char* mqtt_topic_data = "lora_sensor/data";
const char* mqtt_topic_get_now = "lora_sensor/get_now";
const char* mqtt_topic_set_polling = "lora_sensor/set_polling";
const char* mqtt_topic_last_will = "lora_sensor/last_will";

// Cấu hình LoRa E32
#define E32_TX 16
#define E32_RX 17
#define E32_M0 22
#define E32_M1 21

HardwareSerial E32Serial(2);

// Buffer và cấu trúc dữ liệu
#define MAX_PACKET_SIZE 128
#define MAX_NODE_ID_SIZE 16

struct SensorData {
  char nodeId[MAX_NODE_ID_SIZE];
  float temperature;
  float humidity;
  uint16_t batteryLevel;
};

// Cấu trúc lưu trữ config của từng node
struct NodeConfig {
  unsigned long pollingInterval;  // milliseconds
  bool hasUpdate;                // có config mới chưa gửi
  unsigned long lastSeen;        // thời gian nhận data cuối
};

// Variables
SensorData sensorData;
std::map<String, NodeConfig> nodeConfigs;
char receivedPacket[MAX_PACKET_SIZE];
bool newDataReceived = false;
bool getNowRequested = false;

// Button handling
unsigned long buttonPressedTime = 0;
bool buttonPressed = false;

// WiFi và MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Timing
unsigned long lastStatusUpdate = 0;
const long statusUpdateInterval = 60000;

void configModeCallback(WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

// MQTT callback - xử lý lệnh từ server
void callback(char* topic, byte* payload, unsigned int length) {
  // Convert payload to string
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(message);
  
  // Parse JSON message
  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, message);
  
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
  
  // Xử lý lệnh GET_NOW
  if (strcmp(topic, mqtt_topic_get_now) == 0) {
    if (doc["type"] == "GET_NOW") {
      Serial.println("GET_NOW command received");
      getNowRequested = true;
    }
  }
  
  // Xử lý lệnh SET_POLLING
  else if (strcmp(topic, mqtt_topic_set_polling) == 0) {
    if (doc.containsKey("nodeId") && doc.containsKey("polling")) {
      String nodeId = doc["nodeId"];
      String pollingStr = doc["polling"];
      
      Serial.printf("SET_POLLING for node %s: %s\n", nodeId.c_str(), pollingStr.c_str());
      
      unsigned long newInterval;
      if (pollingStr == "OFF") {
        newInterval = 0; // 0 = tắt gửi tự động
      } else {
        newInterval = pollingStr.toInt() * 1000; // convert to milliseconds
      }
      
      // Lưu config mới cho node
      nodeConfigs[nodeId].pollingInterval = newInterval;
      nodeConfigs[nodeId].hasUpdate = true;
      
      Serial.printf("Updated polling for %s to %lu ms\n", nodeId.c_str(), newInterval);
    }
  }
}

void reconnect() {
  int retry_count = 0;
  while (!client.connected() && retry_count < 5) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect(mqtt_client_id, mqtt_username, mqtt_password,
                        mqtt_topic_last_will, 1, true, "{\"type\":\"OFFLINE\"}")) {
      Serial.println("connected");
      
      // Publish online status
      client.publish(mqtt_topic_last_will, "{\"type\":\"ONLINE\"}", true);
      
      // Subscribe to command topics
      client.subscribe(mqtt_topic_get_now);
      client.subscribe(mqtt_topic_set_polling);
      
      Serial.println("Subscribed to command topics");
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
      retry_count++;
    }
  }
}

void displayData() {
  Serial.println("==== New sensor data ====");
  Serial.printf("Node ID: %s\n", sensorData.nodeId);
  Serial.printf("Temperature: %.2f °C\n", sensorData.temperature);
  Serial.printf("Humidity: %.2f %%\n", sensorData.humidity);
  
  float batteryVoltage = (sensorData.batteryLevel * 5.0) / 1023.0;
  Serial.printf("Battery: %.2f V\n", batteryVoltage);
  Serial.println("========================");
}

void publishToMQTT() {
  DynamicJsonDocument doc(256);
  
  doc["nodeId"] = sensorData.nodeId;
  doc["temperature"] = sensorData.temperature;
  doc["humidity"] = sensorData.humidity;
  doc["batteryVoltage"] = (sensorData.batteryLevel * 5.0) / 1023.0;
  
  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);
  
  if (client.publish(mqtt_topic_data, jsonBuffer)) {
    Serial.println("Data published to MQTT successfully");
  } else {
    Serial.println("Failed to publish data to MQTT");
  }
  
  // Cập nhật thời gian nhận data cuối của node
  String nodeId = String(sensorData.nodeId);
  nodeConfigs[nodeId].lastSeen = millis();
}

// Gửi ACK và config về node - Simple format cho Arduino
void sendAckToNode(const char* nodeId) {
  String nodeIdStr = String(nodeId);
  
  // Tạo simple ACK format: ACK,NODE001,polling:30,getNow:1
  String ackMsg = "ACK," + String(nodeId);
  
  // Thêm GET_NOW request nếu có
  if (getNowRequested) {
    ackMsg += ",getNow:1";
    getNowRequested = false;
    Serial.println("Adding GET_NOW to ACK");
  }
  
  // Thêm config polling nếu có update
  if (nodeConfigs[nodeIdStr].hasUpdate) {
    unsigned long interval = nodeConfigs[nodeIdStr].pollingInterval;
    if (interval == 0) {
      ackMsg += ",polling:OFF";
    } else {
      ackMsg += ",polling:" + String(interval / 1000);
    }
    nodeConfigs[nodeIdStr].hasUpdate = false;
    Serial.printf("Adding polling config to ACK: %lu\n", interval);
  }
  
  Serial.printf("Sending ACK to %s: %s\n", nodeId, ackMsg.c_str());
  E32Serial.println(ackMsg);
  
  delay(100); // Đảm bảo dữ liệu được gửi
}

void checkConfigButton() {
  bool currentButtonState = digitalRead(pushButton) == HIGH;
  
  if (currentButtonState && !buttonPressed) {
    buttonPressed = true;
    buttonPressedTime = millis();
    Serial.println("Button pressed, starting timer");
  }
  
  if (!currentButtonState && buttonPressed) {
    buttonPressed = false;
    unsigned long pressDuration = millis() - buttonPressedTime;
    
    if (pressDuration >= 1000 && pressDuration < 5000) {
      Serial.println("Button pressed for config mode");
      WiFiManager wm;
      wm.setConfigPortalTimeout(180);
      wm.startConfigPortal("ESP32-Gateway");
    }
    else if (pressDuration >= 5000) {
      Serial.println("Button long-pressed, resetting WiFi settings");
      WiFiManager wm;
      wm.resetSettings();
      Serial.println("Restarting...");
      delay(1000);
      ESP.restart();
    }
  }
}

void setup() {
  Serial.begin(9600);
  
  pinMode(pushButton, INPUT);
  
  // Khởi tạo LoRa E32
  E32Serial.begin(9600, SERIAL_8N1, E32_RX, E32_TX);
  pinMode(E32_M0, OUTPUT);
  pinMode(E32_M1, OUTPUT);
  setE32Mode(0, 0); // Normal mode
  
  Serial.println("Initializing ESP32 Gateway...");
  delay(1000);
  
  // WiFi setup
  WiFiManager wm;
  wm.setAPCallback(configModeCallback);
  
  bool res = wm.autoConnect("ESP32-Gateway", "password");
  if (!res) {
    Serial.println("Failed to connect WiFi");
  } else {
    Serial.println("Connected to WiFi");
  }
  
  // MQTT setup
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  Serial.println("Gateway ready. Waiting for data...");
}

void loop() {
  checkConfigButton();
  
  // Maintain MQTT connection
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Đọc dữ liệu từ LoRa
  static char buffer[MAX_PACKET_SIZE];
  static int bufferIndex = 0;
  static unsigned long lastCharTime = 0;
  
  while (E32Serial.available()) {
    char c = E32Serial.read();
    
    if (bufferIndex < MAX_PACKET_SIZE - 1) {
      buffer[bufferIndex++] = c;
    }
    
    lastCharTime = millis();
  }
  
  // Process received data
  if (bufferIndex > 0 && (millis() - lastCharTime > 50)) {
    buffer[bufferIndex] = '\0';
    
    Serial.print("Received: ");
    Serial.println(buffer);
    
    if (parseData(buffer)) {
      displayData();
      publishToMQTT();
      
      // Gửi ACK + config về node
      sendAckToNode(sensorData.nodeId);
      
      newDataReceived = true;
    }
    
    bufferIndex = 0;
  }
  
  // Periodic status update
  unsigned long currentMillis = millis();
  if (currentMillis - lastStatusUpdate > statusUpdateInterval) {
    lastStatusUpdate = currentMillis;
    client.publish(mqtt_topic_last_will, "{\"type\":\"ONLINE\"}", true);
    Serial.println("Status updated: online");
  }
  
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    WiFiManager wm;
    wm.setConfigPortalTimeout(60);
    wm.autoConnect();
  }
}

void setE32Mode(int m0, int m1) {
  digitalWrite(E32_M0, m0);
  digitalWrite(E32_M1, m1);
  delay(100);
}

bool parseData(const char* data) {
  // Parse format: ID:NODE001,T:24.18,H:57.94,B:308
  
  char *idPos = strstr(data, "ID:");
  char *tempPos = strstr(data, "T:");
  char *humPos = strstr(data, "H:");
  char *battPos = strstr(data, "B:");
  
  if (!idPos || !tempPos || !humPos || !battPos) {
    Serial.println("Invalid data format");
    return false;
  }
  
  // Extract node ID
  char nodeId[MAX_NODE_ID_SIZE];
  int i = 0;
  idPos += 3;
  while (*idPos != ',' && i < MAX_NODE_ID_SIZE-1) {
    nodeId[i++] = *idPos++;
  }
  nodeId[i] = '\0';
  
  // Extract temperature
  char temp[10];
  i = 0;
  tempPos += 2;
  while (*tempPos != ',' && i < 9) {
    temp[i++] = *tempPos++;
  }
  temp[i] = '\0';
  
  // Extract humidity
  char humidity[10];
  i = 0;
  humPos += 2;
  while (*humPos != ',' && i < 9) {
    humidity[i++] = *humPos++;
  }
  humidity[i] = '\0';
  
  // Extract battery
  uint16_t batteryValue = 0;
  sscanf(battPos, "B:%d", &batteryValue);
  
  // Save to structure
  strncpy(sensorData.nodeId, nodeId, MAX_NODE_ID_SIZE);
  sensorData.temperature = atof(temp);
  sensorData.humidity = atof(humidity);
  sensorData.batteryLevel = batteryValue;
  
  return true;
}
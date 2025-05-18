/**
 * Chương trình ESP32 nhận dữ liệu từ node Arduino qua LoRa E32
 * và gửi lên MQTT broker
 * Sử dụng WiFiManager để cấu hình WiFi
 */

#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>

// Định nghĩa chân cho nút cấu hình
int pushButton = 23;

// Cấu hình MQTT - Thông tin cố định
const char* mqtt_server = "dev.combros.tech";  // Địa chỉ MQTT broker
const int mqtt_port = 1883;                    // Port MQTT không SSL
const char* mqtt_client_id = "ESP32_Client";   // Client ID
// Thông tin xác thực MQTT (cố định)
const char* mqtt_username = "combros";                // Thay đổi nếu broker yêu cầu
const char* mqtt_password = "combros";                // Thay đổi nếu broker yêu cầu

// Cấu hình topic MQTT
const char* mqtt_topic_sensor = "sensor/data";

// Cấu hình Last Will
const char* lwt_topic = "sensor/status";
const char* lwt_message = "offline";
const int lwt_qos = 1;
const bool lwt_retain = true;
const char* status_online = "online";

// Cấu hình chân cho module LoRa E32
#define E32_TX 16 // Kết nối với RX của E32
#define E32_RX 17 // Kết nối với TX của E32
#define E32_M0 22 // M0 của E32
#define E32_M1 21 // M1 của E32

// Sử dụng hardware serial 2 trên ESP32 cho E32
HardwareSerial E32Serial(2); // UART2

// Kích thước tối đa của buffer chuỗi
#define MAX_PACKET_SIZE 128
#define MAX_NODE_ID_SIZE 16

// Cấu trúc để lưu trữ dữ liệu phân tích
struct SensorData {
  char nodeId[MAX_NODE_ID_SIZE];
  float temperature;
  float humidity;
  uint16_t batteryLevel;
  uint32_t timestamp;
};

// Biến toàn cục
SensorData sensorData;
char receivedPacket[MAX_PACKET_SIZE];
unsigned long lastReceiveTime = 0;
bool newDataReceived = false;

// Biến theo dõi trạng thái nút nhấn
unsigned long buttonPressedTime = 0;
bool buttonPressed = false;

// Khởi tạo đối tượng WiFi và MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

// Biến thời gian
unsigned long lastStatusUpdate = 0;
const long statusUpdateInterval = 60000; // 60 giây

// Callback khi vào chế độ cấu hình
void configModeCallback(WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

// Callback function khi nhận thông điệp từ MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Hàm kết nối lại MQTT
void reconnect() {
  int retry_count = 0;
  // Vòng lặp cho tới khi kết nối được
  while (!client.connected() && retry_count < 5) {
    Serial.print("Attempting MQTT connection to ");
    Serial.print(mqtt_server);
    Serial.print(":");
    Serial.print(mqtt_port);
    Serial.print("...");
    
    // Thử kết nối với Last Will message
    if (client.connect(mqtt_client_id, mqtt_username, mqtt_password,
                        lwt_topic, lwt_qos, lwt_retain, lwt_message)) {
      Serial.println("connected");
      
      // Sau khi kết nối thành công, publish trạng thái online
      client.publish(lwt_topic, status_online, lwt_retain);
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      
      // Giải thích lỗi
      switch(client.state()) {
        case -4: Serial.println("Connection timeout"); break;
        case -3: Serial.println("Connection lost"); break;
        case -2: Serial.println("Connection failed - broker unreachable"); break;
        case -1: Serial.println("Connection failed - broker rejected client"); break;
        default: Serial.println("Unknown error");
      }
      
      // Đợi 5 giây trước khi thử lại
      delay(5000);
      retry_count++;
    }
  }
}

// Hiển thị dữ liệu đã phân tích
void displayData() {
  Serial.println("==== Dữ liệu mới từ node cảm biến ====");
  
  Serial.print("Node ID: ");
  Serial.println(sensorData.nodeId);
  
  Serial.print("Temperature: ");
  Serial.print(sensorData.temperature, 2);
  Serial.println(" °C");
  
  Serial.print("Humidity: ");
  Serial.print(sensorData.humidity, 2);
  Serial.println(" %");
  
  Serial.print("Battery: ");
  // Chuyển đổi giá trị ADC thành điện áp (giả định)
  float batteryVoltage = (sensorData.batteryLevel * 5.0) / 1023.0;
  Serial.print(batteryVoltage, 2);
  Serial.println(" V");
  
  Serial.print("Timestamp from node: ");
  Serial.print(sensorData.timestamp);
  Serial.println(" ms");
  
  Serial.print("Time since last reception: ");
  static unsigned long previousTimestamp = 0;
  if (previousTimestamp > 0) {
    Serial.print(sensorData.timestamp - previousTimestamp);
    Serial.println(" ms");
  } else {
    Serial.println("N/A");
  }
  previousTimestamp = sensorData.timestamp;
  
  Serial.println("=======================================");
}

// Gửi dữ liệu lên MQTT broker
void publishToMQTT() {
  // Tạo JSON document
  DynamicJsonDocument doc(256);
  
  doc["nodeId"] = sensorData.nodeId;
  doc["temperature"] = sensorData.temperature;
  doc["humidity"] = sensorData.humidity;
  doc["batteryVoltage"] = (sensorData.batteryLevel * 5.0) / 1023.0;
  doc["timestamp"] = sensorData.timestamp;
  
  // Serialize JSON to string
  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);
  
  // Publish to MQTT
  if (client.publish(mqtt_topic_sensor, jsonBuffer)) {
    Serial.println("Message published to MQTT successfully");
  } else {
    Serial.println("Failed to publish message to MQTT");
  }
}

// Kiểm tra nút nhấn cấu hình
void checkConfigButton() {
  // Đọc trạng thái nút
  bool currentButtonState = digitalRead(pushButton) == HIGH;
  
  // Nếu nút được nhấn và trước đó chưa nhấn
  if (currentButtonState && !buttonPressed) {
    buttonPressed = true;
    buttonPressedTime = millis();
    Serial.println("Button pressed, starting timer");

  }
  
  // Nếu nút được nhả ra
  if (!currentButtonState && buttonPressed) {
    buttonPressed = false;
    // Nếu thời gian nhấn từ 1-5 giây, vào chế độ cấu hình
    if (millis() - buttonPressedTime >= 1000 && millis() - buttonPressedTime < 5000) {
      Serial.println("Button pressed for config mode");
      WiFiManager wm;
      wm.setConfigPortalTimeout(180); // Timeout sau 3 phút
      wm.startConfigPortal("ESP32-Setup");
    }
    // Nếu nút nhấn giữ quá 5 giây, xóa cấu hình và khởi động lại
    else if (millis() - buttonPressedTime >= 5000) {
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
  
  // Cấu hình nút nhấn
  pinMode(pushButton, INPUT);
  
  // Khởi tạo UART2 để giao tiếp với E32
  E32Serial.begin(9600, SERIAL_8N1, E32_RX, E32_TX);
  
  // Thiết lập chân điều khiển E32
  pinMode(E32_M0, OUTPUT);
  pinMode(E32_M1, OUTPUT);
  
  // Thiết lập chế độ hoạt động bình thường cho E32
  setE32Mode(0, 0); // M0=0, M1=0: Normal mode
  
  Serial.println("Initializing ESP32 receiving node...");
  delay(1000); // Đợi E32 khởi động hoàn toàn
  
  // Khởi tạo WiFiManager
  WiFiManager wm;

  // Đặt callback
  wm.setAPCallback(configModeCallback);
  
  // Kết nối đến WiFi hoặc khởi động cấu hình AP
  bool res;
  
  res = wm.autoConnect("ESP32-Setup", "password"); // AP name và password

  if(!res) {
    Serial.println("Failed to connect or hit timeout");
    // Có thể thêm xử lý hoặc khởi động lại ESP
    // ESP.restart();
  } 
  else {
    // Kết nối WiFi thành công
    Serial.println("Connected to WiFi");
  }
  
  // Cấu hình MQTT client
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  Serial.println("The system is ready. Waiting for data...");
  
  // Debug DNS resolution
  Serial.println("Resolving MQTT server domain...");
  IPAddress mqtt_ip;
  if (WiFi.hostByName(mqtt_server, mqtt_ip)) {
    Serial.print("MQTT server IP: ");
    Serial.println(mqtt_ip.toString());
  } else {
    Serial.println("Failed to resolve MQTT server domain");
  }
}

void loop() {
  // Kiểm tra và xử lý nút nhấn cấu hình
  checkConfigButton();
  
  // Kiểm tra và duy trì kết nối MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Biến lưu chuỗi nhận được
  static char buffer[MAX_PACKET_SIZE];
  static int bufferIndex = 0;
  static unsigned long lastCharTime = 0;
  
  // Đọc dữ liệu từ E32
  while (E32Serial.available()) {
    char c = E32Serial.read();
    
    // Nếu nhận được ký tự kết thúc hoặc buffer đầy, xử lý chuỗi
    if (bufferIndex < MAX_PACKET_SIZE - 1) {
      buffer[bufferIndex++] = c;
    }
    
    lastCharTime = millis();
  }
  
  // Nếu đã nhận được dữ liệu và đã qua một khoảng thời gian không nhận ký tự mới
  // Giả sử các gói tin cách nhau ít nhất 50ms
  if (bufferIndex > 0 && (millis() - lastCharTime > 50)) {
    // Thêm null terminator
    buffer[bufferIndex] = '\0';
    
    // Hiển thị chuỗi thô nhận được để debug
    Serial.print("Received raw string: ");
    Serial.println(buffer);
    
    // Phân tích chuỗi nhận được
    if (parseData(buffer)) {
      // Hiển thị dữ liệu đã phân tích
      displayData();
      
      // Cập nhật thời gian nhận và đánh dấu dữ liệu mới
      lastReceiveTime = millis();
      newDataReceived = true;
    }
    
    // Reset buffer để nhận gói tin mới
    bufferIndex = 0;
  }
  
  // Xử lý dữ liệu mới nhận được
  if (newDataReceived) {
    // Gửi dữ liệu lên MQTT broker
    publishToMQTT();
    
    newDataReceived = false;
  }
  
  // Cập nhật trạng thái online định kỳ
  unsigned long currentMillis = millis();
  if (currentMillis - lastStatusUpdate > statusUpdateInterval) {
    lastStatusUpdate = currentMillis;
    
    // Publish trạng thái online
    client.publish(lwt_topic, status_online, lwt_retain);
    Serial.println("Status updated: online");
  }
  
  // Kiểm tra mất kết nối với node cảm biến
  if (millis() - lastReceiveTime > 60000 && lastReceiveTime > 0) {
    Serial.println("Warning: No data received for 60 seconds!");
    // Thực hiện các hành động cần thiết khi mất kết nối
  }
  
  // Kiểm tra kết nối WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    // WiFi.reconnect();
    
    // Hoặc sử dụng WiFiManager để kết nối lại
    WiFiManager wm;
    wm.setConfigPortalTimeout(60);
    wm.autoConnect();
  }
}

// Thiết lập chế độ cho module E32
void setE32Mode(int m0, int m1) {
  digitalWrite(E32_M0, m0);
  digitalWrite(E32_M1, m1);
  delay(100); // Đợi E32 chuyển chế độ
}

// Phân tích chuỗi dữ liệu nhận được
bool parseData(const char* data) {
  // Format chuỗi mới: ID:NODE001,T:24.18,H:57.94,B:308,TS:1140000
  
  // Tạo các chuỗi tạm để lưu trữ giá trị
  char nodeId[MAX_NODE_ID_SIZE], temp[10], humidity[10];
  
  // Khởi tạo các biến để nhận giá trị phân tích
  uint16_t batteryValue = 0;
  uint32_t timestampValue = 0;
  
  // Tìm các thành phần trong chuỗi
  char *idPos = strstr(data, "ID:");
  char *tempPos = strstr(data, "T:");
  char *humPos = strstr(data, "H:");
  char *battPos = strstr(data, "B:");
  char *tsPos = strstr(data, "TS:");
  
  if (!idPos || !tempPos || !humPos || !battPos || !tsPos) {
    Serial.println("Missing one or more components in the string.");
    return false;
  }
  
  // Trích xuất ID node
  int i = 0;
  idPos += 3; // Bỏ qua "ID:"
  while (*idPos != ',' && i < MAX_NODE_ID_SIZE-1) {
    nodeId[i++] = *idPos++;
  }
  nodeId[i] = '\0';
  
  // Trích xuất nhiệt độ
  i = 0;
  tempPos += 2; // Bỏ qua "T:"
  while (*tempPos != ',' && i < 9) {
    temp[i++] = *tempPos++;
  }
  temp[i] = '\0';
  
  // Trích xuất độ ẩm
  i = 0;
  humPos += 2; // Bỏ qua "H:"
  while (*humPos != ',' && i < 9) {
    humidity[i++] = *humPos++;
  }
  humidity[i] = '\0';
  
  // Trích xuất mức pin
  sscanf(battPos, "B:%d", &batteryValue);
  
  // Trích xuất timestamp
  sscanf(tsPos, "TS:%lu", &timestampValue);
  
  // Chuyển đổi chuỗi thành số và lưu vào cấu trúc dữ liệu
  strncpy(sensorData.nodeId, nodeId, MAX_NODE_ID_SIZE);
  sensorData.temperature = atof(temp);
  sensorData.humidity = atof(humidity);
  sensorData.batteryLevel = batteryValue;
  sensorData.timestamp = timestampValue;
  
  // In ra thông tin đã phân tích để kiểm tra
  Serial.println("Successfully parsed:");
  Serial.print("Node ID: "); Serial.println(nodeId);
  Serial.print("Temperature: "); Serial.println(temp);
  Serial.print("Humidity: "); Serial.println(humidity);
  Serial.print("Battery: "); Serial.println(batteryValue);
  Serial.print("Timestamp: "); Serial.println(timestampValue);
  
  return true;
}
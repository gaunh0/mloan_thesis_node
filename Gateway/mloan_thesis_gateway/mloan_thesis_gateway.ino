/**
 * Chương trình ESP32 nhận dữ liệu từ node Arduino qua LoRa E32
 * Node nhận: ESP32 + LoRa E32
 */


 
#include <HardwareSerial.h>

// Cấu hình chân cho module LoRa E32
#define E32_TX 16 // Kết nối với RX của E32
#define E32_RX 17 // Kết nối với TX của E32
#define E32_M0 22 // M0 của E32
#define E32_M1 23 // M1 của E32
// Không sử dụng chân AUX

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

void setup() {
  Serial.begin(9600);  // Serial để hiển thị debug
  
  // Khởi tạo UART2 để giao tiếp với E32
  E32Serial.begin(9600, SERIAL_8N1, E32_RX, E32_TX);
  
  // Thiết lập chân điều khiển E32
  pinMode(E32_M0, OUTPUT);
  pinMode(E32_M1, OUTPUT);
  
  // Thiết lập chế độ hoạt động bình thường cho E32
  setE32Mode(0, 0); // M0=0, M1=0: Normal mode
  
  Serial.println("Initializing ESP32 receiving node...");
  delay(1000); // Đợi E32 khởi động hoàn toàn
  
  Serial.println("The system is ready. Waiting for data...");
}

void loop() {
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
    newDataReceived = false;
    
    // Các hành động bổ sung có thể được thực hiện ở đây
    // Ví dụ: gửi dữ liệu lên cloud, lưu vào SD, hiển thị lên LCD
    
    // Phản hồi lại node gửi (nếu cần)
    // sendResponse();
  }
  
  // Kiểm tra mất kết nối
  if (millis() - lastReceiveTime > 60000 && lastReceiveTime > 0) {
    Serial.println("Warning: No data received for 60 seconds!");
    // Thực hiện các hành động cần thiết khi mất kết nối
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

// Gửi phản hồi (tùy chọn)
void sendResponse() {
  // Chuỗi phản hồi đơn giản
  const char* responseMessage = "OK";
  
  // Gửi phản hồi
  E32Serial.print(responseMessage);
  
  // Thêm độ trễ để đảm bảo E32 có đủ thời gian gửi dữ liệu
  delay(100);
  
  Serial.println("Response sent...");
}
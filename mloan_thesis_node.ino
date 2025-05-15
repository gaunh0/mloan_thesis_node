/**
 * Chương trình Arduino đọc dữ liệu từ cảm biến SHT30 và truyền qua LoRa E32
 * Node truyền: Arduino + SHT30 + LoRa E32
 */

#include <Wire.h>
#include <SoftwareSerial.h>

// Địa chỉ I2C mặc định của SHT30 là 0x44
#define SHT30_ADDRESS 0x44

// Command để đo nhiệt độ và độ ẩm với độ phân giải cao (0x2C06)
#define SHT30_MEASURE_HIGH 0x2C
#define SHT30_MEASURE_HIGH_REP 0x06

// Cấu hình chân cho module LoRa E32
#define E32_M0 6 // M0 của E32
#define E32_M1 7 // M1 của E32
// Không sử dụng chân AUX

// Khởi tạo SoftwareSerial cho giao tiếp với E32
SoftwareSerial E32Serial(11, 10); // RX, TX

// Cấu trúc dữ liệu gửi
#define MAX_PACKET_SIZE 128

// Biến toàn cục
char dataPacket[MAX_PACKET_SIZE]; // Buffer để lưu chuỗi dữ liệu
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 10000; // Gửi dữ liệu mỗi 10 giây

void setup() {
  Serial.begin(9600);  // Khởi tạo Serial để debug
  E32Serial.begin(9600); // Khởi tạo giao tiếp với E32, tốc độ mặc định là 9600
  Wire.begin();        // Khởi tạo I2C
  
  // Thiết lập chân điều khiển E32
  pinMode(E32_M0, OUTPUT);
  pinMode(E32_M1, OUTPUT);
  
  // Thiết lập chế độ hoạt động bình thường cho E32
  setE32Mode(HIGH, HIGH); // M0=0, M1=0: Normal mode
  
  Serial.println("Initializing the system...");

  
  // Kiểm tra kết nối với cảm biến SHT30
  if (isConnected()) {
    Serial.println("The SHT30 sensor has been successfully connected!");
  } else {
    Serial.println("Cannot find the SHT30 sensor. Please check the connection!");
    while (1); // Dừng chương trình nếu không tìm thấy cảm biến
  }

  delay(1000); // Đợi E32 khởi động hoàn toàn
  Serial.println("The system is ready. Starting to read and send data.");
}

void loop() {
  // Đọc dữ liệu từ cảm biến theo chu kỳ
  unsigned long currentTime = millis();
  
  if (currentTime - lastSendTime >= sendInterval) {
    lastSendTime = currentTime;
    
    // Đọc dữ liệu từ cảm biến SHT30
    float temperature, humidity;
    
    if (readSHT30(&temperature, &humidity)) {
      // Hiển thị dữ liệu trên Serial để debug
      Serial.print("Temperature: ");
      Serial.print(temperature, 2);
      Serial.print(" °C\t");
      Serial.print("Humidity: ");
      Serial.print(humidity, 2);
      Serial.println(" %");
      
      // Chuyển đổi giá trị float thành chuỗi
      char tempStr[10];
      char humStr[10];
      
      // Sử dụng dtostrf để chuyển float thành chuỗi
      dtostrf(temperature, 5, 2, tempStr);
      dtostrf(humidity, 5, 2, humStr);
      
      // Loại bỏ khoảng trắng dư thừa
      for(int i=0; tempStr[i]==' '; i++) {
        for(int j=i; tempStr[j]; j++) tempStr[j]=tempStr[j+1];
        i--;
      }
      for(int i=0; humStr[i]==' '; i++) {
        for(int j=i; humStr[j]; j++) humStr[j]=humStr[j+1];
        i--;
      }
      
      // Đọc điện áp pin (tùy chọn)
      uint16_t batteryLevel = analogRead(A0);
      
      // Đóng gói dữ liệu thành chuỗi 
      sprintf(dataPacket, "T:%s,H:%s,B:%d,TS:%lu", 
              tempStr, 
              humStr, 
              batteryLevel, 
              currentTime);
      
      // Gửi dữ liệu qua LoRa E32
      sendData(dataPacket);
    } else {
      Serial.println("Error reading data from the SHT30 sensor.");
    }
  }
}
// Thiết lập chế độ cho module E32
void setE32Mode(int m0, int m1) {
  digitalWrite(E32_M0, m0);
  digitalWrite(E32_M1, m1);
  delay(100); // Đợi E32 chuyển chế độ
}

// Gửi dữ liệu qua E32
void sendData(const char* data) {
  Serial.print("Sending data via LoRa... ");
  Serial.println(data);
  
  // Gửi chuỗi dữ liệu qua UART tới E32
  E32Serial.print(data);
  
  // Thêm độ trễ để đảm bảo E32 có đủ thời gian gửi dữ liệu
  delay(100);
  
  Serial.println("Data sent successfully!!");
}

// Kiểm tra kết nối với cảm biến
bool isConnected() {
  Wire.beginTransmission(SHT30_ADDRESS);
  byte error = Wire.endTransmission();
  
  return (error == 0); // Trả về true nếu không có lỗi
}

// Đọc dữ liệu từ cảm biến SHT30
bool readSHT30(float *temperature, float *humidity) {
  uint8_t data[6];
  
  // Gửi lệnh đo với độ phân giải cao
  Wire.beginTransmission(SHT30_ADDRESS);
  Wire.write(SHT30_MEASURE_HIGH);
  Wire.write(SHT30_MEASURE_HIGH_REP);
  if (Wire.endTransmission() != 0) {
    return false; // Trả về false nếu có lỗi khi gửi lệnh
  }
  
  delay(100); // Đợi cảm biến xử lý lệnh và đo
  
  // Đọc 6 byte dữ liệu (2 nhiệt độ + 1 CRC + 2 độ ẩm + 1 CRC)
  Wire.requestFrom(SHT30_ADDRESS, (uint8_t)6);
  
  if (Wire.available() != 6) {
    return false; // Trả về false nếu không đủ dữ liệu
  }
  
  // Đọc 6 byte dữ liệu
  for (int i = 0; i < 6; i++) {
    data[i] = Wire.read();
  }
  
  // Tính toán CRC cho nhiệt độ
  if (!checkCRC(data[0], data[1], data[2])) {
    return false; // Dữ liệu nhiệt độ không hợp lệ
  }
  
  // Tính toán CRC cho độ ẩm
  if (!checkCRC(data[3], data[4], data[5])) {
    return false; // Dữ liệu độ ẩm không hợp lệ
  }
  
  // Tính toán nhiệt độ
  uint16_t temp_raw = ((uint16_t)data[0] << 8) | data[1];
  *temperature = -45.0f + 175.0f * ((float)temp_raw / 65535.0f);
  
  // Tính toán độ ẩm
  uint16_t hum_raw = ((uint16_t)data[3] << 8) | data[4];
  *humidity = 100.0f * ((float)hum_raw / 65535.0f);
  
  return true; // Đọc dữ liệu thành công
}

// Kiểm tra giá trị CRC
bool checkCRC(uint8_t msb, uint8_t lsb, uint8_t crc) {
  uint8_t calculated_crc = calculateCRC(msb, lsb);
  return (calculated_crc == crc);
}

// Tính toán giá trị CRC
uint8_t calculateCRC(uint8_t msb, uint8_t lsb) {
  uint8_t crc = 0xFF; // Giá trị khởi tạo
  
  // Tính CRC cho byte đầu tiên
  crc ^= msb;
  for (int i = 0; i < 8; i++) {
    if (crc & 0x80) {
      crc = (crc << 1) ^ 0x31; // Đa thức CRC-8: x^8 + x^5 + x^4 + 1 -> 0x31
    } else {
      crc = (crc << 1);
    }
  }
  
  // Tính CRC cho byte thứ hai
  crc ^= lsb;
  for (int i = 0; i < 8; i++) {
    if (crc & 0x80) {
      crc = (crc << 1) ^ 0x31;
    } else {
      crc = (crc << 1);
    }
  }
  
  return crc;
}
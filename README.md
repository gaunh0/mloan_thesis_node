# Hướng dẫn sử dụng hệ thống LoRa Sensor

## Tổng quan hệ thống

Hệ thống gồm 2 thành phần chính:
- **Arduino Node**: Đọc cảm biến SHT30 và truyền dữ liệu qua LoRa
- **ESP32 Gateway**: Nhận dữ liệu từ Node và gửi lên MQTT broker

## 1. Chuẩn bị phần cứng

### Arduino Node
- **Arduino Uno/Nano**
- **Cảm biến SHT30** (kết nối I2C)
- **Module LoRa E32**
- **Nguồn pin** (đọc qua chân A0)

### ESP32 Gateway
- **ESP32 DevKit**
- **Module LoRa E32**
- **Nút nhấn** (GPIO 23)

### Sơ đồ kết nối

#### Arduino Node
```
SHT30:
- VCC → 3.3V
- GND → GND
- SDA → A4 (I2C Data)
- SCL → A5 (I2C Clock)

LoRa E32:
- VCC → 5V
- GND → GND
- RX → Pin 10
- TX → Pin 11
- M0 → Pin 6
- M1 → Pin 7

Pin cảm biến:
- A0 → Đầu đọc điện áp pin
```

#### ESP32 Gateway
```
LoRa E32:
- VCC → 3.3V
- GND → GND
- RX → GPIO 16
- TX → GPIO 17
- M0 → GPIO 22
- M1 → GPIO 21

Nút cấu hình:
- GPIO 23 → Nút nhấn → GND
```

## 2. Cài đặt thư viện

**Xem cho vui chứ không cần nạp code lại chi**

### Arduino IDE - Node
```
Thư viện cần thiết:
- Wire (built-in)
- SoftwareSerial (built-in)
```

### Arduino IDE - Gateway
```
Thư viện cần thiết:
- WiFi (ESP32 built-in)
- WiFiManager
- PubSubClient
- ArduinoJson
```

## 3. Cấu hình và nạp code

### Bước 1: Cấu hình Node ID
Trong code Arduino Node, thay đổi:
```cpp
#define NODE_ID "NODE001"
```
Mỗi node cần có ID khác nhau (NODE002, NODE003, v.v.)

### Bước 2: Cấu hình MQTT (Gateway)
Trong code ESP32 Gateway:
```cpp
const char* mqtt_server = "dev.combros.tech";
const char* mqtt_username = "combros";
const char* mqtt_password = "combros";
```

### Bước 3: Nạp code
1. Nạp code vào Arduino Node trước
2. Nạp code vào ESP32 Gateway

## 4. Khởi động hệ thống

### Khởi động lần đầu

1. **Bật Node**: LED trên Arduino sáng, hiển thị thông tin trên Serial Monitor
2. **Bật Gateway**: ESP32 tạo WiFi hotspot "ESP32-Gateway"
3. **Cấu hình WiFi**: 
   - Kết nối WiFi "ESP32-Gateway" (password: "password")
   - Mở browser → 192.168.4.1
   - Chọn WiFi và nhập password
   - Gateway tự động kết nối MQTT

### Kiểm tra hoạt động
- **Node Serial**: Hiển thị dữ liệu cảm biến và trạng thái gửi
- **Gateway Serial**: Hiển thị dữ liệu nhận và MQTT status
- **MQTT Broker**: Kiểm tra topic `lora_sensor/data`

## 5. Sử dụng MQTT API

### Topics chính

#### Nhận dữ liệu cảm biến
```
Topic: lora_sensor/data
Format: {"nodeId":"NODE001","temperature":24.18,"humidity":57.94,"batteryVoltage":1.55}
```

#### Yêu cầu đọc ngay lập tức
```
Topic: lora_sensor/get_now
Message: {"type":"GET_NOW"}
```

#### Điều khiển chu kỳ gửi dữ liệu
```
Topic: lora_sensor/set_polling
Messages:
- {"nodeId":"NODE001","polling":"30"}    // 30 giây
- {"nodeId":"NODE001","polling":"OFF"}   // Tắt tự động gửi
```

#### Trạng thái Gateway
```
Topic: lora_sensor/last_will
Messages:
- {"type":"ONLINE"}   // Gateway online
- {"type":"OFFLINE"}  // Gateway offline
```


## 6. Cấu hình nâng cao

### Thay đổi chu kỳ mặc định (Node)
```cpp
unsigned long pollingInterval = 30000; // 30 giây
```

### Cấu hình timeout ACK (Node)
```cpp
const unsigned long ACK_TIMEOUT = 3000; // 3 giây
```

### Cấu hình heartbeat (Gateway)
```cpp
const long statusUpdateInterval = 120000; // 2 phút
```

## 7. Quản lý WiFi Gateway

### Cấu hình lại WiFi
1. **Nhấn nút** (GPIO 23) trong **1-5 giây**
2. Gateway vào chế độ cấu hình
3. Kết nối WiFi "ESP32-Gateway"
4. Cấu hình WiFi mới

### Reset hoàn toàn
1. **Nhấn giữ nút** (GPIO 23) **hơn 5 giây**
2. Gateway xóa cấu hình WiFi và khởi động lại

## Danh sách API

Xem trong folder Documents

## Workflow

Xem trong folder Documents

## Kiến trúc hệ thống

Xem trong folder Documents


# LoRa API Documentation

## Tổng quan

LoRa API định nghĩa giao thức truyền thông giữa Node (Arduino) và Gateway (ESP32) qua module LoRa E32.

## Cấu hình phần cứng

### Module LoRa E32
```
Frequency: 433MHz 
Baud Rate: 9600
Mode: Normal (M0=0, M1=0)
Power: 100mW
Range: 3km (line of sight)
```

### Kết nối
**Node (Arduino):**
- TX → Pin 10, RX → Pin 11
- M0 → Pin 6, M1 → Pin 7

**Gateway (ESP32):**
- TX → GPIO 16, RX → GPIO 17  
- M0 → GPIO 22, M1 → GPIO 21

## Giao thức truyền thông

### 1. Node → Gateway (Uplink)

**Định dạng:** `ID:NODE001,T:24.18,H:57.94,B:308`

**Cấu trúc:**
- `ID:` + Node ID (NODE001, NODE002, ...)
- `T:` + Temperature (°C, 2 chữ số thập phân)
- `H:` + Humidity (%, 2 chữ số thập phân)
- `B:` + Battery (ADC value 0-1023)

**Ví dụ:**
```
ID:NODE001,T:25.50,H:65.20,B:756
ID:NODE002,T:22.30,H:45.80,B:892
```

**Tần suất:** Theo chu kỳ được cấu hình (mặc định 10 giây)

### 2. Gateway → Node (Downlink/ACK)

**Định dạng cơ bản:** `ACK,NODE001`

**Với lệnh:** `ACK,NODE001,polling:30,getNow:1`

**Cấu trúc:**
- `ACK` + Node ID
- `polling:XX` - Chu kỳ gửi mới (giây) hoặc "OFF"
- `getNow:1` - Yêu cầu gửi ngay lập tức

**Ví dụ:**
```
ACK,NODE001                           # ACK đơn giản
ACK,NODE001,polling:60                # Đặt chu kỳ 60 giây
ACK,NODE001,getNow:1                  # Yêu cầu gửi ngay
ACK,NODE001,polling:OFF               # Tắt gửi tự động
ACK,NODE001,polling:30,getNow:1       # Kết hợp lệnh
```

## Luồng hoạt động

### Quy trình chuẩn
1. **Node** gửi dữ liệu cảm biến
2. **Gateway** nhận và xử lý dữ liệu
3. **Gateway** gửi ACK (có thể kèm lệnh)
4. **Node** nhận ACK và thực hiện lệnh (nếu có)

### Timeout
- **Node** chờ ACK trong 2 giây
- **Gateway** detect packet hoàn chỉnh sau 50ms không có ký tự mới
- Node timeout > 60 giây được coi là offline

## Chi tiết lệnh

### GET_NOW Command
**Từ Server:** MQTT `get_now` 
**Qua LoRa:** `ACK,NODE001,getNow:1`
**Kết quả:** Node gửi dữ liệu ngay lập tức

### POLLING Command
**Từ Server:** MQTT `set_polling` với giá trị
**Qua LoRa:** `ACK,NODE001,polling:XX`
**Kết quả:** Node thay đổi chu kỳ gửi

**Giá trị hợp lệ:**
- `1-3600`: Chu kỳ tính bằng giây
- `OFF`: Tắt gửi tự động

## Ví dụ communication

### Giao tiếp bình thường
```
Node:    ID:NODE001,T:24.18,H:57.94,B:308
Gateway: ACK,NODE001

Node:    ID:NODE001,T:24.25,H:58.12,B:307  
Gateway: ACK,NODE001
```

### Với lệnh từ server
```
[Server gửi GET_NOW qua MQTT]

Node:    ID:NODE001,T:24.18,H:57.94,B:308
Gateway: ACK,NODE001,getNow:1
Node:    ID:NODE001,T:24.18,H:57.94,B:308  # Gửi ngay lập tức
Gateway: ACK,NODE001
```

### Thay đổi polling
```
[Server gửi set_polling:60 qua MQTT]

Node:    ID:NODE001,T:24.18,H:57.94,B:308
Gateway: ACK,NODE001,polling:60
Node:    [Cập nhật chu kỳ thành 60 giây]
```
## Advanced Configuration

### Custom Node ID
```cpp
#define NODE_ID "SENSOR_01"  // Thay đổi trong code Arduino
```

### Polling Interval Range
- **Minimum:** 1 second
- **Maximum:** 3600 seconds (1 hour)
- **Default:** 10 seconds
- **Recommendation:** 10-60 seconds cho cân bằng battery/responsiveness
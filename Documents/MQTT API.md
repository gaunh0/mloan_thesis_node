# MQTT API Documentation

## Tổng quan

MQTT API cung cấp giao diện để điều khiển và nhận dữ liệu từ hệ thống cảm biến LoRa.

## Thông tin kết nối

```
Server: 
Port: 
Username: 
Password: 
Protocol: MQTT v3.1.1
```

## Topics

### 1. Dữ liệu cảm biến (Publish)

**Topic:** `lora_sensor/data`

**Mô tả:** Gateway publish dữ liệu cảm biến từ các node

**Định dạng:**
```json
{
  "nodeId": "NODE001",
  "temperature": 24.18,
  "humidity": 57.94,
  "batteryVoltage": 1.55
}
```

**Tần suất:** Theo chu kỳ node hoặc khi có lệnh GET_NOW

**Ví dụ:**
```json
{
  "nodeId": "NODE001",
  "temperature": 25.5,
  "humidity": 65.2,
  "batteryVoltage": 3.7
}
```

### 2. Yêu cầu đọc ngay (Subscribe)

**Topic:** `lora_sensor/get_now`

**Mô tả:** Yêu cầu node gửi dữ liệu ngay lập tức

**Định dạng:**
```json
{
  "type": "GET_NOW"
}
```

**Ví dụ sử dụng:**
```bash
mosquitto_pub -h dev.combros.tech -u combros -P combros \
  -t lora_sensor/get_now \
  -m '{"type":"GET_NOW"}'
```

**Kết quả:** Node sẽ gửi dữ liệu ngay lập tức qua topic `lora_sensor/data`

### 3. Điều khiển chu kỳ gửi (Subscribe)

**Topic:** `lora_sensor/set_polling`

**Mô tả:** Thay đổi chu kỳ gửi dữ liệu của node cụ thể

**Định dạng:**
```json
{
  "nodeId": "NODE001",
  "polling": "30"
}
```

**Tham số:**
- `nodeId`: ID của node cần điều khiển
- `polling`: Chu kỳ gửi (giây) hoặc "OFF" để tắt

**Ví dụ:**

Gửi mỗi 60 giây:
```bash
mosquitto_pub -h dev.combros.tech -u combros -P combros \
  -t lora_sensor/set_polling \
  -m '{"nodeId":"NODE001","polling":"60"}'
```

Tắt gửi tự động:
```bash
mosquitto_pub -h dev.combros.tech -u combros -P combros \
  -t lora_sensor/set_polling \
  -m '{"nodeId":"NODE001","polling":"OFF"}'
```

### 4. Trạng thái Gateway (Publish)

**Topic:** `lora_sensor/last_will`

**Mô tả:** Trạng thái online/offline của Gateway

**Định dạng:**
```json
{
  "type": "ONLINE"
}
```
hoặc
```json
{
  "type": "OFFLINE"
}
```

**Tần suất:** 
- ONLINE: Mỗi 60 giây (heartbeat)
- OFFLINE: Khi Gateway mất kết nối


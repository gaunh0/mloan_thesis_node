# Hệ thống Giám sát Cảm biến LoRa

## Tổng quan

Hệ thống giám sát cảm biến LoRa là một giải pháp IoT hoàn chình được xây dựng trên nền tảng Node-RED, cho phép thu thập, lưu trữ và hiển thị dữ liệu từ các cảm biến LoRa trong thời gian thực. Hệ thống hỗ trợ giám sát nhiều node cảm biến đồng thời với khả năng cấu hình linh hoạt và giao diện trực quan.

## Kiến trúc Hệ thống

### 1. **Tầng Giao diện Người dùng (User Interface)**
- **Dashboard Web**: Giao diện trực quan hiển thị dữ liệu real-time
- **Điều khiển Polling**: Cấu hình tần suất thu thập dữ liệu cho từng node (5s, 10s, 30s, 60s)
- **Nút GET NOW**: Lấy dữ liệu ngay lập tức từ tất cả cảm biến
- **Bộ lọc dữ liệu**: Lọc theo ngày và theo node cụ thể

### 2. **Tầng Xử lý Dữ liệu (Data Processing)**
- **Parser Engine**: Phân tích và chuẩn hóa dữ liệu JSON từ MQTT
- **Polling Manager**: Quản lý lịch trình thu thập dữ liệu tự động
- **Data Formatter**: Format dữ liệu cho hiển thị và lưu trữ
- **Status Monitor**: Theo dõi trạng thái kết nối của gateway

### 3. **Tầng Giao tiếp (Communication Layer)**
- **MQTT Broker**: 
  - Topic nhận dữ liệu: `lora_sensor/data`
  - Topic trạng thái: `lora_sensor/main_status`
  - Topic điều khiển: `lora_sensor/get_now`, `lora_sensor/set_polling`

**EMQX** là một MQTT broker mã nguồn mở, hiệu suất cao, được thiết kế cho IoT và ứng dụng real-time. Với khả năng xử lý hàng triệu kết nối đồng thời, EMQX cung cấp độ tin cậy và khả năng mở rộng tuyệt vời cho hệ thống IoT. Broker hỗ trợ đầy đủ các tính năng MQTT 5.0 và có cơ chế bảo mật mạnh mẽ.

### 4. **Tầng Lưu trữ (Storage Layer)**
- **InfluxDB**: 
  - Database: `UIT_LORA`
  - Measurement: `history`
  - Lưu trữ time-series data với tags và fields

**InfluxDB** là cơ sở dữ liệu time-series được tối ưu hóa đặc biệt cho việc lưu trữ và truy vấn dữ liệu có dấu thời gian. Với khả năng nén dữ liệu cao và tốc độ ghi/đọc nhanh, InfluxDB hoàn hảo cho ứng dụng IoT monitoring. Database hỗ trợ retention policy tự động và có ngôn ngữ truy vấn Flux mạnh mẽ cho việc phân tích dữ liệu.

## Chức năng Chính

### **Thu thập Dữ liệu**
- **Automatic Polling**: Thu thập dữ liệu theo lịch trình đã cấu hình
- **Manual Request**: Lấy dữ liệu ngay lập tức khi cần
- **Multi-node Support**: Hỗ trợ đồng thời nhiều node cảm biến

### **Hiển thị Real-time**
- **Node Status Boxes**: Hiển thị thông tin chi tiết từng node
  - Nhiệt độ (°C)
  - Độ ẩm (%)
  - Điện áp pin (V)
  - Thời gian cập nhật cuối
- **Gateway Status**: Trạng thái kết nối ONLINE/OFFLINE
- **Data Table**: Bảng dữ liệu lịch sử có thể lọc và sắp xếp

### **Quản lý Dữ liệu**
- **Historical Data**: Lưu trữ toàn bộ lịch sử đo đạc
- **Date Filtering**: Lọc dữ liệu theo ngày cụ thể
- **Node Filtering**: Xem dữ liệu của node riêng lẻ hoặc tất cả
- **Auto Refresh**: Tự động làm mới dữ liệu khi khởi động

## Thông số Kỹ thuật

### **Cảm biến được Giám sát**
- **Node 1** (NODE001): Cảm biến nhiệt độ, độ ẩm, pin
- **Node 2** (NODE002): Cảm biến nhiệt độ, độ ẩm, pin

### **Tần suất Thu thập**
- **Configurable Intervals**: 5, 10, 30, 60 giây
- **Real-time Monitoring**: Kiểm tra mỗi giây
- **On-demand Polling**: Lấy dữ liệu ngay lập tức

### **Độ chính xác Dữ liệu**
- **Temperature**: Làm tròn 2 chữ số thập phân
- **Humidity**: Làm tròn 2 chữ số thập phân  
- **Battery Voltage**: Làm tròn 2 chữ số thập phân

## Ưu điểm của Hệ thống

### **Reliability**
- Kết nối MQTT tin cậy với QoS 1-2
- Auto-reconnect và error handling
- Backup data trong InfluxDB

### **Scalability** 
- Dễ dàng thêm node mới
- Cấu trúc modular cho việc mở rộng
- Hỗ trợ multiple measurement types

### **User Experience**
- Giao diện trực quan, responsive
- Real-time updates không cần refresh
- Filtering và search capabilities

### **Flexibility**
- Cấu hình polling linh hoạt
- Multiple data export options
- Customizable dashboard layout

## Ứng dụng Thực tế

Hệ thống phù hợp cho:
- **Monitoring môi trường**: Theo dõi nhiệt độ, độ ẩm trong nhà kính, kho bãi
- **IoT Agriculture**: Giám sát điều kiện môi trường nông nghiệp
- **Smart Building**: Quản lý hệ thống HVAC và tiết kiệm năng lượng
- **Research Projects**: Thu thập dữ liệu cho nghiên cứu khoa học

## Roadmap Phát triển

- **Mobile App**: Ứng dụng di động cho giám sát từ xa
- **Advanced Analytics**: Thêm tính năng phân tích xu hướng và cảnh báo
- **Multiple Protocol Support**: Hỗ trợ thêm Zigbee, WiFi
- **Cloud Integration**: Tích hợp với các cloud platform lớn
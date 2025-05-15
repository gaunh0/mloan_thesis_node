# Hệ Thống Thu Thập Dữ Liệu SHT30 qua LoRa E32

Dự án này xây dựng một hệ thống mạng cảm biến không dây sử dụng Arduino, cảm biến SHT30 và mô-đun truyền thông LoRa E32 để thu thập và truyền dữ liệu nhiệt độ và độ ẩm từ nhiều node khác nhau.

## Tổng Quan

Hệ thống này hoạt động như một mạng các node truyền (transmitter), mỗi node có địa chỉ duy nhất và thực hiện các chức năng sau:
- Mỗi node có một ID duy nhất để phân biệt trong mạng lưới cảm biến
- Đọc dữ liệu nhiệt độ và độ ẩm từ cảm biến SHT30 qua giao tiếp I2C
- Định dạng dữ liệu thành chuỗi có cấu trúc bao gồm ID của node
- Truyền dữ liệu không dây qua mô-đun LoRa E32
- Thực hiện đọc và truyền dữ liệu theo chu kỳ được cấu hình

## Yêu Cầu Phần Cứng

- Arduino (Uno/Nano/Pro Mini...)
- Cảm biến SHT30
- Mô-đun LoRa E32
- Dây cáp kết nối
- (Tùy chọn) Pin hoặc nguồn năng lượng độc lập

## Kết Nối Phần Cứng

### Kết nối SHT30 với Arduino:
- VCC → 3.3V hoặc 5V (tùy theo phiên bản SHT30)
- GND → GND
- SDA → A4 (hoặc pin SDA của Arduino)
- SCL → A5 (hoặc pin SCL của Arduino)

### Kết nối LoRa E32 với Arduino:
- VCC → 3.3V
- GND → GND
- RX → Pin 10 (TX Software Serial)
- TX → Pin 11 (RX Software Serial)
- M0 → Pin 6
- M1 → Pin 7
- (AUX không được sử dụng trong mã này)

### Tùy chọn - Đo pin:
- Pin dương → A0 (thông qua bộ chia điện áp nếu cần)

## Đặc Điểm Chính

- **ID Node**: Mỗi node có một định danh duy nhất (mặc định: "NODE001")
- **Chu kỳ đo**: Mặc định là 10 giây (có thể điều chỉnh)
- **Định dạng dữ liệu**: `ID:[id_node],T:[nhiệt_độ],H:[độ_ẩm],B:[mức_pin],TS:[thời_gian]`
- **Kiểm tra tính toàn vẹn**: Sử dụng CRC-8 để xác minh dữ liệu từ cảm biến
- **Chế độ hoạt động LoRa**: Chế độ bình thường (M0=HIGH, M1=HIGH)

## Cài Đặt

1. Cài đặt thư viện cần thiết:
   - Wire.h (tích hợp sẵn trong Arduino IDE)
   - SoftwareSerial.h (tích hợp sẵn trong Arduino IDE)

2. Kết nối phần cứng theo sơ đồ được mô tả ở trên

3. Tải mã nguồn lên Arduino qua Arduino IDE

4. Mở Serial Monitor (tốc độ 9600 baud) để xem dữ liệu debug

## Cấu Hình

Các thông số chính có thể được điều chỉnh:

- `NODE_ID`: Định danh duy nhất của node (mặc định: "NODE001")
- `sendInterval`: Thời gian giữa các lần đọc và gửi dữ liệu (mặc định: 10000ms)
- `MAX_PACKET_SIZE`: Kích thước tối đa của gói dữ liệu (mặc định: 128 byte)
- Chế độ LoRa E32: Được thiết lập trong hàm `setE32Mode()`

## Cách Hoạt Động

1. Khi khởi động, hệ thống sẽ:
   - Thiết lập kết nối với cảm biến SHT30
   - Cấu hình mô-đun LoRa E32 ở chế độ hoạt động bình thường
   - Hiển thị ID của node trên Serial Monitor
   - Kiểm tra xem cảm biến SHT30 có được kết nối chính xác không

2. Trong quá trình hoạt động:
   - Đọc dữ liệu từ SHT30 theo chu kỳ đã cấu hình
   - Kiểm tra tính toàn vẹn dữ liệu bằng CRC
   - Định dạng dữ liệu thành chuỗi có cấu trúc bao gồm ID của node
   - Gửi dữ liệu qua mô-đun LoRa E32
   - Hiển thị thông tin debug trên Serial Monitor

## Xử Lý Lỗi

Mã nguồn có tích hợp một số cơ chế xử lý lỗi:
- Kiểm tra kết nối cảm biến khi khởi động
- Xác minh tính toàn vẹn dữ liệu bằng CRC
- Thông báo lỗi qua Serial Monitor

## Mở Rộng

Hệ thống có thể được mở rộng:
- Triển khai nhiều node với ID khác nhau để tạo mạng cảm biến lớn hơn
- Thêm cảm biến bổ sung vào mỗi node
- Tối ưu hóa tiêu thụ năng lượng cho các ứng dụng chạy bằng pin
- Bổ sung bảo mật cho dữ liệu được truyền
- Thiết lập node trung tâm (gateway) để thu thập dữ liệu từ tất cả các node

## Lưu ý

- Đảm bảo thiết lập `NODE_ID` duy nhất cho mỗi node trong mạng lưới
- Đảm bảo cấu hình node nhận (receiver) phù hợp với các thông số của node truyền
- Kiểm tra điện áp hoạt động của tất cả các thành phần
- Để mở rộng phạm vi, hãy cân nhắc cấu hình mức công suất phát của LoRa E32

## Giấy Phép

MIT

## Tác Giả

mloan
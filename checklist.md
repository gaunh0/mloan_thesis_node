## ✅ **I. Chuẩn bị trước demo**

### A. Hệ thống phần cứng

* [x] ESP32 Gateway (nạp code sẵn)
* [x] 2 Node Arduino (nạp code sẵn)
* [x] Mạch điện & sơ đồ kết nối
* [x] Ăng-ten đã căn chỉnh
* [x] Test ổn định nguồn, pin

### B. Phần mềm hệ thống

* [x] Code gateway và node (đã có trên GitHub)
* [x] ~~File docker-compose chạy EMQX, InfluxDB, Node-RED~~
* [x] ~~Dashboard Node-RED đã export~~
* [ ] ~~File hướng dẫn nạp code~~
* [ ] ~~File hướng dẫn chạy phần mềm (docker-compose + import dashboard)~~
* [x] VPS deploy hệ thống (mua từ [vpssieutoc.vn](https://vpssieutoc.vn/cart.php?a=confproduct&i=0))

**Mua host rồi nên không cần quan tâm**

---

## ✅ **II. Slide trình bày**

### A. Giới thiệu công nghệ

* [x] Slide giới thiệu LoRa
* [x] Slide giới thiệu Node-RED
* [x] Slide giới thiệu EMQX và InfluxDB (ngắn gọn)
* [ ] Slide Use Case thực tế (nông nghiệp, nhà xưởng…)
* [ ] Slide so sánh LoRa vs WiFi/BLE/Zigbee (optional)

### B. Giới thiệu hệ thống

* [ ] Kiến trúc tổng quan
* [ ] Flowchart: Gateway ↔ Node (đã có)
* [ ] Flowchart: Gateway ↔ MQTT
* [ ] Flowchart: Node-RED dashboard
* [ ] Slide test phạm vi truyền (range)
* [ ] Slide test tiêu thụ pin (optional)
* [ ] Slide kết quả đạt được
* [ ] Slide hạn chế
* [ ] Slide hướng phát triển
* [ ] Slide demo trực tiếp

---

## ✅ **III. Demo trực tiếp**

### A. Chuẩn bị

* [ ] Bật nguồn node & gateway
* [ ] Kết nối WiFi sẵn (hoặc auto connect)
* [ ] Kiểm tra gateway kết nối MQTT thành công
* [ ] Mở dashboard Node-RED trên VPS/local

### B. Thực hiện

* [ ] Quan sát dữ liệu sensor thay đổi
* [ ] Dùng filter theo node ID / ngày
* [ ] Tắt/mở thiết bị nếu có (chứng minh real-time)
* [ ] Nếu lỗi → mở video backup

---

## ✅ **IV. Video demo backup (phòng sự cố)**

* [ ] Quay đủ 3 phần:

  * [ ] Gateway giao tiếp LoRa với node
  * [ ] Dashboard hiển thị dữ liệu
  * [ ] Filter & tương tác dữ liệu real-time

---

## ✅ **V. Viết báo cáo**

### Nội dung chính

* [ ] Giới thiệu công nghệ
* [ ] Phân tích hệ thống & luồng dữ liệu
* [ ] Các bước triển khai
* [ ] Flowcharts (đầy đủ)
* [ ] Kết quả đạt được
* [ ] Các hạn chế
* [ ] Hướng phát triển

---

## ✅ **VI. Keyword / Kiến thức cần hiểu khi bảo vệ**

* [ ] LoRa là gì? Tại sao chọn LoRa?
* [ ] Vai trò của gateway
* [ ] MQTT là gì? Broker là gì?
* [ ] InfluxDB lưu dữ liệu như thế nào?
* [ ] Node-RED là gì? Có thể xử lý logic không?
* [ ] Lợi ích của hệ thống: tiết kiệm năng lượng, mở rộng được không?
* [ ] Điểm khác biệt nếu dùng WiFi/Zigbee/BLE?

---

## ✅ **VII. Các rủi ro & cách xử lý**

| **Vấn đề**                           | **Giải pháp**                                    |
| ------------------------------------ | ------------------------------------------------ |
| Mạch điện lỗi                        | Có clip backup, kiểm tra nguồn trước             |
| Gateway không giao tiếp được         | Kiểm tra WiFi, ưu tiên WiFi nội bộ               |
| Lỗi Docker / không mở được dashboard | Đã mua VPS nên sẽ không bị                           |
| Câu hỏi khó từ giảng viên            | Nắm keyword, hiểu tổng thể luồng hệ thống        |
| Viết sai báo cáo/slide               | Dựa trên checklist, đã liệt kê sẵn phần cần viết |



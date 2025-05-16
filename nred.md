Tôi sẽ giúp bạn tạo một cấu hình Docker để chạy MQTT (Mosquitto) và Node-RED cùng nhau. Đây là một cách phổ biến để xây dựng hệ thống IoT với khả năng kết nối và xử lý dữ liệu.

Dưới đây là file `docker-compose.yml` để chạy cả MQTT broker (Mosquitto) và Node-RED trong cùng một môi trường:

```yaml
version: '3'
services:
  mosquitto:
    image: eclipse-mosquitto:latest
    container_name: mosquitto
    ports:
      - "1883:1883"
      - "9001:9001"
    volumes:
      - ./mosquitto/config:/mosquitto/config
      - ./mosquitto/data:/mosquitto/data
      - ./mosquitto/log:/mosquitto/log
    restart: unless-stopped
    networks:
      - iot-network

  nodered:
    image: nodered/node-red:latest
    container_name: nodered
    ports:
      - "1880:1880"
    volumes:
      - ./nodered-data:/data
    restart: unless-stopped
    depends_on:
      - mosquitto
    networks:
      - iot-network

networks:
  iot-network:
    driver: bridge
```

Trước khi chạy, bạn cần tạo các thư mục cần thiết và file cấu hình Mosquitto:

1. Tạo cấu trúc thư mục:

```bash
mkdir -p mosquitto/config mosquitto/data mosquitto/log nodered-data
```

2. Tạo file cấu hình Mosquitto tại `mosquitto/config/mosquitto.conf`:

```
persistence true
persistence_location /mosquitto/data
log_dest file /mosquitto/log/mosquitto.log

# Bật websocket để kết nối qua web
listener 9001
protocol websockets

# Bật MQTT qua cổng 1883
listener 1883
protocol mqtt

# Cho phép các kết nối không cần xác thực
allow_anonymous true
```

3. Chạy Docker Compose:

```bash
docker-compose up -d
```

Sau khi chạy, bạn có thể truy cập:
- Node-RED tại: http://localhost:1880
- MQTT broker tại cổng 1883 cho MQTT và 9001 cho WebSockets

Để sử dụng MQTT trong Node-RED, bạn cần cài đặt nút MQTT trong Node-RED:
1. Vào Node-RED
2. Menu > Manage palette > Install
3. Tìm kiếm "node-red-contrib-mqtt" và cài đặt

Khi cấu hình kết nối MQTT trong Node-RED, dùng `mosquitto` làm tên máy chủ (thay vì localhost) vì đây là tên service trong mạng Docker.

Bạn cần điều chỉnh gì thêm không?
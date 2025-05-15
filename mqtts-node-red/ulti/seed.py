import random
from datetime import datetime, timedelta
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS

# Cấu hình InfluxDB (từ docker-compose.yml trước đó)
url = "http://localhost:8086"
token = "your-admin-token"  # Thay bằng token từ docker-compose.yml
org = "myorg"
bucket = "mybucket"

# Kết nối với InfluxDB
client = InfluxDBClient(url=url, token=token, org=org)
write_api = client.write_api(write_options=SYNCHRONOUS)

# Hàm tạo dữ liệu giả lập
def generate_sensor_data(start_time, end_time, interval_minutes=5):
    data_points = []
    node_ids = ["NODE001", "NODE002", "NODE003"]
    locations = ["room1", "room2", "outdoor"]
    
    current_time = start_time
    while current_time <= end_time:
        for node_id in node_ids:
            for location in locations:
                battery_level = random.randint(600, 1023)  # Giả lập giá trị ADC
                battery_voltage = (battery_level * 5.0) / 1023.0  # Chuyển thành điện áp
                point = Point("environment") \
                    .tag("node_id", node_id) \
                    .tag("location", location) \
                    .field("temperature", random.uniform(20, 35)) \
                    .field("humidity", random.uniform(40, 90)) \
                    .field("battery_voltage", battery_voltage) \
                    .time(current_time, WritePrecision.NS)
                data_points.append(point)
        current_time += timedelta(minutes=interval_minutes)
    
    return data_points

# Thời gian giả lập: 30 ngày trước đến hiện tại
end_time = datetime.utcnow()
start_time = end_time - timedelta(days=30)

# Tạo dữ liệu
print("Generating seed data...")
data_points = generate_sensor_data(start_time, end_time)

# Ghi dữ liệu vào InfluxDB
print(f"Writing {len(data_points)} points to InfluxDB...")
write_api.write(bucket=bucket, org=org, record=data_points)
print("Seed data written successfully!")

# Đóng kết nối
client.close()
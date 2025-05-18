import random
from datetime import datetime, timedelta
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS

# Cấu hình InfluxDB
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
    node_ids = ["NODE01", "NODE02"]
    
    current_time = start_time
    while current_time <= end_time:
        for node_id in node_ids:
            point = Point("environment") \
                .tag("node_id", node_id) \
                .tag("gateway", "GATE01") \
                .field("temperature", random.uniform(20, 35)) \
                .field("humidity", random.uniform(40, 90)) \
                .time(current_time, WritePrecision.NS)
            data_points.append(point)
        current_time += timedelta(minutes=interval_minutes)
    
    return data_points

# Thời gian giả lập: 30 ngày trước đến hiện tại (17/05/2025, 12:11 PM +07)
end_time = datetime(2025, 5, 17, 5, 11)  # 12:11 PM +07 = 05:11 UTC
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
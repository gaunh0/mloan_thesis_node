import json
import random
from datetime import datetime, timedelta, timezone

# --- 1. Thiết lập khoảng thời gian seed (tất cả theo UTC) ---
# Bắt đầu: 2025-04-17 00:00:00 UTC
start_dt = datetime(2025, 5, 15, 0, 0, 0, tzinfo=timezone.utc)
# Kết thúc: 2025-05-31 23:59:59 UTC
end_dt   = datetime(2025, 5, 30, 23, 59, 59, tzinfo=timezone.utc)

# Danh sách nodeId (để xen kẽ)
node_ids = ["node1", "node2"]

# Số record cần tạo mỗi ngày
records_per_day = 100

# Lưu trữ tất cả các record JSON
data_list = []

# --- 2. Duyệt qua từng ngày ---
current_day = start_dt
while current_day.date() <= end_dt.date():
    # Tính epoch-second của "00:00:00" ngày đó (UTC)
    day_start_ts = int(current_day.timestamp())  # epoc_s

    # Khoảng giây giữa 2 record trong ngày
    interval_seconds = 86400 / records_per_day  # = 86400 s / 100 = 864 s mỗi record

    for i in range(records_per_day):
        # timestamp cụ thể (giây)
        ts = int(day_start_ts + i * interval_seconds)

        # Chọn nodeId xen kẽ
        node_id = node_ids[i % len(node_ids)]

        # Tạo giá trị giả cho nhiệt độ, độ ẩm, pin
        temperature    = round(random.uniform(20.0, 35.0), 2)
        humidity       = round(random.uniform(40.0, 90.0), 2)
        battery_voltage = round(random.uniform(0.003, 0.005), 4)

        # Tạo object JSON cho 1 dòng
        record = {
            "nodeId": node_id,
            "temperature": temperature,
            "humidity": humidity,
            "batteryVoltage": battery_voltage,
            "timestamp": ts
        }
        data_list.append(record)

    # Next day
    current_day += timedelta(days=1)

# --- 3. Ghi ra file JSON Lines (mỗi dòng 1 JSON) ---
file_path = "seed_data.txt"
with open(file_path, "w", encoding="utf-8") as f:
    for rec in data_list:
        f.write(json.dumps(rec, ensure_ascii=False) + "\n")

print(f"Đã tạo file seed: {file_path}")
print("Ví dụ 5 dòng đầu:")
with open(file_path, "r", encoding="utf-8") as f:
    for i in range(5):
        print(f.readline().strip())

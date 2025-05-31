import json
from influxdb import InfluxDBClient

# --- 1. Cấu hình kết nối InfluxDB v1.8 ---
HOST     = "3.0.132.68"       # hoặc IP server InfluxDB
PORT     = 8086
USERNAME = ""                # nếu có bật auth, điền username
PASSWORD = ""                # nếu có bật auth, điền password
DATABASE = "UIT_LORA"

client = InfluxDBClient(
    host=HOST,
    port=PORT,
    username=USERNAME,
    password=PASSWORD,
    database=DATABASE
)

# Kiểm tra xem database có tồn tại không, nếu chưa có thì tạo
dbs = [db["name"] for db in client.get_list_database()]
if DATABASE not in dbs:
    print(f"Database '{DATABASE}' chưa tồn tại, đang tạo mới...")
    client.create_database(DATABASE)
    print("Đã tạo database.")
client.switch_database(DATABASE)

# --- 2. Hàm chuẩn bị JSON point cho InfluxDB ---
def json_to_point(data_obj):
    """
    Chuyển một object JSON (dict) thành dict phù hợp cho influxdb.write_points()
    """
    # Format đúng: { "measurement": <str>,
    #               "tags": { ... },
    #               "time": <ISO8601 or epoch (s/ms)>,
    #               "fields": { <number fields> } }
    #
    # Ở đây data_obj có keys: nodeId, temperature, humidity, batteryVoltage, timestamp (giây)
    # Chúng ta sẽ tạo point như sau:
    #
    # + measurement: "history"
    # + tags: { "node": data_obj["nodeId"] }
    # + time: chuyển timestamp (giây) thành ISO8601 có hậu tố 'Z' (UTC), hoặc epoch_s
    # + fields: { "temperature": <float>, "humidity": <float>, "battery": <float> }
    #
    # LƯU Ý: InfluxDB v1 mặc định precision là nanosecond nhưng write_points()
    #         chấp nhận "time" dưới dạng ISO8601 string (Influx tự convert),
    #         hoặc epoch dưới dạng int (precision mặc định là s).
    #
    # Ở đây mình sẽ để "time" = ISO8601 (UTC).
    # Python: datetime.utcfromtimestamp(data_obj["timestamp"]).isoformat() + "Z"

    ts = int(data_obj.get("timestamp", 0))
    # Chuyển giây -> ISO8601 UTC
    iso_time = None
    try:
        from datetime import datetime
        iso_time = datetime.utcfromtimestamp(ts).isoformat() + "Z"
    except:
        iso_time = None

    return {
        "measurement": "history",
        "tags": {
            "nodeId": data_obj.get("nodeId", "")
        },
        "time": iso_time,   # nếu None, influx sẽ tự thêm timestamp server hiện tại
        "fields": {
            "temperature": float(data_obj.get("temperature", 0)),
            "humidity":    float(data_obj.get("humidity", 0)),
            "battery":     float(data_obj.get("batteryVoltage", 0)),
            "nodeId":      data_obj.get("nodeId", "")
        }
    }


# --- 3. Đọc file và gom thành danh sách points ---
file_path = "seed_data.txt"   # hoặc đường dẫn tuyệt đối

points = []
with open(file_path, "r", encoding="utf-8") as f:
    line_count = 0
    for line in f:
        line = line.strip()
        if not line:
            continue
        try:
            obj = json.loads(line)
            pt  = json_to_point(obj)
            points.append(pt)
            line_count += 1
        except json.JSONDecodeError as e:
            print(f"Lỗi JSON ở dòng {line_count+1}: {e}")
            continue

print(f"Đã đọc {line_count} dòng JSON. Chuẩn bị ghi vào InfluxDB...")

# --- 4. Ghi hàng loạt (batch) vào InfluxDB ---
# Lưu ý: write_points() mặc định precision="ns" cho time ISO8601 
#        hoặc precision="s" nếu time là epoch int (s).
# Ở đây time là ISO8601, Influx sẽ tự convert.
#
if points:
    success = client.write_points(points, time_precision="s")
    if success:
        print("Ghi thành công!")
    else:
        print("Ghi KHÔNG thành công. Có thể check lại server InfluxDB.")
else:
    print("Không có points để ghi.")

client.close()

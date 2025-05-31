/**
 * Arduino Node - Optimized for minimal memory usage
 * Đọc sensor SHT30 và gửi qua LoRa E32
 */

#include <Wire.h>
#include <SoftwareSerial.h>

// SHT30 configuration
#define SHT30_ADDRESS 0x44
#define SHT30_MEASURE_HIGH 0x2C
#define SHT30_MEASURE_HIGH_REP 0x06

// LoRa E32 configuration
#define E32_M0 6
#define E32_M1 7

// Node configuration
#define NODE_ID "node1"
#define MAX_PACKET_SIZE 64
#define MAX_ACK_SIZE 32

// SoftwareSerial for E32
SoftwareSerial E32Serial(11, 10); // RX, TX

// Variables - stored in RAM only
char dataPacket[MAX_PACKET_SIZE];
char ackBuffer[MAX_ACK_SIZE];
unsigned long lastSendTime = 0;
unsigned long pollingInterval = 10000; // Default 10 seconds
bool pollingEnabled = true;

// ACK handling
unsigned long ackWaitStart = 0;
const unsigned long ACK_TIMEOUT = 2000;
bool waitingForAck = false;

void setup() {
  Serial.begin(9600);
  E32Serial.begin(9600);
  Wire.begin();
  
  // Setup E32 control pins
  pinMode(E32_M0, OUTPUT);
  pinMode(E32_M1, OUTPUT);
  setE32Mode(0, 0); // Normal mode
  
  Serial.println(F("Node start"));
  Serial.println(NODE_ID);
  
  // Check SHT30
  if (!isConnected()) {
    Serial.println(F("SHT30 error"));
    while (1);
  }
  
  Serial.println(F("Ready"));
  delay(1000);
}

void loop() {
  unsigned long currentTime = millis();
  
  // Check for ACK/commands
  checkForAck();
  
  // Send data if enabled and time reached
  if (pollingEnabled && (currentTime - lastSendTime >= pollingInterval)) {
    sendSensorData();
    lastSendTime = currentTime;
    waitingForAck = true;
    ackWaitStart = currentTime;
  }
  
  // Handle ACK timeout
  if (waitingForAck && (currentTime - ackWaitStart > ACK_TIMEOUT)) {
    waitingForAck = false;
  }
}

void sendSensorData() {
  float temp, hum;
  
  if (readSHT30(&temp, &hum)) {
    // Convert to strings with minimal precision
    int tempInt = (int)(temp * 100);  // 24.18 -> 2418
    int humInt = (int)(hum * 100);    // 57.94 -> 5794
    uint16_t battery = analogRead(A0);
    
    // Create compact packet - no timestamp
    sprintf(dataPacket, "ID:%s,T:%d.%02d,H:%d.%02d,B:%d", 
            NODE_ID, 
            tempInt/100, tempInt%100,
            humInt/100, humInt%100,
            battery);
    
    E32Serial.println(dataPacket);
    Serial.print(F("TX: "));
    Serial.println(dataPacket);
  }
}

void checkForAck() {
  static char buffer[MAX_ACK_SIZE];
  static byte bufferIndex = 0;
  static unsigned long lastCharTime = 0;
  
  while (E32Serial.available()) {
    char c = E32Serial.read();
    
    if (bufferIndex < MAX_ACK_SIZE - 1) {
      buffer[bufferIndex++] = c;
    }
    lastCharTime = millis();
  }
  
  if (bufferIndex > 0 && (millis() - lastCharTime > 100)) {
    buffer[bufferIndex] = '\0';
    processAck(buffer);
    bufferIndex = 0;
    waitingForAck = false;
  }
}

void processAck(const char* msg) {
  Serial.print(F("RX: "));
  Serial.println(msg);
  
  // Simple string parsing without JSON library
  // Expected: ACK,NODE001,polling:30,getNow:1
  
  // Check if it's our ACK
  if (strstr(msg, "ACK") && strstr(msg, NODE_ID)) {
    Serial.println(F("ACK OK"));
    
    // Look for getNow command
    if (strstr(msg, "getNow:1")) {
      Serial.println(F("GET_NOW"));
      sendSensorData();
    }
    
    // Look for polling update
    char* pollingPos = strstr(msg, "polling:");
    if (pollingPos) {
      pollingPos += 8; // Skip "polling:"
      
      if (strncmp(pollingPos, "OFF", 3) == 0) {
        pollingEnabled = false;
        Serial.println(F("Polling OFF"));
      } else {
        int newInterval = parseNumber(pollingPos);
        if (newInterval > 0) {
          pollingInterval = newInterval * 1000UL;
          pollingEnabled = true;
          Serial.print(F("Polling: "));
          Serial.println(newInterval);
        }
      }
      lastSendTime = millis(); // Reset timing
    }
  }
}

// Parse number from string (simpler than atoi)
int parseNumber(const char* str) {
  int result = 0;
  while (*str >= '0' && *str <= '9') {
    result = result * 10 + (*str - '0');
    str++;
  }
  return result;
}

void setE32Mode(int m0, int m1) {
  digitalWrite(E32_M0, m0);
  digitalWrite(E32_M1, m1);
  delay(100);
}

bool isConnected() {
  Wire.beginTransmission(SHT30_ADDRESS);
  return (Wire.endTransmission() == 0);
}

bool readSHT30(float *temperature, float *humidity) {
  uint8_t data[6];
  
  // Send command
  Wire.beginTransmission(SHT30_ADDRESS);
  Wire.write(SHT30_MEASURE_HIGH);
  Wire.write(SHT30_MEASURE_HIGH_REP);
  if (Wire.endTransmission() != 0) return false;
  
  delay(20); // Reduced delay
  
  // Read data
  Wire.requestFrom(SHT30_ADDRESS, (uint8_t)6);
  if (Wire.available() != 6) return false;
  
  for (int i = 0; i < 6; i++) {
    data[i] = Wire.read();
  }
  
  // Skip CRC check to save memory
  
  // Calculate values
  uint16_t temp_raw = ((uint16_t)data[0] << 8) | data[1];
  *temperature = -45.0f + 175.0f * ((float)temp_raw / 65535.0f);
  
  uint16_t hum_raw = ((uint16_t)data[3] << 8) | data[4];
  *humidity = 100.0f * ((float)hum_raw / 65535.0f);
  
  return true;
}
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebServer.h>

WebServer server(80);
const char* ssid = "PRO192";
const char* password = "12345678";

WebSocketsServer webSocket = WebSocketsServer(81);

#define TRIG1 26
#define ECHO1 14
#define TRIG2 13
#define ECHO2 27

#define ENCODER_PIN 15
#define WHEEL_CIRCUMFERENCE 0.2042
#define PULSES_PER_REV 20

#define IN1 5
#define IN2 18
#define IN3 19
#define IN4 23
#define ENA 32
#define ENB 33

#define BUZZER_PIN 16  // Chân còi báo động

volatile int pulseCount = 0;
float distanceTraveled = 0.0;
unsigned long lastTime = 0;
float lastSpeed = 0;

void IRAM_ATTR countPulse() {
  pulseCount++;
}

float getSpeed() {
  unsigned long currentTime = millis();
  float timeInterval = (currentTime - lastTime) / 1000.0;

  if (timeInterval >= 1.0) {
    float wheelRevolutions = pulseCount / (float)PULSES_PER_REV;
    float speed_mps = (wheelRevolutions * WHEEL_CIRCUMFERENCE) / timeInterval;
    float speed_cms = speed_mps * 100;

    pulseCount = 0;
    lastTime = currentTime;
    lastSpeed = speed_cms;

    return speed_cms;
  }
  return lastSpeed;
}

float getDistance() {
  float wheelRevolutions = pulseCount / (float)PULSES_PER_REV;
  distanceTraveled += wheelRevolutions * WHEEL_CIRCUMFERENCE;
  float distance_cm = distanceTraveled;
  return distance_cm;
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("🔄 Đang kết nối WiFi");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(250);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi kết nối thành công!");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Không thể kết nối WiFi!");
    ESP.restart();
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_TEXT) {
    String command = String((char*)payload);
    Serial.println("Command received: " + command);

    if (command == "forward") {
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      digitalWrite(ENA, HIGH);
      digitalWrite(ENB, HIGH);
    } else if (command == "backward") {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      digitalWrite(ENA, HIGH);
      digitalWrite(ENB, HIGH);
    } else if (command == "left") {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      digitalWrite(ENA, HIGH);
      digitalWrite(ENB, HIGH);
    } else if (command == "right") {
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      digitalWrite(ENA, HIGH);
      digitalWrite(ENB, HIGH);
    } else if (command == "brake") {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      digitalWrite(ENA, LOW);
      digitalWrite(ENB, LOW);
    }
  }
}


void sendSensorData() {
  float frontDistance = getDistance(TRIG1, ECHO1);
  float rearDistance = getDistance(TRIG2, ECHO2);
  int batteryLevel = random(30, 100);
  float speed = getSpeed();
  float distance = getDistance();

  String jsonData = "{\"speed\":" + String(speed) + ", \"distance\":" + String(distance) + ", \"battery\":" + String(batteryLevel) + ", \"frontDistance\":" + String(frontDistance) + ", \"rearDistance\":" + String(rearDistance) + "}";

  webSocket.broadcastTXT(jsonData);
}

float getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2;
}

void setup() {
  Serial.begin(115200);
  connectWiFi();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  server.begin();

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), countPulse, RISING);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  lastTime = millis();
}

void loop() {
  webSocket.loop();
  server.handleClient();

  float speed = getSpeed();
  float distance = getDistance();
  if (speed != -1) {
   // Serial.printf("🚗 Tốc độ: %.2f cm/s | Quãng đường: %.2f cm\n", speed, distance);
  }

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 100) {
    sendSensorData();
    lastUpdate = millis();
  }

  float frontDistance = getDistance(TRIG1, ECHO1);
  float rearDistance = getDistance(TRIG2, ECHO2);


  if (frontDistance < 20) {
    tone(BUZZER_PIN, 700);  // Còi cảnh báo
   // Serial.println("🚨 Cảnh báo! Vật cản phía trước! Xe sẽ tự động lùi...");

    // Xe lùi lại 30 cm
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    digitalWrite(ENA, HIGH);
    digitalWrite(ENB, HIGH);
    delay(1000);  // Điều chỉnh thời gian để xe lùi khoảng 30 cm

    // Dừng xe
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    digitalWrite(ENA, LOW);
    digitalWrite(ENB, LOW);
    noTone(BUZZER_PIN);
    //Serial.println("✅ Hoàn tất lùi xe, tiếp tục điều khiển.");
  }

  if (rearDistance < 20) {
    tone(BUZZER_PIN, 700);  // Còi cảnh báo
   // Serial.println("🚨 Cảnh báo! Vật cản phía sau! Xe sẽ tự động tiến...");

    // Xe tiến lên 30 cm
    digitalWrite(IN1, HIGH);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    digitalWrite(ENA, HIGH);
    digitalWrite(ENB, HIGH);
    delay(1000);  // Điều chỉnh thời gian để xe tiến khoảng 30 cm

    // Dừng xe
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    digitalWrite(ENA, LOW);
    digitalWrite(ENB, LOW);

    noTone(BUZZER_PIN);
    //Serial.println("✅ Hoàn tất tiến xe, tiếp tục điều khiển.");
  }
}

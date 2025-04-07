#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// Thay bằng thông tin mạng Wi-Fi của bạn
const char* ssid = "your-SSID";    
const char* password = "your-PASSWORD";  

// Khởi tạo server
AsyncWebServer server(80);

// Khai báo các chân GPIO cho cảm biến HC-SR04
#define TRIG1 26
#define ECHO1 14
#define TRIG2 13
#define ECHO2 27
#define ENCODER_PIN 15  // Encoder (GPIO 15)

long distance1, distance2;
int encoderData = 0;  // Dữ liệu giả định từ encoder

void setup() {
  Serial.begin(115200);
  
  // Kết nối ESP32 với Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Định nghĩa các route HTTP
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    // Lấy dữ liệu từ các cảm biến và encoder
    distance1 = readDistance(TRIG1, ECHO1);  // Đọc cảm biến HC-SR04 phía trước
    distance2 = readDistance(TRIG2, ECHO2);  // Đọc cảm biến HC-SR04 phía sau
    encoderData = digitalRead(ENCODER_PIN);  // Đọc tín hiệu encoder (giả sử)
    
    // Trả dữ liệu về client (máy tính hoặc điện thoại)
    String data = "Position: " + String(encoderData) + ", Distance1: " + String(distance1) + " cm, Distance2: " + String(distance2) + " cm";
    request->send(200, "text/plain", data);  // Gửi dữ liệu về client
  });

  server.begin();
}

void loop() {
  // Code này để trống vì chúng ta chỉ cần nhận yêu cầu HTTP
}

// Hàm đọc khoảng cách từ cảm biến HC-SR04
long readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  long distance = (duration * 0.0344) / 2;  // Công thức tính khoảng cách (cm)
  return distance;
}

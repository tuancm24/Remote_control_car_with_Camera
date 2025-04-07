#define TRIG1 26
#define ECHO1 14
#define TRIG2 13
#define ECHO2 27
#define ENCODER_PIN 15  // Sử dụng GPIO 15 cho Encoder

long distance1, distance2;

void setup() {
  Serial.begin(115200);
  
  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);
  pinMode(ENCODER_PIN, INPUT);  // Đọc tín hiệu từ encoder
}

void loop() {
  // Đọc khoảng cách từ cảm biến HC-SR04 phía trước
  distance1 = readDistance(TRIG1, ECHO1);  
  // Đọc khoảng cách từ cảm biến HC-SR04 phía sau
  distance2 = readDistance(TRIG2, ECHO2);  

  // Đọc tín hiệu từ encoder (số xung từ bánh xe)
  int encoderData = digitalRead(ENCODER_PIN); 

  // In kết quả ra Serial Monitor
  Serial.print("Distance1 (Front): ");
  Serial.print(distance1);
  Serial.print(" cm, Distance2 (Back): ");
  Serial.print(distance2);
  Serial.print(" cm, Encoder Data: ");
  Serial.println(encoderData);

  delay(500);  // Đợi 500ms trước khi đo lại
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

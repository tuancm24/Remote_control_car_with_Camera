#define TRIG1 26
#define ECHO1 14
#define TRIG2 13
#define ECHO2 27

void setup() {
    Serial.begin(115200);
    
    pinMode(TRIG1, OUTPUT);
    pinMode(ECHO1, INPUT);
    pinMode(TRIG2, OUTPUT);
    pinMode(ECHO2, INPUT);

    Serial.println("🚀 ĐANG KIỂM TRA CẢM BIẾN KHOẢNG CÁCH...");
}

// Hàm đo khoảng cách của một cảm biến
float measureDistance(int trigPin, int echoPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    long duration = pulseIn(echoPin, HIGH);
    float distance = duration * 0.034 / 2;  // Công thức chuyển đổi thời gian thành khoảng cách (cm)
    return distance;
}

void loop() {
    float distance1 = measureDistance(TRIG1, ECHO1);
    float distance2 = measureDistance(TRIG2, ECHO2);

    Serial.printf("📏 Cảm biến 1: %.2f cm | Cảm biến 2: %.2f cm\n", distance1, distance2);
    
    delay(500);  // Chờ 500ms trước khi đo tiếp
}

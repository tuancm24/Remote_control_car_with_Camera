// 🔹 Kết nối Encoder
#define ENCODER_PIN 15  // Chân tín hiệu từ Encoder
#define WHEEL_CIRCUMFERENCE 0.2042  // Chu vi bánh xe TT Motor (m)
#define PULSES_PER_REV 20  // Số xung mỗi vòng bánh xe

// 🔹 Kết nối động cơ
#define IN1 5   // Điều khiển bánh trái tiến
#define IN2 18  // Điều khiển bánh trái lùi
#define IN3 19  // Điều khiển bánh phải tiến
#define IN4 23  // Điều khiển bánh phải lùi
#define ENA 32  // Kích hoạt động cơ trái
#define ENB 33  // Kích hoạt động cơ phải

volatile int pulseCount = 0;  
float distanceTraveled = 0.0;  
unsigned long lastTime = 0;

// 🔹 Hàm đếm xung Encoder
void IRAM_ATTR countPulse() {
    pulseCount++;
}

// 🔹 Hàm điều khiển xe chạy thẳng
void moveForward() {
    Serial.println("🚗 Xe đang chạy thẳng...");
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    digitalWrite(ENA, HIGH); digitalWrite(ENB, HIGH);
}

// 🔹 Hàm tính tốc độ & quãng đường
void calculateSpeedAndDistance() {
    unsigned long currentTime = millis();
    float timeInterval = (currentTime - lastTime) / 1000.0;

    if (timeInterval >= 1.0) {  
        float wheelRevolutions = pulseCount / (float)PULSES_PER_REV;  
        float speed_mps = (wheelRevolutions * WHEEL_CIRCUMFERENCE) / timeInterval;  
        float speed_kph = speed_mps * 3.6;  

        distanceTraveled += wheelRevolutions * WHEEL_CIRCUMFERENCE / 1000.0;  

        Serial.printf("🚗 Tốc độ: %.2f km/h | Quãng đường: %.3f km\n", speed_kph, distanceTraveled);
        
        pulseCount = 0;  
        lastTime = currentTime;
    }
}

void setup() {
    Serial.begin(115200);
    
    // 🔹 Cấu hình Encoder
    pinMode(ENCODER_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), countPulse, RISING);

    // 🔹 Cấu hình động cơ
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);

    lastTime = millis();

    // 🚗 **Bắt đầu chạy thẳng ngay khi khởi động**
    moveForward();
}

void loop() {
    calculateSpeedAndDistance();
}

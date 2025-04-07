#define BLYNK_TEMPLATE_ID "TMPL6-yyApoIE"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char auth[] = "r0HKsTyZ4UG8Yt59bGKMNNuCkh4PvQL6";
char ssid[] = "PRO192";
char pass[] = "12345678";

BlynkTimer timer;

// 🔹 Chân điều khiển động cơ
#define IN1 5
#define IN2 18
#define IN3 19
#define IN4 23
#define ENA 32  // Bật/Tắt động cơ trái
#define ENB 33  // Bật/Tắt động cơ phải

int xValue = 0, yValue = 0;

void connectWiFi() {
    WiFi.begin(ssid, pass);
    Serial.print("🔄 Đang kết nối WiFi");

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ WiFi kết nối thành công!");
    } else {
        Serial.println("\n❌ Không thể kết nối WiFi!");
    }
}

// 🔹 Hàm điều khiển xe với Joystick 360 độ
void moveCar(int x, int y) {
    // Tính độ lớn (magnitude) của vector Joystick
    float magnitude = sqrt(x * x + y * y);
    // Tính góc (độ) từ X và Y bằng atan2
    float angle = atan2(y, x) * 180 / PI;

    // Ngưỡng để xác định xe có di chuyển hay không
    const int threshold = 20;

    if (magnitude < threshold) {  // Dừng nếu Joystick gần trung tâm
        Serial.println("⏹️ Dừng lại");
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
        digitalWrite(ENA, LOW);
        digitalWrite(ENB, LOW);
        return;
    }

    // Điều khiển xe dựa trên góc
    if (angle > -45 && angle <= 45) {  // Rẽ phải
        Serial.println("➡️ Rẽ phải");
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
    }
    else if (angle > 45 && angle <= 135) {  // Tiến
        Serial.println("🔼 Tiến");
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
    }
    else if (angle > 135 || angle <= -135) {  // Rẽ trái
        Serial.println("⬅️ Rẽ trái");
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
    }
    else if (angle > -135 && angle <= -45) {  // Lùi
        Serial.println("🔽 Lùi");
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
    }

    // Bật động cơ khi xe di chuyển
    digitalWrite(ENA, HIGH);
    digitalWrite(ENB, HIGH);
}

// 🔹 Nhận dữ liệu X từ V0
BLYNK_WRITE(V0) {
    xValue = param.asInt();
    Serial.printf("🎮 X Value: %d\n", xValue);
}

// 🔹 Nhận dữ liệu Y từ V1
BLYNK_WRITE(V1) {
    yValue = param.asInt();
Serial.printf("🎮 Y Value: %d\n", yValue);
}

void setup() {
    Serial.begin(115200);
    connectWiFi();

    Blynk.config(auth);
    Blynk.connect();

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);

    timer.setInterval(100L, []() {
        moveCar(xValue, yValue);
    });
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("🔴 Mất kết nối WiFi! Đang thử lại...");
        connectWiFi();
        Blynk.connect();
    }

    Blynk.run();
    timer.run();
}
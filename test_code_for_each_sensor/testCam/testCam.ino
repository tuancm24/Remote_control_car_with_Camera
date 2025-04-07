#include "esp_camera.h"
#include <WiFi.h>

const char* ssid = "PRO192";
const char* password = "12345678";

#define CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


#define TX_PIN 14  // Chân TX của ESP32-CAM
#define RX_PIN 15  // Chân RX của ESP32-CAM



HardwareSerial mySerial(1);
void startCameraServer();

void connectWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("🔄 Đang kết nối WiFi...");

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ Kết nối WiFi thành công!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n❌ Không thể kết nối WiFi!");
    }
}

void setup() {
    Serial.begin(115200);
    mySerial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

    // Cấu hình camera
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound()) {
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("❌ Lỗi khởi tạo camera: 0x%x", err);
        return;
    }

    connectWiFi();

    startCameraServer();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("🔴 Mất kết nối WiFi! Đang thử lại...");
        connectWiFi();
    }

    String streamURL = "http://" + WiFi.localIP().toString() + ":81/stream";
    mySerial.println("STREAM:" + streamURL);
    Serial.println("📡 Gửi URL stream: " + streamURL);
    
    delay(10000);  // Gửi mỗi 10 giây để tránh mất kết nối
}

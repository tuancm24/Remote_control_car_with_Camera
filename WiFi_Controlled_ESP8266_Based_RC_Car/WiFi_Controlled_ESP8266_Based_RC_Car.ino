#define BLYNK_TEMPLATE_ID "TMPL6-yyApoIE"
#define BLYNK_TEMPLATE_NAME "Quickstart Device"
#define BLYNK_AUTH_TOKEN "9gK32Q1v2M6W9iO4l3xIjIfn_LDcBoYc"

#define BLYNK_PRINT Serial
#define BLYNK_NO_INFO  // Tắt debug để tiết kiệm bộ nhớ

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = "@@@";
char pass[] = "21102004";

BlynkTimer timer;

// Xử lý khi nhận dữ liệu từ Virtual Pin V0
BLYNK_WRITE(V0) {
  int value = param.asInt();
  Blynk.virtualWrite(V1, value);
}

// Xử lý khi thiết bị kết nối thành công với Blynk Cloud
BLYNK_CONNECTED() {
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

// Gửi thời gian hoạt động (uptime) lên Virtual Pin V2 mỗi giây
void myTimerEvent() {
  Blynk.virtualWrite(V2, millis() / 1000);
}

// Kết nối WiFi với timeout
void connectWiFi() {
  WiFi.begin(ssid, pass);
  int timeout = 15;  // Thời gian chờ tối đa là 15 giây
  Serial.print("Đang kết nối WiFi...");
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(1000);
    Serial.print(".");
    timeout--;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi đã kết nối!");
  } else {
    Serial.println("\nKết nối WiFi thất bại!");
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Bắt đầu khởi động ESP8266...");

  connectWiFi();  // Kết nối WiFi trước

  if (WiFi.status() == WL_CONNECTED) {
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
  }

  timer.setInterval(1000L, myTimerEvent);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi bị mất! Đang kết nối lại...");
    connectWiFi();
  }

  if (!Blynk.connected()) {
    Serial.println("Blynk bị mất kết nối! Đang kết nối lại...");
    Blynk.connect();
  }

  Blynk.run();
  timer.run();
}

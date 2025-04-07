#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>

WiFiManager wm;  // Tạo WiFiManager
AsyncWebServer server(80);  // Tạo Web Server trên cổng 80

void setup() {
    Serial.begin(115200);

    // Kiểm tra nếu có WiFi đã lưu thì kết nối
    if (!wm.autoConnect("ESP32-Config")) {
        Serial.println("⚠️ Không thể kết nối WiFi!");
        ESP.restart();
    } else {
        Serial.println("✅ Kết nối WiFi thành công!");
    }

    // 🖥 Tạo trang web chính với tiêu đề (Title)
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Lấy địa chỉ IP của ESP32
        String ipAddress = WiFi.localIP().toString();

        // Tạo HTML hiển thị địa chỉ IP
        String html = "<!DOCTYPE html><html><head>";
        html += "<title>ESP32 WiFi Manager</title>";  // Tiêu đề trên tab trình duyệt
        html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
        html += "<style> body { font-family: Arial, sans-serif; text-align: center; } ";
        html += "button { padding: 10px 20px; margin: 10px; border: none; cursor: pointer; } ";
        html += ".reset { background: red; color: white; } ";
        html += ".disconnect { background: orange; color: white; } ";
        html += "</style></head><body>";
        html += "<h2>ESP32 WiFi Manager</h2>";
        html += "<p><b>IP Address:</b> " + ipAddress + "</p>";  // Hiển thị địa chỉ IP
        html += "<p><b>SSID:</b> " + WiFi.SSID() + "</p>";
        html += "<p><button class='reset' onclick=\"window.location.href='/reset'\">Reset WiFi</button></p>";
        html += "<p><button class='disconnect' onclick=\"window.location.href='/disconnect'\">Ngắt Kết Nối WiFi</button></p>";
        html += "</body></html>";

        // Gửi trang web với thông tin Wi-Fi và địa chỉ IP
        request->send(200, "text/html", html);
    });

    // 🔄 API Reset WiFi (Xóa WiFi đã lưu)
    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "🛑 Đa reset WiFi! ESP32 se khoi đong lai...");
        wm.resetSettings();  // Xóa WiFi đã lưu
        delay(1000);
        ESP.restart();  // Khởi động lại ESP để nhập WiFi mới
    });

    // 🔌 API Ngắt Kết Nối WiFi (Mất kết nối nhưng không reset)
    server.on("/disconnect", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "⚠️ Da ngat ket noi thanh cong WiFi!");
        WiFi.disconnect(true, true);  // Ngắt kết nối WiFi nhưng không reset
    });

    server.begin();  // Bắt đầu Web Server
}

void loop() {}

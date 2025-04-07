#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebServer.h>
#include <Adafruit_INA219.h>
WebServer server(80);
const char* ssid = "PRO192";
const char* password = "12345678";

WebSocketsServer webSocket = WebSocketsServer(81);

#define TRIG1 26
#define ECHO1 14
#define TRIG2 13
#define ECHO2 27

#define ENCODER_PIN 15
#define WHEEL_CIRCUMFERENCE 0.2198 // Chu vi = 21,98cm
#define PULSES_PER_REV // 20 Đây là số xung mà cảm biến phát ra mỗi khi bánh xe quay một vòng hoàn chỉnh.



#define IN1 5
#define IN2 18
#define IN3 19
#define IN4 23
#define ENA 32
#define ENB 33

#define BUZZER_PIN 16  // Chân còi báo động

volatile int pulseCount = 0; // Đếm xung
float distanceTraveled = 0.0; // Quãng đường đã di chuyển
unsigned long lastTime = 0; // Thời gian lần gọi hàm trước đó
float lastSpeed = 0; // Tốc độ cuối cùng đo được

void IRAM_ATTR countPulse() {
  pulseCount++; // Đây là một hàm được gọi mỗi khi có một xung được phát hiện
                // Hàm này được đánh dấu với IRAM_ATTR, điều này giúp tối ưu hóa hàm để chạy trong bộ nhớ IRAM (bộ nhớ có thể truy xuất nhanh hơn trên ESP32).
}

float getSpeed() {
   // Hàm này tính toán tốc độ của bánh xe dựa trên số xung đã đếm và thời gian trôi qua.
  unsigned long currentTime = millis(); // Lấy thời gian hiện tại tính bằng mili giây kể từ khi Arduino bắt đầu chạy.
                                        // millis() là một hàm của Arduino trả về số mili giây đã trôi qua từ khi chương trình bắt đầu chạy (kể từ lúc Arduino reset hoặc bật nguồn).
  float timeInterval = (currentTime - lastTime) / 1000.0;
  // Tính khoảng thời gian trôi qua giữa lần gọi hàm trước và lần gọi hiện tại (tính bằng giây). lastTime là thời gian của lần gọi trước đó.

  if (timeInterval >= 1.0) { // Nếu khoảng thời gian đã đạt ít nhất 1 giây, thì thực hiện tính toán tốc độ.
    float wheelRevolutions = pulseCount / (float)PULSES_PER_REV;
        // Số vòng quay được của bánh xe = Số lần nhận được xung trong 1 khoảng thời gian / số xung cố định khi quay hết 1 vòng bánh xe (20 xung)
    
    float speed_mps = (wheelRevolutions * WHEEL_CIRCUMFERENCE) / timeInterval; 
        // Tính tốc độ của bánh xe theo mét mỗi giây (m/s) bằng cách lấy số vòng quay nhân với chu vi bánh xe và chia cho thời gian.

    float speed_cms = speed_mps * 100;
        // Quy sang cm/s
    pulseCount = 0; // Đặt lại số xung đếm về 0 để đếm lại từ đầu cho lần tính toán tiếp theo.
    lastTime = currentTime; // Cập nhật thời gian hiện tại thành lastTime để sử dụng cho lần tính toán tiếp theo.
    lastSpeed = speed_cms; // Lưu tốc độ tính được vào biến lastSpeed để có thể trả về nếu chưa đủ 1 giây.

    return speed_cms; // Trả về tốc độ tính được (cm/s).

    /*
    Khi getSpeed() được gọi:

    Nếu khoảng thời gian đã trôi qua đủ 1 giây (timeInterval >= 1.0), hàm sẽ tính toán tốc độ mới và lưu giá trị vào biến lastSpeed.

    Sau khi tính toán tốc độ mới và lưu vào lastSpeed, biến lastSpeed sẽ chứa giá trị tốc độ của lần tính toán gần nhất.

    Khi getSpeed() được gọi lại lần nữa trước khi đủ 1 giây:
    
    */ 

  }
  return lastSpeed;
  /*
  Câu lệnh return lastSpeed; đảm bảo rằng khi không đủ 1 giây để tính toán tốc độ mới, giá trị tốc độ trước đó (lưu trong lastSpeed) sẽ được trả về, giúp duy trì tính ổn định và tránh tính toán lại không cần thiết.
  
  Nếu chưa đủ 1 giây (tức là timeInterval < 1.0), hàm sẽ không tính toán tốc độ mới mà sẽ trả về giá trị tốc độ đã lưu trong lastSpeed từ lần gọi trước đó.
    Tối ưu hóa tài nguyên: tiết kiệm tài nguyên và giảm tải cho vi xử lý.
    Giữ giá trị ổn định: khi không có đủ thời gian để tính toán lại tốc độ, trả về giá trị đã lưu giúp duy trì giá trị ổn định, tránh việc thay đổi tốc độ không cần thiết.
  */

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
// webSocketEvent là một hàm callback (hàm sự kiện) được gọi mỗi khi một sự kiện WebSocket xảy ra.
/*
uint8_t num: Chỉ số kết nối WebSocket (nếu có nhiều kết nối WebSocket, num giúp phân biệt các kết nối khác nhau).

WStype_t type: Kiểu của dữ liệu nhận được (WebSocket có thể nhận nhiều loại dữ liệu khác nhau, ví dụ như text, binary).

uint8_t* payload: Dữ liệu mà client gửi qua WebSocket. Đây là một con trỏ tới mảng byte chứa dữ liệu.

size_t length: Độ dài của dữ liệu nhận được (tính bằng byte).
*/

/*
WebSocket có thể gửi các loại dữ liệu khác nhau như text (văn bản), binary (dữ liệu nhị phân), hoặc các dạng khác, và WStype_TEXT là loại dữ liệu văn bản (text).

Nếu kiểu dữ liệu là text, thì đoạn mã bên trong điều kiện if sẽ được thực thi.
Dòng mã này tạo ra một chuỗi văn bản từ dữ liệu nhận được và gán nó cho biến command.
*/
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

float dung_luong_ban_dau = 4000.0;  // Dung lượng pin ban đầu (mAh)
float V_pin_max = 8.4;  // Điện áp đầy của pin 2P2S (tối đa)
// 1 pin thông số mặc định là 3.7V. Khi mà sạc đầy sẽ là 4.2V 


int tinhPhanTramPin(float V_pin, float V_pin_max, float dung_luong_ban_dau) {
  float dung_luong_con_lai = dung_luong_ban_dau * (V_pin / V_pin_max);
  float phan_tram_con_lai = (dung_luong_con_lai / dung_luong_ban_dau) * 100.0;
  return round(phan_tram_con_lai);  // Làm tròn phần trăm
}



void sendSensorData() {
    // Sensor khoảng cách trước
  float frontDistance = getDistance(TRIG1, ECHO1); // Trig nhận tín hiệu ESP32, rồi Trig để kích hoạt cảm biến phát sóng siêu âm tới vật cản. Echo là chân nhận sau khi tín hiệu xung gặp vật cản
    // Sensor khoảng cách sau
  float rearDistance = getDistance(TRIG2, ECHO2);  

  float V_pin = ina219.getBusVoltage_V();  // Điện áp pin thực tế

  int batteryLevel = tinhPhanTramPin(V_pin, V_pin_max, dung_luong_ban_dau);  // Tính phần trăm pin
  float speed = getSpeed();
  float distance = getDistance();

  String jsonData = "{\"speed\":" + String(speed) + 
                    ", \"distance\":" + String(distance) + 
                    ", \"battery\":" + String(tinhPhanTramPin) + 
                    ", \"frontDistance\":" + String(frontDistance) + 
                    ", \"rearDistance\":" + String(rearDistance) + "}";

  webSocket.broadcastTXT(jsonData);
}
/*
Tạo chuỗi JSON: Tạo một chuỗi JSON chứa thông tin về tốc độ (speed), quãng đường (distance), pin (battery), khoảng cách phía trước (frontDistance), và khoảng cách phía sau (rearDistance).

Gửi dữ liệu qua WebSocket: Sử dụng WebSocket để gửi chuỗi JSON tới tất cả các client đang kết nối, giúp truyền tải thông tin thời gian thực.
*/

float getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2); // Tạm tắt 2 micro giây . Có delay vì để nó không xung đột khi gửi liên tục mà không theo đợt, như thế sensor sẽ không hoạt động đúng. Mà nó sẽ gửi tín hiệu từng đợt để chân Echo nhận từng đợt 
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); // Sau 10 micro giây sẽ tắt. 
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2; // 1 m/s = 100 cm/s (vì 1 mét = 100 cm).
                              // 1 giây = 1,000,000 micro giây. Nhân 100 rồi chia 1000000 là ra

}




void setup() {
  Serial.begin(115200); // Tốc độ truyền dữ liệu) được thiết lập, đo bằng bit mỗi giây (bps).  Phải sử dụng cùng một baud rate để có thể truyền nhận dữ liệu đúng đắn.
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
    tone(BUZZER_PIN, 700);  // Còi cảnh báo .  Tần số tối đa thường nằm trong khoảng từ 4 kHz đến 5 kHz
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

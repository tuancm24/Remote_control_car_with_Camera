#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

// Thông số pin của bạn (tùy chỉnh theo dung lượng pin thực tế)
float dung_luong_ban_dau = 2200.0;  // Dung lượng pin ban đầu (mAh)
float V_pin_max = 8.4;  // Điện áp đầy của pin 2P2S (tối đa)

void setup() {
  Serial.begin(115200);
  
  if (!ina219.begin()) {
    Serial.println("Không tìm thấy INA219, kiểm tra kết nối!");
    while (1);
  }
  Serial.println("INA219 đã sẵn sàng.");
}

void loop() {
  float V_pin = ina219.getBusVoltage_V();  // Điện áp pin
  float I_load = ina219.getCurrent_mA() / 1000.0;  // Dòng điện tiêu thụ (A)
  
  // Tính dung lượng còn lại theo tỷ lệ điện áp (đơn giản hóa)
  float dung_luong_con_lai = dung_luong_ban_dau * (V_pin / V_pin_max);

  // Tính thời gian sử dụng còn lại (giờ)
  float thoi_gian_con_lai = dung_luong_con_lai / (I_load * 1000.0);

  // Tính phần trăm dung lượng còn lại và làm tròn đến số nguyên
  float phan_tram_con_lai = (dung_luong_con_lai / dung_luong_ban_dau) * 100.0;
  int phan_tram_tron = round(phan_tram_con_lai);  // Làm tròn phần trăm

  // Hiển thị kết quả
  Serial.println("==== Dữ liệu PIN ====");
  Serial.print("Phần trăm dung lượng còn lại: "); Serial.print(phan_tram_tron); Serial.println(" %");
  Serial.print("Thời gian sử dụng còn lại: "); Serial.print(thoi_gian_con_lai); Serial.println(" giờ");
  Serial.println("----------------------");

  delay(1000);  // Đọc mỗi giây
}
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <driver/i2s.h>
#include <Audio.h>
#include <Adafruit_GFX.h>    // Thư viện đồ họa cơ bản
// #include <Adafruit_ST7789.h> // Thư viện cho ST7735
#include <SPI.h>
// #include "qrCodeImage.h"


// Thông tin kết nối WiFi và API
const char* ssid = "DTM E-SMART";
const char* password = "0919890938";
String api_token = "myAPIToken"; // Thay thế bằng API token của bạn
String accountNumber = "myAccountNumber";//Thay thế bằng account của bạn

// Cấu hình I2S
#define I2S_DOUT 27 // Thay đổi chân tùy theo thiết bị của bạn
#define I2S_BCLK 26
#define I2S_LRC  25

// // Cấu hình chân TFT ST7735
// #define TFT_CS    5  // Chân CS của TFT
// #define TFT_RST   15 // Chân Reset của TFT
// #define TFT_DC    4  // Chân DC (Data/Command) của TFT

// Định nghĩa chân button
const int buttonSpeak = 13;

// Biến toàn cục
Audio audio;
unsigned long timeGetData = 0;
String transactionDate = "";
long amountIn = 0;

// Thông tin tài khoản
String accountName = "VO VAN HON";  // Tên tài khoản
String bankName = "MBBANK";         // Tên ngân hàng
bool isSpeaking = false; // Biến cờ kiểm tra xem loa có đang phát không

// Khởi tạo đối tượng TFT
// Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Functions interface
void displayTransactions(JsonArray transactions);
String formatNumber(long number); // Hàm định dạng số

// put function declarations here:

// Hàm định dạng số với dấu phẩy phân cách hàng ngàn
String formatNumber(long number) {
  String numStr = String(number);
  String formatted = "";
  int len = numStr.length();
  
  for (int i = 0; i < len; i++) {
    formatted += numStr[i];
    if ((len - i - 1) % 3 == 0 && i < len - 1) {
      formatted += ",";
    }
  }
  return formatted;
}

// Hàm hiển thị các phần cố định (tiêu đề, QR code, thông tin tài khoản) - chỉ gọi một lần
void displayStaticContent() {
  // tft.fillScreen(ST77XX_BLACK); // Xóa màn hình một lần duy nhất

  // // Hiển thị tiêu đề "LOA BAO CHUYEN KHOAN" canh giữa toàn màn hình
  // tft.setTextSize(2); // Kích thước chữ lớn cho tiêu đề
  // tft.setTextColor(ST77XX_YELLOW); // Màu trắng cho tiêu đề
  // String title = "LOA BAO CHUYEN KHOAN";
  // int titleWidth = title.length() * 12; // Ước tính chiều rộng tiêu đề
  // int titleX = (320 - titleWidth) / 2; // Canh giữa trên màn hình 320 pixel
  // tft.setCursor(titleX, 5); // Đặt tiêu đề ở y=5
  // tft.println(title);

  // // Hiển thị ảnh QR code bên trái, dưới tiêu đề
  // int headerHeight = 30; // Chiều cao phần tiêu đề
  // tft.drawRGBBitmap(0, headerHeight, qrCodeImage, 150, 150); // Vẽ ảnh tại x=0, y=headerHeight

  // // Hiển thị thông tin tài khoản nhận chuyển khoản dưới QR code
  // int accountStartY = 182; // Bắt đầu ngay dưới QR code để tránh chồng lấn

  // // Nhãn "TEN TK" (cỡ nhỏ, canh giữa) và accountName (cỡ lớn, canh giữa)
  // tft.setTextSize(1); // Chữ nhỏ cho nhãn
  // tft.setTextColor(ST77XX_WHITE); // Màu trắng
  // int labelNameWidth = 6 * 6; // "TEN TK" dài 36 pixel
  // tft.setCursor((150 - labelNameWidth) / 2, accountStartY); // Canh giữa trong 150 pixel
  // tft.println("TEN TK");
  
  // tft.setTextSize(2); // Chữ lớn cho nội dung
  // tft.setTextColor(ST77XX_YELLOW); // Màu trắng
  // int nameWidth = accountName.length() * 12; // Độ dài của accountName (12 pixel/chữ)
  // tft.setCursor((150 - nameWidth) / 2, accountStartY + 12); // Canh giữa, cách nhãn 12 pixel
  // tft.println(accountName); // Sử dụng biến accountName

  // // Nhãn "NGAN HANG" (cỡ nhỏ, canh giữa) và bankName (cỡ lớn, canh giữa)
  // tft.setTextSize(1); // Chữ nhỏ cho nhãn
  // tft.setTextColor(ST77XX_WHITE); // Màu vàng
  // int labelBankWidth = 9 * 6; // "NGAN HANG" dài 54 pixel
  // tft.setCursor((150 - labelBankWidth) / 2, accountStartY + 32); // Cách dòng đầu 37 pixel
  // tft.println("NGAN HANG");
  
  // tft.setTextSize(2); // Chữ lớn cho nội dung
  // tft.setTextColor(ST77XX_GREEN); // Màu vàng
  // int bankWidth = bankName.length() * 12; // Độ dài của bankName (12 pixel/chữ)
  // tft.setCursor((150 - bankWidth) / 2, accountStartY + 44); // Canh giữa, cách nhãn 12 pixel
  // tft.println(bankName); // Sử dụng biến bankName
}

// Hàm chỉ cập nhật 5 ô giao dịch bên phải
void displayTransactions(JsonArray transactions) {
  // int boxHeight = 42;           // Chiều cao mỗi ô
  // int headerHeight = 30;        // Chiều cao phần tiêu đề

  // // Xóa vùng bên phải từ dưới tiêu đề (y=30) đến đáy màn hình (y=240)
  // tft.fillRect(160, headerHeight, 160, 240 - headerHeight, ST77XX_BLACK); // Xóa 160x210 pixel

  // // Hiển thị 5 ô giao dịch
  // for (int i = 0; i < min(5, (int)transactions.size()); i++) {
  //   JsonObject transaction = transactions[i];
  //   String date = transaction["transaction_date"].as<String>();
  //   long amount = transaction["amount_in"].as<long>(); // Làm tròn số nguyên

  //   // Tô màu nền cho giao dịch gần nhất (i = 0)
  //   if (i == 0) {
  //     tft.fillRect(160, headerHeight, 160, boxHeight, ST77XX_BLUE); // Ô đầu tiên
  //     tft.drawRect(160, headerHeight, 160, boxHeight, ST77XX_WHITE); // Viền trắng
  //   } else {
  //     tft.drawRect(160, headerHeight + i * boxHeight, 160, boxHeight, ST77XX_WHITE); // Các ô khác
  //   }

  //   // Vẽ vòng tròn và số thứ tự bên trong
  //   int circleX = 170; // Tọa độ x của tâm vòng tròn
  //   int circleY = headerHeight + i * boxHeight + 10; // Tọa độ y của tâm vòng tròn
  //   tft.fillCircle(circleX, circleY, 8, ST77XX_YELLOW); // Vòng tròn màu vàng, bán kính 8
  //   tft.setTextSize(1);
  //   tft.setTextColor(ST77XX_BLACK); // Màu đen cho số để nổi trên nền vàng
  //   tft.setCursor(circleX - 3, circleY - 3); // Canh giữa số trong vòng tròn
  //   tft.print(i + 1);

  //   // Hiển thị ngày giờ (căn giữa ngang với vòng tròn)
  //   tft.setTextColor(ST77XX_WHITE);
  //   tft.setCursor(182, circleY - 3); // Căn y của ngày với tâm vòng tròn (circleY - 3 để chữ cân đối)
  //   tft.println(date);

  //   // Hiển thị số tiền (dịch sang trái)
  //   tft.setTextSize(2); // Chữ lớn cho ký tự "$" và số tiền
  //   tft.setTextColor(ST77XX_YELLOW); // Màu vàng cho "$"
  //   tft.setCursor(170, headerHeight + i * boxHeight + 22); // Giữ nguyên vị trí số tiền
  //   tft.print("$"); // Ký tự "$" màu vàng
    
  //   tft.setTextColor(ST77XX_GREEN); // Màu xanh cho số tiền
  //   tft.setCursor(182, headerHeight + i * boxHeight + 22); // Giữ nguyên vị trí số tiền
  //   tft.print(formatNumber(amount)); // Giá trị số tiền với dấu phẩy
  // }

  // // Vẽ đường kẻ ngang phân chia
  // tft.drawFastHLine(160, headerHeight + boxHeight, 160, ST77XX_WHITE); // Đường 1
  // tft.drawFastHLine(160, headerHeight + boxHeight * 2, 160, ST77XX_WHITE); // Đường 2
  // tft.drawFastHLine(160, headerHeight + boxHeight * 3, 160, ST77XX_WHITE); // Đường 3
  // tft.drawFastHLine(160, headerHeight + boxHeight * 4, 160, ST77XX_WHITE); // Đường 4
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);

  pinMode(buttonSpeak, INPUT_PULLUP);

  // Khởi tạo màn hình TFT
  // tft.init(240, 320);
  // tft.invertDisplay(false);
  // tft.setRotation(1);        // Xoay màn hình ngang
  // tft.fillScreen(ST77XX_BLACK); // Xóa màn hình ban đầu

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // displayStaticContent();

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(30);
  audio.connecttospeech("Xin chào, đây là loa thông báo chuyển khoản tự động! Hệ thống đã khởi động xong!", "vi");

  Serial.println("Nhập số tiền (ví dụ: 15000000):");
  timeGetData = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    long number = input.toInt();
    Serial.print("Đọc số: ");
    Serial.println(number);
    String str = "Thanh toán thành công, đã nhận "+String(number)+" đồng";
    isSpeaking = true; // Đặt cờ khi bắt đầu phát
    audio.connecttospeech(str.c_str(), "vi");
    Serial.println("Nhập số tiền mới:");
  }

  if (!isSpeaking && millis() - timeGetData > 6000) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String apiUrl = "https://my.sepay.vn/userapi/transactions/list?account_number=" + accountNumber + "&limit=5";
      http.begin(apiUrl);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Authorization", "Bearer " + String(api_token));
      
      int httpCode = http.GET();
      Serial.println("HTTP CODE: " + String(httpCode));
      
      if (httpCode > 0) {
        String payload = http.getString();
        Serial.println("Received JSON:");
        Serial.println(payload);
        
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
          Serial.print("JSON Parsing failed: ");
          Serial.println(error.f_str());
          return;
        }
        
        JsonArray transactions = doc["transactions"].as<JsonArray>();
        for (JsonObject transaction : transactions) {
          Serial.println("----------------------");
          Serial.print("Transaction Date: ");
          Serial.println(transaction["transaction_date"].as<String>());
          Serial.print("Amount In: ");
          Serial.println(transaction["amount_in"].as<String>());
          Serial.print("Transaction Content: ");
          Serial.println(transaction["transaction_content"].as<String>());
          Serial.print("Code: ");
          Serial.println(transaction["code"].as<String>());
        }

        JsonObject transaction = transactions[0];
        if (transaction["transaction_date"].as<String>() != transactionDate) {
          // Hiển thị giao dịch lên TFT
          displayTransactions(transactions);
          amountIn = transaction["amount_in"].as<long>();
          if(transactionDate!=""){
            String str = "Thanh toán thành công, đã nhận "+String(amountIn)+" đồng";
            isSpeaking = true; // Đặt cờ khi phát âm thanh
            audio.connecttospeech(str.c_str(), "vi");
          }
          transactionDate = transaction["transaction_date"].as<String>();
        }
      } else {
        Serial.println("Error on HTTP request");
      }
      http.end();
    }
    timeGetData = millis();
  }

  if (digitalRead(buttonSpeak) == HIGH) {
    String str = "Thanh toán thành công, đã nhận "+String(amountIn)+" đồng";
    isSpeaking = true; // Đặt cờ khi bắt đầu phát
    audio.connecttospeech(str.c_str(), "vi");
  }

  audio.loop();
  if (!audio.isRunning()) {
    isSpeaking = false; // Đặt lại cờ khi âm thanh dừng
  }
}

// put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }
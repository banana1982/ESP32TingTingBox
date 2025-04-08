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

void setup() {
  // put your setup code here, to run once:
  // int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }
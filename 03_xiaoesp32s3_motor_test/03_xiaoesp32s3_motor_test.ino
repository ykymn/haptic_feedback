#include <WiFi.h> // Wi-Fiライブラリ

// Wi-Fi情報
const char* ssid = "Living-Lab_2.4";
const char* password = "livinglab";

void setup() {
  // シリアルモニタを初期化
  Serial.begin(115200);
  delay(1000);

  // Wi-Fiに接続
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  // 接続が完了するまで待機
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  // 接続完了後の処理
  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // ここでは何もしない
}

#include <WiFi.h>       // WiFi接続を管理するためのライブラリ
#include <WiFiUdp.h>    // UDP通信を行うためのライブラリ

// 定義するピン番号（M5StampS3のGPIOピンを使用）
const int motor_pin = 33;  // M5StampS3で使用するモーターピン (GPIO 33に設定)
int freq            = 10000;   // PWMの周波数（モーター制御用）
int ledChannel      = 2;       // PWM制御用のチャンネル番号
int resolution      = 10;      // PWMの解像度（ビット数）

const char* ssid = "Living-Lab_2.4";        // WiFiのSSID
const char* password = "livinglab"; // WiFiのパスワード
const int localPort = 8888;        // UDPサーバーがリッスンするポート番号

int receivedNumber = 0;            // UDPで受信した数値を保持する変数

WiFiUDP udp; // UDP通信を扱うオブジェクト

// 振動の強さを定義する列挙型
enum Vibrate_mode_t {
    stop = 0,
    weak,
    medium,
    strong,
    very_strong,
    strongest,
};
static uint8_t mode = stop;  // 現在の振動モードを保持する変数

// 振動モーターを制御する関数
void Vibrator_update() {
    ledcWrite(ledChannel, receivedNumber);  //チャンネルに receivednumber/1024 の値を出力
    Serial.printf("PWM: %4d%%\\n", receivedNumber);  // 現在のPWM値をシリアルモニタに表示
}

void setup() {
    Serial.begin(115200);
    delay(100);

    // Wi-Fiに接続
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    // WiFiが接続されるまで待機
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print("Attempting to connect... Status: ");
      Serial.println(WiFi.status());  // WiFi接続ステータスを表示
    }
    Serial.println("");
    Serial.println("WiFi connected");

    // 割り当てられたIPアドレスをシリアルモニタに表示
    Serial.print("Assigned IP: ");
    Serial.println(WiFi.localIP());

    // UDPサーバーを開始
    udp.begin(localPort);
    Serial.println("UDP Server started");

    // PWMの設定を行う
    ledcSetup(ledChannel, freq, resolution); //チャンネル名, 周波数, 解像度=10bit → 1024段階
    ledcAttachPin(motor_pin, ledChannel);

    Vibrator_update();
}

void loop() {
  // UDPパケットの受信処理
  int packetSize = udp.parsePacket(); // 受信したパケットのサイズを取得
  if (packetSize) {  // パケットが受信された場合のみ処理を実行
    char packetBuffer[255];  // 受信データを保持するバッファ
    udp.read(packetBuffer, packetSize); // パケットを読み込む
    packetBuffer[packetSize] = '\\0';  // 文字列の終端を追加

    // 受信したデータを数値に変換
    receivedNumber = atoi(packetBuffer); // 受信データを整数値に変換し、振動の強さに反映
    Vibrator_update(); // 振動を更新
  }

  delay(5);  // 短い待機時間を設定
}

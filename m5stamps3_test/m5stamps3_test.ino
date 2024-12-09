#include <WiFi.h>       // WiFi接続を管理するためのライブラリ
#include <WiFiUdp.h>    // UDP通信を行うためのライブラリ

const char* ssid = "Living-Lab_2.4";        // WiFiのSSID
const char* password = "livinglab";         // WiFiのパスワード
const int localPort = 8888;                 // UDPサーバーがリッスンするポート番号

int receivedNumber = 0;                     // UDPで受信した数値を保持する変数

WiFiUDP udp; // UDP通信を扱うオブジェクト

const int ledRedPin = 2;      // 内蔵LEDのRedチャンネル（例としてGPIO 2に接続されていると仮定）
const int ledGreenPin = 4;    // 内蔵LEDのGreenチャンネル（例としてGPIO 4に接続）
const int ledBluePin = 15;    // 内蔵LEDのBlueチャンネル（例としてGPIO 15に接続）

// LEDの色を設定する関数
void setLedColor(int value) {
    int red = 0, green = 0, blue = 0;

    if (value >= 0 && value < 200) {
        blue = 255; // 青色 (0-199)
    } else if (value >= 200 && value < 400) {
        green = 255; // 緑色 (200-399)
    } else if (value >= 400 && value < 600) {
        red = 255; green = 255; // 黄色 (400-599)
    } else if (value >= 600 && value < 800) {
        red = 255; green = 165; // オレンジ色 (600-799)
    } else if (value >= 800) {
        red = 255; // 赤色 (800以上)
    }
    
    // PWM出力でRGBの強さを調整
    ledcWrite(0, red);
    ledcWrite(1, green);
    ledcWrite(2, blue);
}

void setup() {
    Serial.begin(115200);
    delay(100);

    // LED PWMチャネルのセットアップ
    ledcSetup(0, 5000, 8); // チャンネル0、周波数5000Hz、8ビット解像度
    ledcAttachPin(ledRedPin, 0);
    ledcSetup(1, 5000, 8); // チャンネル1
    ledcAttachPin(ledGreenPin, 1);
    ledcSetup(2, 5000, 8); // チャンネル2
    ledcAttachPin(ledBluePin, 2);

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

    setLedColor(0); // 初期色をオフに設定
}

void loop() {
    // UDPパケットの受信処理
    int packetSize = udp.parsePacket(); // 受信したパケットのサイズを取得
    if (packetSize) {  // パケットが受信された場合のみ処理を実行
        char packetBuffer[255];  // 受信データを保持するバッファ
        udp.read(packetBuffer, packetSize); // パケットを読み込む
        packetBuffer[packetSize] = '\0';  // 文字列の終端を追加

        // 受信したデータを数値に変換
        receivedNumber = atoi(packetBuffer); // 受信データを整数値に変換
        setLedColor(receivedNumber); // 受信した数値に応じてLEDの色を設定

        Serial.printf("Received Number: %4d\n", receivedNumber);  // 現在の受信数値をシリアルモニタに表示
    }

    delay(5);  // 短い待機時間を設定
}

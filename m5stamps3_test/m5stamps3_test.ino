#include <WiFi.h>       // WiFi接続を管理するためのライブラリ
#include <WiFiUdp.h>    // UDP通信を行うためのライブラリ
#include <FastLED.h>    // LED制御用のライブラリ

#define PIN_LED    21   // 本体フルカラーLEDの使用端子（G21）
#define NUM_LEDS   1    // 本体フルカラーLEDの数

const char* ssid = "Living-Lab_2.4";        // WiFiのSSID
const char* password = "livinglab";         // WiFiのパスワード
const int localPort = 8888;                 // UDPサーバーがリッスンするポート番号

int receivedNumber = 0;                     // UDPで受信した数値を保持する変数
WiFiUDP udp;                                // UDP通信を扱うオブジェクト
CRGB leds[NUM_LEDS];                        // LED配列

// LEDの色を設定する関数
void setLedColor(int value) {
    if (value >= 0 && value < 200) {
        leds[0] = CRGB::Blue; // 青色 (0-199)
    } else if (value >= 200 && value < 400) {
        leds[0] = CRGB::Green; // 緑色 (200-399)
    } else if (value >= 400 && value < 600) {
        leds[0] = CRGB::Yellow; // 黄色 (400-599)
    } else if (value >= 600 && value < 800) {
        leds[0] = CRGB::Orange; // オレンジ色 (600-799)
    } else if (value >= 800) {
        leds[0] = CRGB::Red; // 赤色 (800以上)
    }
    FastLED.show();  // LEDに色を反映
}

void setup() {
    Serial.begin(115200);
    delay(100);

    // LEDの初期化
    FastLED.addLeds<WS2812B, PIN_LED, GRB>(leds, NUM_LEDS);
    leds[0] = CRGB::Black; // 初期色は消灯
    FastLED.show();

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
        Serial.printf("Received Number: %4d\n", receivedNumber);  // 現在の受信数値をシリアルモニタに表示

        // 受信した数値に応じてLEDの色を設定
        setLedColor(receivedNumber);
    }

    delay(5);  // 短い待機時間を設定
}

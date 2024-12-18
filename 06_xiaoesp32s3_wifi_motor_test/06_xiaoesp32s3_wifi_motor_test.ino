#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// Wi-Fi情報
const char* ssid = "Living-Lab_2.4";
const char* password = "livinglab";
const int localPort = 8888; // UDPサーバーがリッスンするポート番号
WiFiUDP udp;

// モータードライバピン
const int motorPinA = 4;   // ドライバの入力Aピン（PWM）
const int motorPinB = 3;   // ドライバの入力Bピン（PWM）
const int encoderPinA = 2; // エンコーダピンA
const int encoderPinB = 1; // エンコーダピンB

// エンコーダパラメータ
const int PULSES_PER_REVOLUTION = 3;  // エンコーダの1回転のパルス数

// エンコーダカウント
volatile long encoderCount = 0;   // エンコーダのカウント
unsigned long prevTime = 0;       // 前回の時間
float rpm = 0.0;                  // 回転数
int currentSpeed = 0;             // 現在の速度
int receivedNumber = 0;           // 受信データ

// エンコーダ割り込みハンドラ
void IRAM_ATTR encoderISR() {
  int stateA = digitalRead(encoderPinA);
  int stateB = digitalRead(encoderPinB);
  encoderCount += (stateA == stateB) ? 1 : -1;
}

void setup() {
  // シリアル通信を初期化
  Serial.begin(115200);
  while (!Serial) {
    ; // シリアルポートが開くのを待つ
  }

  // モーターピンを初期化
  ledcAttach(motorPinA, 500, 8);  // 500Hz, 8ビット解像度
  ledcAttach(motorPinB, 500, 8);  // 500Hz, 8ビット解像度

  // エンコーダピンを初期化
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  // 割り込みを設定（両方のピンで割り込み）
  attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), encoderISR, CHANGE);

  // Wi-Fi接続
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // UDPサーバーの初期化
  udp.begin(localPort);
  Serial.println("UDP Server started");

  Serial.println("Setup complete. Ready to receive commands.");
}

void loop() {
  // UDPパケットの受信処理
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char packetBuffer[255];
    udp.read(packetBuffer, packetSize);
    packetBuffer[packetSize] = '\0';
    receivedNumber = atoi(packetBuffer);
    
    // モータ制御関数の呼び出し
    handleCommand(receivedNumber);
  }

  // エンコーダ情報を1秒ごとに計算・表示
  unsigned long currentTime = millis();
  if (currentTime - prevTime >= 1000) {
    // RPMの計算（回転方向を考慮）
    rpm = (encoderCount * 60.0) / PULSES_PER_REVOLUTION;
    
    // シリアルモニタに情報を出力
    Serial.printf("%d, %.2f, %s\n", 
      receivedNumber,  // 受信した数値
      rpm,             // RPM（回転方向を反映）
      WiFi.localIP().toString().c_str()  // IPアドレス
    );

    // カウンタをリセット
    encoderCount = 0;
    prevTime = currentTime;
  }
}

void motorForward(int pwmValue) {
  currentSpeed = pwmValue;
  ledcWrite(motorPinA, pwmValue);   // モーターA
  ledcWrite(motorPinB, 0);          // モーターB
}

void motorBackward(int pwmValue) {
  currentSpeed = -pwmValue;
  ledcWrite(motorPinA, 0);          // モーターA
  ledcWrite(motorPinB, pwmValue);   // モーターB
}

void motorStop() {
  currentSpeed = 0;
  ledcWrite(motorPinA, 0);
  ledcWrite(motorPinB, 0);
}

void handleCommand(int speed) {
  if (abs(speed) > 255) {
    Serial.println("Speed must be between -255 and 255.");
    return;
  } 

  if (speed > 0) {
    motorForward(speed);
  } else if (speed < 0) {
    motorBackward(abs(speed));
  } else {
    motorStop();
  }
}
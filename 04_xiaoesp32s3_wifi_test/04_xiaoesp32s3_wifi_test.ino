#include <WiFi.h>
#include <WiFiUdp.h>

// Wi-Fi情報
const char* ssid = "Living-Lab_2.4";
const char* password = "livinglab";
const int localPort = 8888; // UDPサーバーがリッスンするポート番号
WiFiUDP udp;               // UDP通信を扱うオブジェクト

// モータードライバピン
const int motorPinA = 4;    // ドライバの入力Aピン（PWM）
const int motorPinB = 3;    // ドライバの入力Bピン（PWM）
const int encoderPinA = 2;  // エンコーダピンA
const int encoderPinB = 1;  // エンコーダピンB

volatile int encoderCount = 0;      // エンコーダカウント
unsigned long prevTime = 0;         // 前回の時間
float rpm = 0.0;                    // 回転数
const int pulsesPerRevolution = 20; // エンコーダの1回転あたりのパルス数に合わせて調整
int receivedNumber = 0;             // 受信データの初期値

// PWM設定
const int pwmChannelA = 0;     // チャンネル0にmotorPinAを割り当て
const int pwmChannelB = 1;     // チャンネル1にmotorPinBを割り当て
const int pwmFreq = 1000;      // PWM周波数
const int pwmResolution = 8;   // PWM解像度（0～255の範囲）

// 割り込みハンドラ
void IRAM_ATTR encoderISR() {
  int state = digitalRead(encoderPinA);
  if (digitalRead(encoderPinB) == state) {
    encoderCount++;
  } else {
    encoderCount--;
  }
}

void setup() {
  // モーターピンの設定
  pinMode(motorPinA, OUTPUT);
  pinMode(motorPinB, OUTPUT);
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  // PWM設定
  ledcSetup(pwmChannelA, pwmFreq, pwmResolution);
  ledcAttachPin(motorPinA, pwmChannelA);
  ledcSetup(pwmChannelB, pwmFreq, pwmResolution);
  ledcAttachPin(motorPinB, pwmChannelB);

  // エンコーダの割り込み設定
  attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderISR, CHANGE);

  Serial.begin(115200);
  delay(1000);

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

  // UDPサーバー開始
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
      receivedNumber = atoi(packetBuffer); // 受信データを整数値に変換
      Serial.printf("Received Number: %4d at Time: %lu ms\n", receivedNumber, millis());  // 受信した時刻を表示
      handleCommand(receivedNumber); // コマンド処理
  }

  // エンコーダ情報を表示
  rpm = calculateRPM();
  Serial.printf("RPM: %.2f / IP Address: %s\n", rpm, WiFi.localIP().toString().c_str());
  delay(500);
}

// モーター制御関数
void motorForward(int pwmValue) {
  pwmValue = constrain(pwmValue, 0, 255); // PWM値を制限
  ledcWrite(pwmChannelA, pwmValue);
  ledcWrite(pwmChannelB, 0);
}

void motorBackward(int pwmValue) {
  pwmValue = constrain(pwmValue, 0, 255); // PWM値を制限
  ledcWrite(pwmChannelA, 0);
  ledcWrite(pwmChannelB, pwmValue);
}

void motorStop() {
  ledcWrite(pwmChannelA, 0);
  ledcWrite(pwmChannelB, 0);
  Serial.println("Motor stopped");
}

// 受信コマンド処理
void handleCommand(int receivedNumber) {
  if (receivedNumber > 255 || receivedNumber < -255) {
    Serial.println("Speed must be between -255 and 255.");
    return;
  }

  if (receivedNumber > 0) {
    motorForward(receivedNumber);
    Serial.printf("Moving Forward at speed: %d\n", receivedNumber);
  } else if (receivedNumber < 0) {
    motorBackward(abs(receivedNumber));
    Serial.printf("Moving Backward at speed: %d\n", abs(receivedNumber));
  } else {
    motorStop();
  }
}

// 回転数計算
float calculateRPM() {
  unsigned long currentTime = millis();
  float calculatedRPM = 0.0;
  if (currentTime - prevTime >= 1000) { // 1秒ごとに回転数を計算
    calculatedRPM = (encoderCount / (float)pulsesPerRevolution) * 60.0;
    encoderCount = 0;
    prevTime = currentTime;
  }
  return calculatedRPM;
}
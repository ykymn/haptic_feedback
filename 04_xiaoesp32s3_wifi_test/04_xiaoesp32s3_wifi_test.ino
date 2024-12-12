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

// エンコーダカウント
volatile int encoderCount = 0;

// 受信データ用
int receivedNumber = 0;

void setup() {
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

  // ピン初期化
  pinMode(motorPinA, OUTPUT);
  pinMode(motorPinB, OUTPUT);
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  // 割り込み設定
  attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderISR, CHANGE);

  Serial.println("Setup complete. Ready to receive commands.");
}

void loop() {
  int packetSize = udp.parsePacket(); // 受信したパケットのサイズ
  if (packetSize) {
    char packetBuffer[255]; // 受信データのバッファ
    int len = udp.read(packetBuffer, sizeof(packetBuffer) - 1); // パケットを読み込む
    if (len > 0) {
      packetBuffer[len] = '\0'; // 文字列終端
      receivedNumber = atoi(packetBuffer); // 数値に変換
      Serial.printf("Received Number: %4d at Time: %lu ms\n", receivedNumber, millis());
      handleCommand(receivedNumber); // コマンド処理
    }
  }

  // エンコーダ情報を表示
  Serial.printf("Encoder Count: %d / IP Address: %s\n", encoderCount, WiFi.localIP().toString().c_str());
  delay(500);
}

// モーター制御関数
void motorForward(int pwmValue) {
  pwmValue = constrain(pwmValue, 0, 255); // PWM値を制限
  analogWrite(motorPinA, pwmValue);
  analogWrite(motorPinB, 0);
}

void motorBackward(int pwmValue) {
  pwmValue = constrain(pwmValue, 0, 255); // PWM値を制限
  analogWrite(motorPinA, 0);
  analogWrite(motorPinB, pwmValue);
}

void motorStop() {
  analogWrite(motorPinA, 0);
  analogWrite(motorPinB, 0);
  Serial.println("Motor stopped");
}

// エンコーダ割り込みハンドラ
void encoderISR() {
  int stateA = digitalRead(encoderPinA);
  int stateB = digitalRead(encoderPinB);
  encoderCount += (stateA == stateB) ? 1 : -1;
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

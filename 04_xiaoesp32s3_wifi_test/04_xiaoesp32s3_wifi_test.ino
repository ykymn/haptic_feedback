#include <WiFi.h>
#include <WiFiUdp.h>

// Wi-Fi情報
const char* ssid = "Living-Lab_2.4";
const char* password = "livinglab";
const int localPort = 8888; // UDPサーバーがリッスンするポート番号
WiFiUDP udp;

// モータードライバピン
const uint8_t motorPinA = 4; // ドライバの入力Aピン
const uint8_t motorPinB = 3; // ドライバの入力Bピン
const uint8_t encoderPinA = 2; // エンコーダピンA
const uint8_t encoderPinB = 1; // エンコーダピンB

volatile int encoderCount = 0;      // エンコーダカウント
unsigned long prevTime = 0;         // 前回の時間
float rpm = 0.0;                    // 回転数
const int pulsesPerRevolution = 20; // エンコーダの1回転あたりのパルス数
int receivedNumber = 0;             // 受信データ

// PWM設定
const uint8_t pwmResolution = 8; // PWM解像度（0～255）
const uint16_t pwmFrequency = 2000; // PWM周波数
const uint8_t pwmChannelA = 0; // チャンネル0にmotorPinAを割り当て
const uint8_t pwmChannelB = 1; // チャンネル1にmotorPinBを割り当て

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
  // シリアル通信
  Serial.begin(115200);

  // モーターピンの設定
  pinMode(motorPinA, OUTPUT);
  pinMode(motorPinB, OUTPUT);
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  // PWMの初期化
  ledcAttachChannel(motorPinA, pwmFrequency, pwmResolution, pwmChannelA);
  ledcAttachChannel(motorPinB, pwmFrequency, pwmResolution, pwmChannelB);

  // エンコーダ割り込み設定
  attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderISR, CHANGE);

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
}

void loop() {
  // UDPパケットの受信処理
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char packetBuffer[255];
    udp.read(packetBuffer, packetSize);
    packetBuffer[packetSize] = '\0';
    receivedNumber = atoi(packetBuffer);
    Serial.printf("Received Number: %d\n", receivedNumber);
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    handleCommand(receivedNumber);
  }

  // エンコーダ情報を表示
  unsigned long currentTime = millis();
  if (currentTime - prevTime >= 1000) {
    rpm = (encoderCount * 60.0) / pulsesPerRevolution;
    encoderCount = 0;
    prevTime = currentTime;
    Serial.printf("RPM: %.2f\n", rpm);
  }
}

// モーター制御関数
void motorForward(int pwmValue) {
  pwmValue = constrain(pwmValue, 0, 255);
  ledcWrite(pwmChannelA, pwmValue);
  ledcWrite(pwmChannelB, 0);
}

void motorBackward(int pwmValue) {
  pwmValue = constrain(pwmValue, 0, 255);
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

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// Wi-Fi情報
const char* ssid     = "Living-Lab_2.4";
const char* password = "livinglab";
const int localPort = 8888;
WiFiUDP udp;
const IPAddress ip(192, 168, 1, 201);     // (192, 168, 1, 〇〇) 〇〇の部分を設定する
const IPAddress gateway(192, 168, 1, 1);
const IPAddress subnet(255, 255, 255, 0);
const IPAddress DNS(192, 168, 11, 1);

// モータードライバピン
const int motorPinA = 4;
const int motorPinB = 3;
const int encoderPinA = 2;
const int encoderPinB = 1;

// PWMチャンネル定義
#define PWM_CHANNEL_A 0
#define PWM_CHANNEL_B 1
#define PWM_FREQ 500
#define PWM_RESOLUTION 8

// タイムアウト設定
const unsigned long TIMEOUT_DURATION = 5000; // 5秒のタイムアウト
unsigned long lastCommandTime = 0;

// エンコーダパラメータ
const int PULSES_PER_REVOLUTION = 3;  // エンコーダの1回転のパルス数

// グローバル変数
volatile long encoderCount = 0;
volatile int lastEncoded = 0;
volatile long lastencoderValue = 0;
unsigned long prevTime = 0;
float rpm = 0.0;
int currentSpeed = 0;
int receivedNumber = 0;

// エンコーダの状態を更新する関数
void IRAM_ATTR updateEncoder() {
  int MSB = digitalRead(encoderPinA);    // MSB = most significant bit
  int LSB = digitalRead(encoderPinB);    // LSB = least significant bit

  int encoded = (MSB << 1) | LSB;        // 2進数に変換
  int sum = (lastEncoded << 2) | encoded;

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
    encoderCount++;
  } else if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
    encoderCount--;
  }

  lastEncoded = encoded;
}

void setup() {
  // PWMチャンネルの設定
  ledcSetup(PWM_CHANNEL_A, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_B, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(motorPinA, PWM_CHANNEL_A);
  ledcAttachPin(motorPinB, PWM_CHANNEL_B);

  // エンコーダピンを初期化
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  // エンコーダの初期状態を読み取り
  lastEncoded = (digitalRead(encoderPinA) << 1) | digitalRead(encoderPinB);

  // 割り込みを設定（エンコーダピンAのみに設定）
  attachInterrupt(digitalPinToInterrupt(encoderPinA), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), updateEncoder, CHANGE);

  // Wi-Fi接続
  WiFi.config(ip, gateway, subnet, DNS);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
  }
  udp.begin(localPort);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char packetBuffer[255];
    int len = udp.read(packetBuffer, 255);
    packetBuffer[len] = '\0';     
    receivedNumber = atoi(packetBuffer);       
    handleCommand(receivedNumber);
    lastCommandTime = millis(); // コマンド受信時刻を更新
  }

  // タイムアウトチェック
  if (millis() - lastCommandTime > TIMEOUT_DURATION) {
    if (currentSpeed != 0) { // モーターが動いている場合のみ停止
      motorStop();
    }
  }

  // RPMの計算（1秒ごと）
  unsigned long currentTime = millis();
  if (currentTime - prevTime >= 1000) {
    // エンコーダカウントからRPMを計算
    // エンコーダが4逓倍なので、実際のパルス数は4で割る
    long encoderDelta = encoderCount;
    rpm = (encoderDelta * 60.0) / (PULSES_PER_REVOLUTION * 4.0);
    encoderCount = 0;  // カウンタをリセット
    prevTime = currentTime;
  }
}

// モーターの回転方向と回転速度を指示
void motorForward(int pwmValue) {
  ledcWrite(PWM_CHANNEL_A, pwmValue);
  ledcWrite(PWM_CHANNEL_B, 0);
}

void motorBackward(int pwmValue) {
  ledcWrite(PWM_CHANNEL_A, 0);
  ledcWrite(PWM_CHANNEL_B, -pwmValue);
}

void motorStop() {
  ledcWrite(PWM_CHANNEL_A, 0);
  ledcWrite(PWM_CHANNEL_B, 0);
}

// モーターの正転・逆転・停止を指示
void handleCommand(int speed) {
  if (speed > 0) {
    motorForward(speed);
  } else if (speed < 0) {
    motorBackward(speed);
  } else {
    motorStop();
  }
}
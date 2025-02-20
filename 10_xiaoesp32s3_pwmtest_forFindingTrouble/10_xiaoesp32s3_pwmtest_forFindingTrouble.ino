#include <Arduino.h>

// モータードライバピン
const int motorPinA = 1;   // ドライバの入力Aピン（PWM）
const int motorPinB = 2;   // ドライバの入力Bピン（PWM）
const int encoderPinA = 4; // エンコーダピンA
const int encoderPinB = 3; // エンコーダピンB

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
    // シリアル通信を初期化
  Serial.begin(115200);
  while (!Serial) {
    ; // シリアルポートが開くのを待つ
  }

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

}

void loop() {

  // シリアルモニタからのコマンド入力を処理
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    handleCommand(command);
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
  ledcWrite(PWM_CHANNEL_B, abs(pwmValue));
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
    motorBackward(abs(speed));
  } else {
    motorStop();
  }
}

void handleCommand(String command) {
  command.trim();  // 余計な空白を削除

  if (command.length() == 0) {
    Serial.println("Invalid command. Use format: speed (-255 to 255)");
    return;
  }

  int speed = command.toInt();
  
  if (abs(speed) > 255) {
    Serial.println("Speed must be between -255 and 255.");
    return;
  } 

  if (speed > 0) {
    motorForward(speed);
    Serial.println("Moving Forward at speed: " + String(speed));
  } else if (speed < 0) {
    motorBackward(abs(speed));
    Serial.println("Moving Backward at speed: " + String(abs(speed)));
  } else {
    motorStop();
  }
}
#include <Arduino.h>

// モータードライバピン
const int motorPinA = 4;   // ドライバの入力Aピン（PWM）
const int motorPinB = 3;   // ドライバの入力Bピン（PWM）
const int encoderPinA = 2; // エンコーダピンA
const int encoderPinB = 1; // エンコーダピンB

// エンコーダカウント
volatile long encoderCount = 0;   // エンコーダのカウント
unsigned long prevTime = 0;       // 前回の時間
float rpm = 0.0;                  // 回転数
// volatile int lastEncoderState = 0;

//割り込みハンドラ
void IRAM_ATTR encoderISR() {
  int state = digitalRead(encoderPinA);
  if (digitalRead(encoderPinB) == state) {
    encoderCount++;
  } else {
    encoderCount--;
  }
}

void setup() {
  // シリアル通信を初期化
  Serial.begin(115200);
  while (!Serial) {
    ; // シリアルポートが開くのを待つ
  }

  // モーターピンを初期化
  ledcAttach(motorPinA, 500, 8);  // 5kHz, 8ビット解像度
  ledcAttach(motorPinB, 500, 8);  // 5kHz, 8ビット解像度

  // エンコーダピンを初期化
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  // 割り込みを設定
  attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderISR, CHANGE);

  Serial.println("Setup complete. Ready to receive commands.");
}

void loop() {
  // シリアルモニタからのコマンド入力を処理
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    handleCommand(command);
  }

  rpmCalculate(encoderCount);
  // エンコーダのカウントを定期的に表示
  // static unsigned long lastPrintTime = 0;
  // if (millis() - lastPrintTime >= 500) {
  //   Serial.print("Encoder Count: ");
  //   Serial.println(encoderCount);
  //   lastPrintTime = millis();
  // }
}

void rpmCalculate(encoderCount) {
  unsigned long currentTime = millis();
  if (currentTime - prevTime >= 1000) { // 1秒ごとに回転数を計算
    rpm = (encoderCount / 360) * 60.0; // 1秒ごとにRPMを計算
    encoderCount = 0;
    prevTime = currentTime;

    Serial.print("RPM: ");
    Serial.print(rpm);
    Serial.print(" | Speed: ");
    Serial.println(speed);
  }
}

void motorForward(int pwmValue) {
  ledcWrite(motorPinA, pwmValue);   // モーターA
  ledcWrite(motorPinB, 0);          // モーターB
}

void motorBackward(int pwmValue) {
  ledcWrite(motorPinA, 0);          // モーターA
  ledcWrite(motorPinB, pwmValue);   // モーターB
}

void motorStop() {
  ledcWrite(motorPinA, 0);
  ledcWrite(motorPinB, 0);
  Serial.println("Motor stopped");
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
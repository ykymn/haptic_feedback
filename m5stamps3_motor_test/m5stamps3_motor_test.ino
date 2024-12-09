#include <Encoder.h>

// エンコーダのピン設定
#define ENCODER_PIN_A 13
#define ENCODER_PIN_B 15

// モータの制御ピン設定
#define MOTOR_PIN_A 7
#define MOTOR_PIN_B 9

// エンコーダのインスタンスを作成
Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);

// モータの速度設定用変数
int motorSpeed = 200; // モータの速度（固定値）
int rotationTime = 0; // 回転時間（秒）
bool isRotating = false;
unsigned long startTime = 0;

void setup() {
  Serial.begin(115200); // シリアルモニター用の初期化

  // モータ制御ピンの設定
  pinMode(MOTOR_PIN_A, OUTPUT);
  pinMode(MOTOR_PIN_B, OUTPUT);

  // エンコーダの初期化
  encoder.write(0);  // エンコーダのカウントをゼロにリセット

  Serial.println("Enter rotation time in seconds:");
}

void loop() {
  if (Serial.available() > 0) {
    rotationTime = Serial.parseInt(); // シリアルから入力された秒数を取得
    if (rotationTime > 0) {
      isRotating = true;
      startTime = millis(); // 回転開始時間を記録
      Serial.print("Rotating for ");
      Serial.print(rotationTime);
      Serial.println(" seconds...");
    }
  }

  // 回転中の場合
  if (isRotating) {
    // 指定時間内はモータを回転させる
    if (millis() - startTime < rotationTime * 1000) {
      analogWrite(MOTOR_PIN_A, motorSpeed); // モータを前進方向に回転
      analogWrite(MOTOR_PIN_B, 0);          // 反対側を停止
    } else {
      // 指定時間が経過したらモータを停止
      analogWrite(MOTOR_PIN_A, 0);
      analogWrite(MOTOR_PIN_B, 0);
      isRotating = false;
      Serial.println("Rotation complete.");
    }
  }

  // エンコーダの値をシリアルモニタに表示
  long encoderValue = encoder.read();
  Serial.print("Encoder: ");
  Serial.println(encoderValue);

  delay(100); // 100msのディレイを入れる
}

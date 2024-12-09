#include <WiFi.h> // Wi-Fiライブラリ

// Wi-Fi情報
const char* ssid = "Living-Lab_2.4";
const char* password = "livinglab";

// モータードライバピン
const int motorPinA = 4;  // ドライバの入力Aピン（PWM）
const int motorPinB = 3;  // ドライバの入力Bピン（PWM）
const int encoderPinA = 2;  // エンコーダピンA
const int encoderPinB = 1;  // エンコーダピンB

// エンコーダカウント
volatile int encoderCount = 0;

void setup() {
  // シリアル通信を初期化
  Serial.begin(115200);
  delay(1000);

  // Wi-Fi接続のデバッグメッセージ
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  // Wi-Fi接続待機（接続完了まで無限ループ）
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  // Wi-Fi接続完了
  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // モーターピンを初期化
  pinMode(motorPinA, OUTPUT);
  pinMode(motorPinB, OUTPUT);

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

  // エンコーダのカウントを表示
  Serial.print("Encoder Count: ");
  Serial.print(encoderCount);
  Serial.print(" / IP Address: ");
  Serial.println(WiFi.localIP());
  delay(500);
}

// モーター制御関数
void motorForward(int pwmValue) {
  analogWrite(motorPinA, pwmValue);
  analogWrite(motorPinB, 0);
}

void motorBackward(int pwmValue) {
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

// シリアルコマンドを処理
void handleCommand(String command) {
  // コマンドをパース
  command.trim();  // 余計な空白を削除

  if (command.length() == 0) {
    Serial.println("Invalid command. Use format: f/r speed or s to stop");
    return;
  }

  char direction = command[0];  // 最初の文字が方向

  if (direction == 's') {
    motorStop();
    return;
  }

  if (command.length() < 2) {
    Serial.println("Invalid command. Use format: f/r speed (e.g., f128)");
    return;
  }

  int speed = command.substring(1).toInt();  // 残りが速度

  if (speed < 0 || speed > 255) {
    Serial.println("Speed must be between 0 and 255.");
    return;
  }

  if (direction == 'f') {
    motorForward(speed);
    Serial.println("Moving Forward at speed: " + String(speed));
  } else if (direction == 'r') {
    motorBackward(speed);
    Serial.println("Moving Backward at speed: " + String(speed));
  } else {
    Serial.println("Invalid direction. Use 'f' for forward, 'r' for backward, or 's' to stop.");
  }
}

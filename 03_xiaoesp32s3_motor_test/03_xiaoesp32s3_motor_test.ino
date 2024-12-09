// モータードライバピン
const int motorPinA = 4;  // ドライバの入力Aピン（PWM）
const int motorPinB = 3;  // ドライバの入力Bピン（PWM）

// エンコーダピン
const int encoderPinA = 2;  // エンコーダのC1ピン
const int encoderPinB = 1;  // エンコーダのC2ピン

// エンコーダカウント
volatile int encoderCount = 0;

void setup() {
  // シリアル通信の初期化
  Serial.begin(115200);

  // モータードライバピンの設定
  pinMode(motorPinA, OUTPUT);
  pinMode(motorPinB, OUTPUT);

  // エンコーダピンの設定
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  // エンコーダAピンの割り込み設定
  attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderISR, CHANGE);

  Serial.println("Setup Complete. Enter command: F/B speed or S to stop");
}

void loop() {
  // シリアル入力を監視
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');  // コマンドを読み取る
    handleCommand(command);                         // コマンドを処理
  }

  // エンコーダのカウントを表示
  Serial.print("Encoder Count: ");
  Serial.println(encoderCount);
  delay(500);  // データ出力の間隔
}

// モーターを前進させる
void motorForward(int pwmValue) {
  analogWrite(motorPinA, pwmValue);  // AピンにPWM出力
  analogWrite(motorPinB, 0);        // BピンはLOW
}

// モーターを逆転させる
void motorBackward(int pwmValue) {
  analogWrite(motorPinA, 0);        // AピンはLOW
  analogWrite(motorPinB, pwmValue); // BピンにPWM出力
}

// モーターを停止させる
void motorStop() {
  analogWrite(motorPinA, 0);
  analogWrite(motorPinB, 0);
  Serial.println("Motor Stopped");
}

// エンコーダ割り込みサービスルーチン
void encoderISR() {
  int stateA = digitalRead(encoderPinA);
  int stateB = digitalRead(encoderPinB);

  // カウント方向の判定
  if (stateA == stateB) {
    encoderCount++;  // 正方向
  } else {
    encoderCount--;  // 逆方向
  }
}

// シリアルコマンドを処理
void handleCommand(String command) {
  // コマンドをパース
  command.trim();  // 余計な空白を削除

  if (command.length() == 0) {
    Serial.println("Invalid command. Use format: F/B speed or S to stop");
    return;
  }

  char direction = command[0];  // 最初の文字が方向

  if (direction == 'S') {
    motorStop();
    return;
  }

  if (command.length() < 2) {
    Serial.println("Invalid command. Use format: F/B speed (e.g., F128)");
    return;
  }

  int speed = command.substring(1).toInt();  // 残りが速度

  if (speed < 0 || speed > 255) {
    Serial.println("Speed must be between 0 and 255.");
    return;
  }

  if (direction == 'F') {
    motorForward(speed);
    Serial.println("Moving Forward at speed: " + String(speed));
  } else if (direction == 'B') {
    motorBackward(speed);
    Serial.println("Moving Backward at speed: " + String(speed));
  } else {
    Serial.println("Invalid direction. Use 'F' for forward, 'B' for backward, or 'S' to stop.");
  }
}

// ピン設定
const int motorPinA = 7;   // ドライバの入力Aピン
const int motorPinB = 9;   // ドライバの入力Bピン
const int encoderPinA = 15; // エンコーダのC1ピン
const int encoderPinB = 13; // エンコーダのC2ピン

volatile int encoderCount = 0; // エンコーダのカウント
unsigned long prevTime = 0;    // 前回の時間
float rpm = 0.0;               // 回転数
char command = 's';            // 初期コマンド（停止）

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

  // エンコーダピンの設定
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  
  // エンコーダの割り込み設定
  attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderISR, CHANGE);

  Serial.begin(115200);
  Serial.println("Enter 'f' for forward, 'r' for reverse, 's' to stop:");
}

void loop() {
  // PCからのコマンドを読み取る
  if (Serial.available() > 0) {
    command = Serial.read();
  }

  // コマンドに応じてモーターの回転方向を設定
  if (command == 'f') {        // 正転
    digitalWrite(motorPinA, HIGH);
    digitalWrite(motorPinB, LOW);
  } else if (command == 'r') { // 逆転
    digitalWrite(motorPinA, LOW);
    digitalWrite(motorPinB, HIGH);
  } else if (command == 's') { // 停止
    digitalWrite(motorPinA, LOW);
    digitalWrite(motorPinB, LOW);
  }

  // 回転数計算
  unsigned long currentTime = millis();
  if (currentTime - prevTime >= 1000) { // 1秒ごとに回転数を計算
    rpm = (encoderCount / 20.0) * 60.0; // 20はエンコーダのパルス数（調整が必要）
    encoderCount = 0;
    prevTime = currentTime;
    
    Serial.print("RPM: ");
    Serial.println(rpm);
  }
}

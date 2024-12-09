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

  Serial.println("Setup Complete");
}

void loop() {
  // モーターを前進させる
  motorForward(128);  // 50%のPWMデューティ比
  delay(2000);

  // モーターを停止
  motorStop();
  delay(1000);

  // モーターを逆転させる
  motorBackward(128);  // 50%のPWMデューティ比
  delay(2000);

  // モーターを停止
  motorStop();
  delay(1000);

  // エンコーダのカウントを表示
  Serial.print("Encoder Count: ");
  Serial.println(encoderCount);
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

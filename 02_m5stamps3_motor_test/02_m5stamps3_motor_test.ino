// ピン設定
const int motorPinA = 7;       // ドライバの入力Aピン（PWM）
const int motorPinB = 9;       // ドライバの入力Bピン（PWM）
const int encoderPinA = 15;    // エンコーダのC1ピン
const int encoderPinB = 13;    // エンコーダのC2ピン

volatile int encoderCount = 0; // エンコーダのカウント
unsigned long prevTime = 0;    // 前回の時間
float rpm = 0.0;               // 回転数
const int pulsesPerRevolution = 20; // エンコーダの1回転あたりのパルス数に合わせて調整
char command = 's';            // 初期コマンド（停止）
int speed = 0;                 // モーター速度（0～255の範囲）

// PWM設定
const int pwmChannelA = 0;     // チャンネル0にmotorPinAを割り当て
const int pwmChannelB = 1;     // チャンネル1にmotorPinBを割り当て
const int pwmFreq = 1000;      // PWM周波数
const int pwmResolution = 8;   // PWM解像度（0～255の範囲）

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

  // PWM設定
  ledcSetup(pwmChannelA, pwmFreq, pwmResolution);
  ledcAttachPin(motorPinA, pwmChannelA);
  ledcSetup(pwmChannelB, pwmFreq, pwmResolution);
  ledcAttachPin(motorPinB, pwmChannelB);

  // エンコーダピンの設定
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  
  // エンコーダの割り込み設定
  attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderISR, CHANGE);

  Serial.begin(115200);
  Serial.println("Enter 'f' for forward, 'r' for reverse, 's' to stop, and a number (0-255) for speed:");
}

void loop() {
  // PCからのコマンドを読み取る
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim(); // 空白と改行を削除

    if (input == "f") {           // 正転
      command = 'f';
    } else if (input == "r") {    // 逆転
      command = 'r';
    } else if (input == "s") {    // 停止
      command = 's';
    } else {
      int inputSpeed = input.toInt();
      if (inputSpeed >= 0 && inputSpeed <= 255) {
        speed = inputSpeed;       // 入力された速度に設定
      }
    }
  }

  // コマンドに応じてモーターの動作を設定
  if (command == 'f') {           // 正転
    ledcWrite(pwmChannelA, speed);
    ledcWrite(pwmChannelB, 0);
  } else if (command == 'r') {    // 逆転
    ledcWrite(pwmChannelA, 0);
    ledcWrite(pwmChannelB, speed);
  } else if (command == 's') {    // 停止
    ledcWrite(pwmChannelA, 0);
    ledcWrite(pwmChannelB, 0);
  }

  // 回転数計算
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

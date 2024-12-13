// モータードライバピン
const uint8_t motorPinA = 4; // ドライバの入力Aピン
const uint8_t motorPinB = 3; // ドライバの入力Bピン
const uint8_t encoderPinA = 2; // エンコーダピンA
const uint8_t encoderPinB = 1; // エンコーダピンB

// 必要な定数とピン定義
const int motorPWMChannel = 0;  // PWMチャネル番号
const int pwmFrequency = 5000; // PWM周波数 (5kHz)
const int pwmResolution = 8;   // PWM分解能 (0-255)

void setup() {
  // シリアル通信の初期化
  Serial.begin(115200);
  while (!Serial) {
    ; // シリアルポートが準備されるのを待つ
  }

  // PWM出力の初期設定
  ledcSetup(motorPWMChannel, pwmFrequency, pwmResolution);
  ledcAttachPin(motorPinA, motorPWMChannel);

  // モータードライバピンの設定
  pinMode(motorPinA, OUTPUT);
  pinMode(motorPinB, OUTPUT);

  Serial.println("モータ速度制御プログラムが起動しました。");
  Serial.println("0-255の範囲で速度を入力してください。");
}

void loop() {
  // シリアル通信から入力があるか確認
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n'); // 改行までの入力を読み取る
    input.trim(); // 余分な空白を削除

    // 入力値を整数に変換
    int speed = input.toInt();

    // 入力値の検証 (0-255 の範囲に制限)
    if (speed >= 0 && speed <= 255) {
      // PWM信号を設定
      ledcWrite(motorPWMChannel, speed);

      // モータの方向制御 (例: 前進方向のみ設定)
      digitalWrite(motorPinA, HIGH);
      digitalWrite(motorPinB, LOW);

      Serial.print("設定速度: ");
      Serial.println(speed);
    } else {
      Serial.println("無効な入力です。0-255の範囲で再入力してください。");
    }
  }
}

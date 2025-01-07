#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// Wi-Fi情報
const char* ssid = "Living-Lab_2.4";
const char* password = "livinglab";
const int localPort = 8888;
WiFiUDP udp;

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
    Serial.begin(115200);
    while (!Serial) {
        ;
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

    // Wi-Fi接続
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected to Wi-Fi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    udp.begin(localPort);
    Serial.printf("UDP Server started on port %d\n", localPort);
    Serial.println("Setup complete. Ready to receive commands.");
}

void loop() {
    int packetSize = udp.parsePacket();
    if (packetSize) {
        char packetBuffer[255];
        int len = udp.read(packetBuffer, 255);
        packetBuffer[len] = '\0';
        
        // Serial.printf("Received %d bytes from %s, port %d\n", 
        //     packetSize, 
        //     udp.remoteIP().toString().c_str(), 
        //     udp.remotePort());
        // Serial.printf("Contents: %s\n", packetBuffer);
        
        receivedNumber = atoi(packetBuffer);
        // Serial.printf("Parsed number: %d\n", receivedNumber);
        
        handleCommand(receivedNumber);
    }

    // RPMの計算（1秒ごと）
    unsigned long currentTime = millis();
    if (currentTime - prevTime >= 1000) {
        // エンコーダカウントからRPMを計算
        // エンコーダが4逓倍なので、実際のパルス数は4で割る
        long encoderDelta = encoderCount;
        rpm = (encoderDelta * 60.0) / (PULSES_PER_REVOLUTION * 4.0);
        
        Serial.printf("Speed: %d, EncoderCount: %ld, RPM: %.2f, IP: %s\n", 
            receivedNumber,
            encoderDelta,
            rpm,
            WiFi.localIP().toString().c_str()
        );

        encoderCount = 0;  // カウンタをリセット
        prevTime = currentTime;
    }
}

void motorForward(int pwmValue) {
    currentSpeed = pwmValue;
    ledcWrite(PWM_CHANNEL_A, pwmValue);
    ledcWrite(PWM_CHANNEL_B, 0);
}

void motorBackward(int pwmValue) {
    currentSpeed = -pwmValue;
    ledcWrite(PWM_CHANNEL_A, 0);
    ledcWrite(PWM_CHANNEL_B, pwmValue);
}

void motorStop() {
    currentSpeed = 0;
    ledcWrite(PWM_CHANNEL_A, 0);
    ledcWrite(PWM_CHANNEL_B, 0);
}

void handleCommand(int speed) {
    if (abs(speed) > 255) {
        Serial.println("Speed must be between -255 and 255.");
        return;
    } 

    if (speed > 0) {
        motorForward(speed);
    } else if (speed < 0) {
        motorBackward(abs(speed));
    } else {
        motorStop();
    }
}
const int LED_PIN = 21;  // XIAO ESP32S3の内蔵LED

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  Serial.println("LED ON");
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  Serial.println("LED OFF");
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}

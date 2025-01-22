#include <WiFi.h>

const char* ssid     = "xxxxxxxx";
const char* password = "xxxxxxxx";
const IPAddress ip(192, 168, 11, 52);
const IPAddress gateway(192, 168, 11, 1);
const IPAddress subnet(255, 255, 255, 0);
const IPAddress dns1(192, 168, 11, 1);

void setup()
{
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  if (!WiFi.config(ip,gateway,subnet,dns1)){
      Serial.println("Failed to configure!");
  }
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  // put your main code here, to run repeatedly:
}
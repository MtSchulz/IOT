#include <WiFi.h>

const char* ssid = "Smart 4.0 (3)";
const char* password = "Smart4.0";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Conectando ao Wi-Fi..");
  }
  Serial.print("Conectado a rede Wi-Fi: ");
  Serial.println(ssid);

  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Endereço MAC: ");
  Serial.println(WiFi.macAddress());

}

void loop() {
  // put your main code here, to run repeatedly:

}

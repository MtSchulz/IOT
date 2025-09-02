#include <WiFi.h>

const char* ssid = "Smart 4.0 (3)";
const char* password = "Smart4.0";

int LED = 27;

WiFiServer server(8080);

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Conectando ao Wi-Fi..");
  }

  Serial.println("");
  Serial.println("Wi-Fi conectada.");
  Serial.println("Endereço de IP: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if(client) {
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) {
      if(client.available()) {
        char c = client.read();
        Serial.write(c);
        if(c == '\n') {
          if(currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            client.print("Click <a href=\"/H\">aqui<a> para ligar o LED no gpio 27. <br>");
            client.print("Click <a href=\"/L\">aqui<a> para desligar o LED no grpio 27. <br>");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
        if(currentLine.endsWith("GET /H")) {
          digitalWrite(LED, HIGH);
        }
        if(currentLine.endsWith("GET /L")) {
          digitalWrite(LED, LOW);
        }
      }
    }
    client.stop();
    Serial.println("Client Disconnected.");
  } 
}

#include <WiFiServer.h>
#include <WiFi.h>
#include <LiquidCrystal.h>

#define LED_R 25
#define LED_G 26
#define LED_B 27

LiquidCrystal lcd(32, 33, 23, 22, 21, 18);

WiFiServer sv(555);
WiFiClient cl;

void setup() {
  Serial.begin(115200);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  // Configura PWM dos LEDs
  ledcSetup(0, 5000, 8); 
  ledcSetup(1, 5000, 8); 
  ledcSetup(2, 5000, 8); 

  ledcAttachPin(LED_R, 0);
  ledcAttachPin(LED_G, 1);
  ledcAttachPin(LED_B, 2);

  lcd.begin(16,2);

  WiFi.softAP("RedeWilSchulz", "");
  sv.begin();
  Serial.println("Server TCP iniciado");
}

void loop() {
  tcp();
}

void tcp() {
  if (!cl || !cl.connected()) {
    cl = sv.available();
    return;
  }

  // Se houver dados do cliente
  if (cl.available() > 0) {
    String req = "";
    while (cl.available() > 0) {
      char c = cl.read();
      req += c;  
    }
    req.trim(); // Remove espaços e quebras de linha

    Serial.println("");
    Serial.print("Mensagem recebida: ");
    Serial.println(req);

    // LCD
    lcd.clear();
    lcd.print("Led: ");
    lcd.println(req);

    // Divide a string por vírgula
    int virg1 = req.indexOf(',');
    int virg2 = req.lastIndexOf(',');

    if (virg1 > 0 && virg2 > virg1) {
      int r = req.substring(0, virg1).toInt();
      int g = req.substring(virg1 + 1, virg2).toInt();
      int b = req.substring(virg2 + 1).toInt();

      // Converte 0–100% para 0–255
      int pwmR = map(r, 0, 100, 0, 255);
      int pwmG = map(g, 0, 100, 0, 255);
      int pwmB = map(b, 0, 100, 0, 255);

      // Acende os LEDs
      ledcWrite(0, pwmR);
      ledcWrite(1, pwmG);
      ledcWrite(2, pwmB);

      Serial.print("RGB PWM: ");
      Serial.print(pwmR); Serial.print(", ");
      Serial.print(pwmG); Serial.print(", ");
      Serial.println(pwmB);
    } else {
      Serial.println("Formato inválido! Use R,G,B (0-100)");
    }
  }
}
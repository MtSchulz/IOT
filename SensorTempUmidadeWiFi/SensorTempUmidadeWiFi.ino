#include <WiFi.h>
#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal.h>

#define DHTPIN 15
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal lcd(32, 33, 23, 22, 21, 18);

WiFiServer sv(555);
WiFiClient cl;

// Variáveis globais
float temperatura = 0.0;
float umidade = 0.0;
unsigned long ultimoUpdate = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();

  lcd.begin(16, 2);  
  lcd.print("Iniciando...");

  WiFi.mode(WIFI_AP);
  WiFi.softAP("RedeWilSchulz", "");
  sv.begin();

  Serial.println("Servidor TCP iniciado");
  Serial.print("IP do servidor: ");
  Serial.println(WiFi.softAPIP());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Servidor pronto");
}

void loop() {
  // Atualiza temperatura e umidade a cada 2s
  if (millis() - ultimoUpdate > 2000) {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      temperatura = t;
      umidade = h;

      Serial.print("Nova leitura -> ");
      Serial.print("Temp: ");
      Serial.print(temperatura);
      Serial.print(" C, Umid: ");
      Serial.print(umidade);
      Serial.println(" %");
    } else {
      Serial.println("Falha na leitura do DHT11!");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Erro sensor DHT");
    }

    ultimoUpdate = millis();
  }

  // Verifica conexão de cliente
  if (!cl || !cl.connected()) {
    cl = sv.available();
    return;
  }

  // Processa comandos recebidos
  if (cl.available() > 0) {
    String req = cl.readStringUntil('\n');
    req.trim();

    Serial.print("Comando recebido: ");
    Serial.println(req);
    lcd.clear();

    if (req.equalsIgnoreCase("temperatura")) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Temp: ");
      lcd.print(temperatura, 1);
      lcd.print(" C");

      cl.println("Temperatura: " + String(temperatura, 1) + " C");
    } 
    else if (req.equalsIgnoreCase("umidade")) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Umidade: ");
      lcd.print(umidade, 1);
      lcd.print(" %");

      cl.println("Umidade: " + String(umidade, 1) + " %");
    } 
    else {
      cl.println("Comando invalido. Use: temperatura ou umidade");
    }
  }
}

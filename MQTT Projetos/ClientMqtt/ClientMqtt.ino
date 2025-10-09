#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <LiquidCrystal.h>

#define LED_PIN 27
#define DHTPIN 15
#define DHTTYPE DHT11
#define TRIG_PIN 13
#define ECHO_PIN 35

#define LCD_RS 32
#define LCD_EN 33
#define LCD_D4 23
#define LCD_D5 22
#define LCD_D6 21
#define LCD_D7 18

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

const char* ssid = "Smart 4.0 (3)";
const char* password = "Smart4.0";
const char* mqtt_server = "10.74.241.795";

WiFiClient espClient;
PubSubClient client(espClient);

bool ledStatus = false;
unsigned long lastMsg = 0;
const long interval = 5000;

float medirDistancia() {
  digitalWrite(TRIG_PIN, 0);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, 1);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, 0);
  long duracao = pulseIn(ECHO_PIN, 1);
  float distancia = duracao * 0.034 / 2;
  return distancia;
}

String gerarJSON() {
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();
  float distancia = medirDistancia();

  StaticJsonDocument<200> doc;
  doc["led"] = ledStatus ? "ON" : "OFF";
  doc["temperatura"] = temperatura;
  doc["umidade"] = umidade;
  doc["distancia_cm"] = distancia;

  String jsonStr;
  serializeJson(doc, jsonStr);

  return jsonStr;
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";

  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.print("Comando recebido: ");
  Serial.println(msg);

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, msg);
  if (error) {
    Serial.println("Erro ao interpretar JSON!");
    return;
  }

  if (doc.containsKey("led")) {
    String comandoLed = doc["led"];
    if (comandoLed == "ON") {
      digitalWrite(LED_PIN, 1);
      ledStatus = true;
    } else if (comandoLed == "OFF") {
      digitalWrite(LED_PIN, 0);
      ledStatus = false;
    }
  }

  if (doc.containsKey("mensagem")) {
    String mensagem = doc["mensagem"].as<String>();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(mensagem);
  }
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conenctando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi conectado.");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    if (client.connect("ESP32_CLIENT")) {
      Serial.println("Conectado ao Broker!");
      client.subscribe("esp32/comandos");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  dht.begin();

  lcd.begin(16, 2);
  lcd.clear();

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;
    String mensagem = gerarJSON();
    client.publish("esp32/sensores", mensagem.c_str());
    Serial.print("Enviado - > ");
    Serial.println(mensagem);
  }
}

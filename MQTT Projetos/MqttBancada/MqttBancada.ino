#include <WiFiManager.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <LiquidCrystal.h>
#include <WebServer.h>   // Para o servidor HTTP local

#define LED_PIN   27
#define DHTPIN    15
#define DHTTYPE   DHT11
#define TRIG_PIN  13
#define ECHO_PIN  35

#define LCD_RS 32
#define LCD_EN 33
#define LCD_D4 23
#define LCD_D5 22
#define LCD_D6 21
#define LCD_D7 18

#define LED_R 26
#define LED_G 25
#define LED_B 14

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

WiFiClient espClient;
PubSubClient client(espClient);
WebServer server(80);   // Porta HTTP padrão

bool ledStatus = false;
unsigned long lastMsg = 0;
const long interval = 5000;

// =================== Funções auxiliares ===================

float medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duracao = pulseIn(ECHO_PIN, HIGH);
  return duracao * 0.034 / 2;
}

void showLCD(const char* line1, const char* line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void setLedRGB(bool wifi, bool mqtt) {
  if (!wifi) { // Verde
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, HIGH);
  } else if (wifi && !mqtt) { // Azul
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, LOW);
  } else if (wifi && mqtt) { // Vermelho
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, LOW);
  }
}
// =================== Callback MQTT ===================

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.println("[MQTT] " + msg);
}

// =================== Servidor HTTP ===================

void handleRoot() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  float dist = medirDistancia();

  String json = "{";
  json += "\"temperatura\":" + String(temp) + ",";
  json += "\"umidade\":" + String(hum) + ",";
  json += "\"distancia\":" + String(dist) + ",";
  json += "\"led\":\"" + String(setLedRGB ? "ON" : "OFF") + "\"";
  json += "}";

  server.send(200, "application/json", json);
  Serial.println("[HTTP] Dados enviados -> " + json);
}

void handleLed() {
  if (server.hasArg("state")) {
    String state = server.arg("state");
    if (state == "ON") {
      digitalWrite(LED_PIN, HIGH);
      ledStatus = true;
    } else {
      digitalWrite(LED_PIN, LOW);
      ledStatus = false;
    }
  }
  server.send(200, "text/plain", "OK");
}

// =================== Wi-Fi + Access Point ===================

void setup_wifi() {
  WiFi.mode(WIFI_AP_STA);  // STA + AP
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  Serial.println("\nIniciando Wi-Fi com WiFiManager...");
  showLCD("Wi-Fi:", "Conectando...");
  setLedRGB(false, false);

  WiFiManager wm;
  bool res = wm.autoConnect("ESP32_W1lSchulz", "12345678");

  if (!res) {
    Serial.println("Falha ao conectar Wi-Fi! Reiniciando...");
    showLCD("Wi-Fi:", "Falha!");
    setLedRGB(false, false);
    delay(3000);
    ESP.restart();
  } else {
    Serial.println("Wi-Fi conectado!");
    Serial.print("IP (STA): ");
    Serial.println(WiFi.localIP());
    showLCD("Wi-Fi: OK", "MQTT: Aguardando...");
    setLedRGB(true, false);
  }

  // ---------- Criação do Access Point ----------
  WiFi.softAP("ESP32_W1lSchulz", "12345678");
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP ativo em: ");
  Serial.println(IP);
  showLCD("AP: ESP32_W1lSchulz", ("IP: " + WiFi.softAPIP().toString()).c_str());
}

// =================== MQTT Reconnect ===================

void reconnect() {
  while (!client.connected()) {
    Serial.println("Tentando MQTT...");
    if (client.connect("ESP32_CLIENT")) {
      Serial.println("Conectado ao broker!");
      client.subscribe("/smart4.0");
      showLCD("Wi-Fi: OK", "MQTT: Conectado");
      setLedRGB(true, true);
    } else {
      Serial.println("Falha MQTT, tentando novamente...");
      showLCD("Wi-Fi: OK", "MQTT: Falhou!");
      setLedRGB(true, false);
      delay(5000);
    }
  }
}

// =================== SETUP ===================

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  dht.begin();
  lcd.begin(16, 2);
  lcd.clear();

  setup_wifi();

  // Configurar servidor HTTP
  server.on("/", handleRoot);
  server.on("/led", handleLed);
  server.begin();
  Serial.println("Servidor HTTP iniciado na porta 80");

  // Configurar MQTT
  client.setServer("10.74.241.95", 1883);
  client.setCallback(callback);
}

// =================== LOOP ===================

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    showLCD("Wi-Fi:", "Desconectado!");
    setLedRGB(false, false);
    setup_wifi();
  }

  client.loop();
  server.handleClient(); // Mantém servidor HTTP ativo

  if (!client.connected()) reconnect();
}

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>
#include <WiFiManager.h>

// ------------------- Definições de Hardware -------------------
#define LED_RED       27
#define LED_GREEN     26
#define LED_BLUE      25

#define SW1 0
#define SW2 4

#define LCD_RS 32
#define LCD_EN 33
#define LCD_D4 23
#define LCD_D5 22
#define LCD_D6 21
#define LCD_D7 18

// ------------------- Instâncias -------------------
WiFiManager wm;
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
WiFiClient espClient;
PubSubClient client(espClient);

// ------------------- Variáveis Globais -------------------
int paginaAtual = 0;
bool wifiConnected = false;
bool mqttConnected = false;

#define MAX_VARS 20
String history[MAX_VARS];
int varCount = 0;

// ------------------- LEDs -------------------
void setupLEDs() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
}

void setLEDStatus() {
  if (!wifiConnected) {
    analogWrite(LED_RED, 255);
    analogWrite(LED_GREEN, 0);
    analogWrite(LED_BLUE, 0);
  } else if (!mqttConnected) {
    analogWrite(LED_RED, 0);
    analogWrite(LED_GREEN, 0);
    analogWrite(LED_BLUE, 255);
  } else {
    analogWrite(LED_RED, 0);
    analogWrite(LED_GREEN, 255);
    analogWrite(LED_BLUE, 0);
  }
}

// ------------------- Exibição no LCD -------------------
void mostrarPagina(int pagina) {
  lcd.clear();

  if (varCount == 0) {
    lcd.setCursor(0, 0);
    if (!wifiConnected) {
      lcd.print("Conectando WiFi");
    } else if (!mqttConnected) {
      lcd.print("Conectando MQTT");
    } else {
      lcd.print("Aguardando...");
    }
    lcd.setCursor(0, 1);
    lcd.print("Sem dados");
    return;
  }

  int idx = pagina % varCount;
  lcd.setCursor(0, 0);
  lcd.print("Var ");
  lcd.print(idx + 1);
  lcd.print("/");
  lcd.print(varCount);

  String displayLine = history[idx];
  if (displayLine.length() > 16) {
    displayLine = displayLine.substring(0, 16);
  }

  lcd.setCursor(0, 1);
  lcd.print(displayLine);
}

// ------------------- Conexão Wi-Fi -------------------
void setupWiFi() {
  Serial.println("Configurando WiFi...");

  wm.setConfigPortalTimeout(180);
  wm.setConnectTimeout(30);
  wm.setDebugOutput(false);

  lcd.clear();
  lcd.print("Conectando WiFi");
  lcd.setCursor(0, 1);
  lcd.print("Aguarde...");

  bool res = wm.autoConnect("ESP32_WILSCHULZ", "12345678");

  if (!res) {
    Serial.println("Falha na conexao WiFi");
    lcd.clear();
    lcd.print("Modo AP ativo");
    lcd.setCursor(0, 1);
    lcd.print("ESP32_SMART");
    wifiConnected = false;
  } else {
    Serial.println("WiFi conectado");
    wifiConnected = true;
    lcd.clear();
    lcd.print("WiFi conectado");
    delay(2000);
  }

  setLEDStatus();
}

// ------------------- Callback MQTT -------------------
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("----------------------------------------");
  Serial.print("Mensagem recebida no topico: ");
  Serial.println(topic);

  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Payload: ");
  Serial.println(message);

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("Erro ao interpretar JSON: ");
    Serial.println(error.c_str());
    varCount = 1;
    history[0] = "Raw: " + message.substring(0, 14);
  } else {
    varCount = 0;
    JsonObject obj = doc.as<JsonObject>();

    for (JsonPair kv : obj) {
      if (varCount < MAX_VARS) {
        String key = String(kv.key().c_str());
        String value;

        if (kv.value().is<float>()) {
          value = String(kv.value().as<float>(), 2);
        } else if (kv.value().is<int>()) {
          value = String(kv.value().as<int>());
        } else if (kv.value().is<const char*>()) {
          value = kv.value().as<const char*>();
        } else {
          value = "null";
        }

        history[varCount] = key + ":" + value;
        varCount++;
      }
    }
  }

  Serial.print("Total de variaveis processadas: ");
  Serial.println(varCount);
  Serial.println("----------------------------------------");

  mostrarPagina(paginaAtual);
}

// ------------------- Conexão MQTT -------------------
void setupMQTT() {
  client.setServer("10.74.241.95", 1883);
  client.setCallback(callback);
}

void reconnectMQTT() {
  if (!wifiConnected) return;

  Serial.print("Conectando ao broker MQTT...");
  String clientId = "ESP32_Client_" + String(random(0xffff), HEX);

  if (client.connect(clientId.c_str())) {
    Serial.println(" conectado");
    client.subscribe("/smart4.0");
    mqttConnected = true;
    lcd.clear();
    lcd.print("MQTT conectado");
    delay(1000);
    mostrarPagina(paginaAtual);
  } else {
    Serial.print(" falhou, rc=");
    Serial.println(client.state());
    mqttConnected = false;
  }

  setLEDStatus();
}

// ------------------- Setup -------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando ESP32...");

  setupLEDs();
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Inicializando...");
  delay(1000);

  setupWiFi();
  if (wifiConnected) setupMQTT();

  mostrarPagina(paginaAtual);
}

// ------------------- Loop Principal -------------------
void loop() {
  // Verifica estado do Wi-Fi
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    mqttConnected = false;
    setLEDStatus();

    static unsigned long lastWiFiReconnect = 0;
    if (millis() - lastWiFiReconnect > 10000) {
      Serial.println("Tentando reconexao WiFi...");
      WiFi.reconnect();
      lastWiFiReconnect = millis();
    }
  } else {
    wifiConnected = true;
  }

  // MQTT
  if (wifiConnected) {
    if (!client.connected()) {
      reconnectMQTT();
    }
    client.loop();
  }

  // Navegação pelas variáveis
  if (digitalRead(SW1) == LOW) {
    if (varCount > 0) {
      paginaAtual = (paginaAtual - 1 + varCount) % varCount;
      mostrarPagina(paginaAtual);
    }
    delay(300);
  }

  if (digitalRead(SW2) == LOW) {
    if (varCount > 0) {
      paginaAtual = (paginaAtual + 1) % varCount;
      mostrarPagina(paginaAtual);
    }
    delay(300);
  }

  // Reset Wi-Fi se o botão SW1 for pressionado por 5s
  static unsigned long buttonTime = 0;
  static bool resetTriggered = false;

  if (digitalRead(SW1) == LOW && !resetTriggered) {
    if (buttonTime == 0) {
      buttonTime = millis();
    } else if (millis() - buttonTime > 5000) {
      Serial.println("Reset WiFi solicitado");
      lcd.clear();
      lcd.print("Reset WiFi...");
      wm.resetSettings();
      delay(2000);
      ESP.restart();
    }
  } else {
    buttonTime = 0;
    resetTriggered = false;
  }

  delay(100);
}

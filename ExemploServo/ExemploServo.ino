#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// ----------- Configurações WiFi -----------
const char* ssid     = "Smart 4.0 (3)";
const char* password = "Smart4.0";

// ----------- Configuração do servidor -----------
WebServer server(80);

// ----------- Configuração do DHT11 -----------
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ----------- Configuração do HC-SR04 -----------
#define TRIG_PIN 13
#define ECHO_PIN 35   

// ----------- Protótipo (declaração antecipada) -----------
float lerDistancia();

// ---------------- Handlers da API ----------------
void handleRoot() {
  server.send(200, "text/plain", "API REST ESP32 Ativa!");
}

void handleDHT() {
  float temperatura = dht.readTemperature();
  float umidade     = dht.readHumidity();

  if (isnan(temperatura) || isnan(umidade)) {
    server.send(500, "application/json", "{\"status\":\"Erro na leitura do DHT11\"}");
    return;
  }

  String json = "{";
  json += "\"temperatura\":" + String(temperatura, 1) + ",";
  json += "\"umidade\":" + String(umidade, 1);
  json += "}";

  server.send(200, "application/json", json);
}

void handleDistancia() {
  float distancia = lerDistancia();

  // Faixa típica útil do HC-SR04 ~2 a 400 cm
  if (distancia <= 0 || distancia > 400) {
    server.send(500, "application/json", "{\"status\":\"Erro na leitura do HC-SR04\"}");
    return;
  }

  String json = "{";
  json += "\"distancia_cm\":" + String(distancia, 1);
  json += "}";

  server.send(200, "application/json", json);
}

// ----------- Setup -----------
void setup() {
  Serial.begin(115200);

  // Pinos do HC-SR04
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Inicializa DHT
  dht.begin();

  // WiFi
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.localIP());

  // Rotas
  server.on("/", handleRoot);
  server.on("/dht", HTTP_GET, handleDHT);
  server.on("/distancia", HTTP_GET, handleDistancia);

  // Inicia servidor
  server.begin();
  Serial.println("Servidor HTTP iniciado!");
}

// ----------- Loop -----------
void loop() {
  server.handleClient();
}

// ---------------- Função para medir distância ----------------
float lerDistancia() {
  // Trigger de 10us
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Timeout de 30ms (~5 m de alcance máx. teórico)
  unsigned long duracao = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (duracao == 0) return -1.0; // sem eco dentro do timeout

  // Velocidade do som ~0,0343 cm/us; divide por 2 (ida e volta)
  return (duracao * 0.0343f) / 2.0f;
}

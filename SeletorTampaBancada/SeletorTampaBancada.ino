#include <WiFi.h>
#include <WebServer.h>

// ===== CONFIGURAÇÕES WiFi =====
const char* ssid = "Smart 4.0 (3)";
const char* password = "Smart4.0";

WebServer server(80);
 
// ===== PINOS =====
#define LED_R 25
#define LED_G 26
#define LED_B 27

#define DIR_PIN 12
#define STEP_PIN 14

// Botões = fim-de-curso
#define BTN_1 0
#define BTN_2 4
#define BTN_3 16

// Variáveis de controle
int ultimaPosicao = 0;   // Última posição atingida (0-3)
int posicaoDesejada = 0; // Requisição REST
bool emMovimento = false;

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  // Configuração LEDs
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  // Pinos motor
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);

  // Fim de curso
  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
  pinMode(BTN_3, INPUT_PULLUP);

  // Liga LED vermelho ao ligar
  digitalWrite(LED_R, HIGH);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);

  // Conectar WiFi
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");

  // Mostrar IP no Serial
  Serial.print("IP do ESP32 (API Server): http://");
  Serial.println(WiFi.localIP());

  // LED Azul = conectado
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_B, HIGH);

  // Endpoints REST
  server.on("/", handleRoot);
  server.on("/api/move_pos", handleMovePos);
  server.on("/api/last_result", handleLastResult);
  server.on("/api/info", handleInfo);

  server.begin();
  Serial.println("Servidor iniciado. Use o navegador para acessar o IP acima.");
}

// ===== LOOP =====
void loop() {
  server.handleClient();
}

// ===== ENDPOINTS =====
void handleRoot() {
  String html = "<h2>Servidor ESP32</h2>";
  html += "<p>IP: " + WiFi.localIP().toString() + "</p>";
  html += "<p>Use /api/move_pos?pos=X para mover o motor</p>";
  html += "<p>Use /api/last_result para verificar a ultima posicao</p>";
  html += "<p>Use /api/info para ver informacoes da rede</p>";
  server.send(200, "text/html", html);
}

void handleInfo() {
  String resposta = "IP: " + WiFi.localIP().toString();
  resposta += "\nSSID: " + String(ssid);
  resposta += "\nRSSI (sinal WiFi): " + String(WiFi.RSSI()) + " dBm";
  server.send(200, "text/plain", resposta);
}

void handleMovePos() {
  if (server.hasArg("pos")) {
    posicaoDesejada = server.arg("pos").toInt();
    Serial.print("Movendo para posicao: ");
    Serial.println(posicaoDesejada);

    moverMotor(posicaoDesejada);

    server.send(200, "text/plain", "Movimento concluido");
  } else {
    server.send(400, "text/plain", "Parametro 'pos' nao informado");
  }
}

void handleLastResult() {
  String resposta = "Ultima posicao: " + String(ultimaPosicao);
  server.send(200, "text/plain", resposta);
}

// ===== CONTROLE MOTOR =====
void moverMotor(int destino) {
  emMovimento = true;

  // LED Verde ligado durante movimento
  digitalWrite(LED_B, LOW);
  digitalWrite(LED_G, HIGH);

  int passos = 0;
  ultimaPosicao = 0;

  // Direção (para simplificar: destino > ultimaPosicao = horário)
  if (destino > ultimaPosicao) {
    digitalWrite(DIR_PIN, HIGH);
  } else {
    digitalWrite(DIR_PIN, LOW);
  }

  while (ultimaPosicao != destino) {
    passos++;

    // Um passo
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(500);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(500);

    // Simulação dos Fim-de-Curso
    if (digitalRead(BTN_1) == LOW) ultimaPosicao = 1;
    else if (digitalRead(BTN_2) == LOW) ultimaPosicao = 2;
    else if (digitalRead(BTN_3) == LOW) ultimaPosicao = 3;

    if (ultimaPosicao == destino) break;
  }

  // Movimento concluído
  emMovimento = false;
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, HIGH);

  Serial.print("Posicao atingida: ");
  Serial.println(ultimaPosicao);
}

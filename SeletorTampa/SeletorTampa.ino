#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal.h>

// ===== CONFIGURAÇÕES =====
const char* ssid = "Smart 4.0 (3)";
const char* password = "Smart4.0";

WebServer server(80);

// LCD paralelo: RS, EN, D4, D5, D6, D7
LiquidCrystal lcd(32, 33, 23, 22, 21, 18);

// LEDs RGB
#define LED_R 25
#define LED_G 26
#define LED_B 27

// Botões
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

  // Config. Botões
  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
  pinMode(BTN_3, INPUT_PULLUP);

  
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi conectado!");
  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.localIP());


  // Liga LED vermelho ao ligar
  digitalWrite(LED_R, HIGH);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);

  // LCD
  lcd.begin(16, 2);  // inicializa display 16x2
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");

  // Conectar WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // LED Azul = conectado
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_B, HIGH);

  // Endpoints REST
  server.on("/api/move_pos", handleMovePos);
  server.on("/api/last_result", handleLastResult);

  server.begin();
}

// ===== LOOP =====
void loop() {
  server.handleClient();
}

// ===== ENDPOINTS =====
void handleMovePos() {
  if (server.hasArg("pos")) {
    posicaoDesejada = server.arg("pos").toInt();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Movendo para:");
    lcd.setCursor(0,1);
    lcd.print("Pos "); lcd.print(posicaoDesejada);

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

// ===== SIMULACAO MOTOR =====
void moverMotor(int destino) {
  emMovimento = true;

  // LED Verde ligado durante movimento
  digitalWrite(LED_B, LOW);
  digitalWrite(LED_G, HIGH);

  int passos = 0;
  ultimaPosicao = 0;

  while (ultimaPosicao != destino) {
    passos++;
    delay(200); // simula 1 passo (~1.8 graus)

    // Atualiza LCD
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("STEP-MOTOR_MOVE");
    lcd.setCursor(0,1);
    lcd.print("Passos: ");
    lcd.print(passos);

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

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("STEP-MOTOR_STOP");
  lcd.setCursor(0,1);
  lcd.print("Passos: ");
  lcd.print(passos);
}

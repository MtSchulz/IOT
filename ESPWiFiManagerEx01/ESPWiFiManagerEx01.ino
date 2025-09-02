#include <WiFi.h>
#include <WiFiManager.h>  // Biblioteca WiFi Manager

// Definições de hardware
#define RESET_BUTTON 0   // Botão de reset no GPIO0
#define STATUS_LED 25    // LED simples no GPIO25

// ----- RGB -----
#define LED_R 14
#define LED_G 33
#define LED_B 32

// Instância do WiFiManager
WiFiManager wm;

// Protótipos das funções de callback
void configModeCallback(WiFiManager *myWiFiManager);
void saveConfigCallback();
void resetConfigCallback();

// ===== Função para controlar cor do LED RGB =====
void setColor(bool r, bool g, bool b) {
  digitalWrite(LED_R, r ? HIGH : LOW);
  digitalWrite(LED_G, g ? HIGH : LOW);
  digitalWrite(LED_B, b ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== Iniciando ESP32 com WiFiManager ===");

  // Configurações dos pinos
  pinMode(RESET_BUTTON, INPUT_PULLUP);
  pinMode(STATUS_LED, OUTPUT);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  // Apaga tudo no início
  setColor(0, 0, 0);
  digitalWrite(STATUS_LED, LOW);

  // Associa as funções de callback
  wm.setAPCallback(configModeCallback);
  wm.setSaveConfigCallback(saveConfigCallback);
  wm.setConfigResetCallback(resetConfigCallback);

  // Verifica se o botão está pressionado na inicialização
  if (digitalRead(RESET_BUTTON) == LOW) {
    Serial.println("Botão pressionado na inicialização!");
    Serial.println("Limpando credenciais WiFi...");
    wm.resetSettings();
    delay(3000);
  }

  // Tenta conectar ao último WiFi salvo
  // Caso não consiga, abre o portal de configuração
  if (!wm.autoConnect("REDE_WILSCHULZ", "12345678")) {
    Serial.println("Falha ao conectar e tempo limite esgotado!");
    Serial.println("Reiniciando ESP32...");
    ESP.restart();
    delay(1000);
  }

  // Se chegou aqui, conectou com sucesso
  Serial.println("Conectado ao WiFi com sucesso!");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // LED azul indica conectado
  setColor(0, 0, 1);
  digitalWrite(STATUS_LED, HIGH);
}

void loop() {
  // Se perder a conexão, pisca o LED simples
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
    Serial.println("Tentando reconectar...");
    delay(500);
  } else {
    digitalWrite(STATUS_LED, HIGH);
    setColor(0, 0, 1); // Azul fixo se conectado
  }

  // Se o botão for pressionado durante a execução
  if (digitalRead(RESET_BUTTON) == LOW) {
    Serial.println("Botão pressionado durante execução!");
    Serial.println("Limpando credenciais e reiniciando portal...");
    wm.resetSettings();
    ESP.restart();
  }

  delay(1000);
}

// =================== CALLBACKS DO WIFIMANAGER =====================

// Callback quando entra no modo AP (portal de configuração)
void configModeCallback(WiFiManager *myWiFiManager) {
  Serial.println("Entrou no modo AP (Configuração)");
  Serial.print("SSID do AP: ");
  Serial.println(myWiFiManager->getConfigPortalSSID());
  Serial.print("IP do AP: ");
  Serial.println(WiFi.softAPIP());

  // Vermelho piscando enquanto está no AP
  for (int i = 0; i < 100; i++) {
    setColor(1, 0, 0);
    delay(500);
    setColor(0, 0, 0);
    delay(500);
  }
}

// Callback: quando salva novas credenciais
void saveConfigCallback() {
  Serial.println("Novas credenciais WiFi salvas na memória!");
}

// Callback: quando as configurações são resetadas
void resetConfigCallback() {
  Serial.println("Credenciais WiFi apagadas (resetConfigCallback).");
}

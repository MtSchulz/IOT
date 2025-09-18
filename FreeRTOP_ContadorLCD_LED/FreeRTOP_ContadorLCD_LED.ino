#include <Arduino.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(32,33,23,22,21,18);

const int botao = 0;
const int led = 27;

volatile bool ledLigado = false;
volatile int contador = 0;

void IRAM_ATTR isrBotao() {
  ledLigado = !ledLigado;
}

void tarefaLedLCD(void *pvParameters) {
  lcd.begin(16,2);
  for(;;) {
    if(ledLigado) {
      digitalWrite(led, 1);
      vTaskDelay(500 / portTICK_PERIOD_MS);
      digitalWrite(led, 0);
      vTaskDelay(500 / portTICK_PERIOD_MS);
    } else {
      digitalWrite(led, 0 );
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    lcd.setCursor(0,0);
    lcd.print("Contador: ");
    lcd.setCursor(9,0);
    lcd.print("       ");
    lcd.setCursor(9,0);
    lcd.print(contador);

    lcd.setCursor(0,1);
    lcd.print("LED: ");
    lcd.setCursor(5,1);
    lcd.print("     ");
    lcd.setCursor(5,1);
    lcd.print(ledLigado ? "ON" : "OFF");
  }
}

void tarefaContador(void *pvParameters) {
  for(;;) {
    contador++;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(botao, INPUT);
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  attachInterrupt(digitalPinToInterrupt(botao), isrBotao, FALLING);

  xTaskCreatePinnedToCore(
    tarefaLedLCD,       // Função a ser executada
    "TaskLedLCD",       // Nome da Task
    4096,               // Tamanho da Stack em Bytes
    NULL,               // Parametro se necessários
    1,                  // Prioridade
    NULL,               // Handle para manipulação futura
    0                   // Alocado no Core 0
  );

  xTaskCreatePinnedToCore(tarefaContador, "TaskContador", 2048, NULL, 1, NULL, 1); 

}

void loop() {

}

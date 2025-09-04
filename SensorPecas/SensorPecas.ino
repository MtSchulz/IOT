#include <LiquidCrystal.h>

LiquidCrystal lcd(32, 33, 23, 22, 21, 18);

#define TRIG_PIN 13
#define ECHO_PIN 35

#define RGB_R 25
#define RGB_G 26
#define RGB_B 27 

#define BTN_1 0
#define BTN_2 4
#define BTN_3 16
#define BTN_4 17

bool modoAutomatico = true;

// ---------------- Funções ----------------
void mostrarTitulo() {
  lcd.clear();
  lcd.setCursor(3, 0);  
  lcd.print("Sensor de");
  lcd.setCursor(5, 1);  
  lcd.print("Pecas");
  delay(5000);
}

float lerDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW); 
  long duracao = pulseIn(ECHO_PIN, HIGH);
  return duracao * 0.034 / 2;
}

void modoAutomaticoFunc() {
  float distancia = lerDistancia();

  if (distancia > 20) {  
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, LOW);
  }
  else if (distancia >= 3 && distancia <= 7) {  
    digitalWrite(RGB_R, HIGH);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, LOW);
    lcd.setCursor(0, 0);
    lcd.print("Recipiente: A");
  }
  else if (distancia >= 8 && distancia <= 12) { 
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, HIGH);
    digitalWrite(RGB_B, LOW);
    lcd.setCursor(0, 0);
    lcd.print("Recipiente: B");
  }
  else if (distancia >= 13 && distancia <= 17) { 
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Recipiente: C");
  }
  else {
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, LOW);
  }
}

void modoManualFunc() {
  if (digitalRead(BTN_1) == LOW) { // Vermelho
    digitalWrite(RGB_R, HIGH);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, LOW);
    lcd.setCursor(0, 0);
    lcd.print("Recipiente: A");
  }
  else if (digitalRead(BTN_2) == LOW) { // Azul
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Recipiente: C");
  }
  else if (digitalRead(BTN_3) == LOW) { // Verde
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, HIGH);
    digitalWrite(RGB_B, LOW);
    lcd.setCursor(0, 0);
    lcd.print("Recipiente: B");
  }
  else {
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, LOW);
    lcd.setCursor(0, 0);
    lcd.print("Manual: ON");
  }
}

// ---------------- Setup ----------------
void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);

  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
  pinMode(BTN_3, INPUT_PULLUP);
  pinMode(BTN_4, INPUT_PULLUP);

  mostrarTitulo();
}

// ---------------- Loop ----------------
void loop() {
  // Alternar modo com BTN_4
  if (digitalRead(BTN_4) == LOW) {
    modoAutomatico = !modoAutomatico;
    delay(300); // debounce
  }

  lcd.clear();

  if (modoAutomatico) {
    modoAutomaticoFunc();
  } else {
    modoManualFunc();
  }

  delay(100);
}
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Smart 4.0 (3)";
const char* password = "Smart4.0";

const char* mqtt_server = "10.74.241.95"; // IP da Máquina

WiFiClient espClient;
PubSubClient client(espClient);

int ledRGB_R = 27;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico [");
  Serial.print(topic);
  Serial.print("]: ");
  String mensagem;
  for(int i=0; i< length; i++){
    mensagem += (char)payload[i];
  }

  Serial.println(mensagem);

  if( mensagem == "ON" ){
    digitalWrite(ledRGB_R, HIGH);
    Serial.println("Led Ligado!");
  }else if( mensagem == "ON" ){
    digitalWrite(ledRGB_R, LOW);
    Serial.println("Led Desligado!");
  }
}

void setup_wifi() {
  delay(10);
  Serial.print("Conectando ao WiFi...");
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED ){
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}


// ------- Reconexão MQTT -------
void reconnect(){
  while(!client.connected()){
    Serial.print("Tentando reconexão ao Broker...");
    if(client.connect("Esp32Client")){
      Serial.print("Conectado");
      client.subscribe("/smart4.0/temp");
    }else{
      Serial.print("Falhou, rc=");
      Serial.print(client.state());
      Serial.println(" Testando novamente em 2s");
      delay(2000);
    }
  }
}

void setup(){
  pinMode(ledRGB_R, HIGH);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if( !client.connected() ){
    reconnect();
  }

  client.loop();

  // realizando um publish

  String msg = "{\"temperatura\":25.43}";
  client.publish("/smart4.0/temp", msg.c_str());
  delay(500);
}

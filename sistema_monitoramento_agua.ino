
#include <WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>

// Definições de pinos
#define PIN_PH_SENSOR    36  // GPIO36 (VDET_1)
#define PIN_TURBIDITY    39  // GPIO39 (VDET_2)
#define PIN_VALVE_RELAY  23  // GPIO23 para controle da válvula solenoide

// Constantes para calibração dos sensores
#define PH_OFFSET        0.00
#define PH_SAMPLES       10
#define TURBIDITY_SAMPLES 10

// Limites para acionamento da válvula
#define PH_MIN           6.5
#define PH_MAX           8.5
#define TURBIDITY_MAX    50.0  // NTU

// Configurações de rede Wi-Fi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Configurações MQTT
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_client_id = "ESP32_MonitorAgua";
const char* mqtt_topic_ph = "agua/ph";
const char* mqtt_topic_turbidity = "agua/turbidez";
const char* mqtt_topic_valve = "agua/valvula";

// Variáveis globais
float ph_value = 0.0;
float turbidity_value = 0.0;
bool valve_state = false;
unsigned long last_reading_time = 0;
const unsigned long reading_interval = 5000; // 5 minutos em milissegundos

// Objetos para Wi-Fi e MQTT
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // Inicialização da comunicação serial
  Serial.begin(115200);
  Serial.println("Inicializando Sistema de Monitoramento da Água...");
  
  // Configuração dos pinos
  pinMode(PIN_VALVE_RELAY, OUTPUT);
  digitalWrite(PIN_VALVE_RELAY, LOW); // Válvula fechada inicialmente
  
  // Inicialização da EEPROM para armazenar dados de calibração
  EEPROM.begin(512);
  
  // Conexão Wi-Fi
  setup_wifi();
  
  // Configuração MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  Serial.println("Sistema inicializado e pronto para monitoramento!");
}

void loop() {
  // Verifica conexão MQTT e reconecta se necessário
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();
  
  // Verifica se é hora de fazer uma nova leitura (a cada 5 minutos)
  unsigned long current_time = millis();
  if (current_time - last_reading_time >= reading_interval || last_reading_time == 0) {
    last_reading_time = current_time;
    
    // Leitura dos sensores
    read_sensors();
    
    // Processamento e tomada de decisão
    process_data();
    
    // Publicação dos dados via MQTT
    publish_data();
    
    // Exibe informações no monitor serial
    print_data();
  }
}

// Função para configurar conexão Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando à rede Wi-Fi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("Wi-Fi conectado");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

// Função de callback para receber mensagens MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);
  
  // Implementar lógica para receber comandos remotos se necessário
}

// Função para reconectar ao broker MQTT
void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    if (client.connect(mqtt_client_id)) {
      Serial.println("conectado");
      
      // Inscrição em tópicos para receber comandos (se necessário)
      // client.subscribe("agua/comandos");
    } else {
      Serial.print("falha, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

// Função para ler os sensores
void read_sensors() {
  // Leitura do sensor de pH com média de amostras
  float ph_sum = 0;
  for (int i = 0; i < PH_SAMPLES; i++) {
    int ph_raw = analogRead(PIN_PH_SENSOR);
    float voltage = ph_raw * (3.3 / 4095.0);
    // Conversão de tensão para pH (calibração aproximada)
    // pH = 7 - ((2.5 - voltage) / 0.18)
    float ph_sample = 7 - ((2.5 - voltage) / 0.18) + PH_OFFSET;
    ph_sum += ph_sample;
    delay(10);
  }
  ph_value = ph_sum / PH_SAMPLES;
  
  // Leitura do sensor de turbidez com média de amostras
  float turbidity_sum = 0;
  for (int i = 0; i < TURBIDITY_SAMPLES; i++) {
    int turbidity_raw = analogRead(PIN_TURBIDITY);
    
    // Reduz a sensibilidade mapeando para uma faixa menor
    float turbidity_sample = map(turbidity_raw, 0, 4095, 0, 90);
    
    // Alternativa com a fórmula original
    // float voltage = turbidity_raw * (3.3 / 4095.0);
    // float turbidity_sample = (-1120.4 * voltage * voltage + 5742.3 * voltage - 4352.9) / 4;
    
    turbidity_sum += turbidity_sample;
    delay(10);
  }
  turbidity_value = turbidity_sum / TURBIDITY_SAMPLES;
}

// Função para processar dados e tomar decisões
void process_data() {
  // Verifica se os parâmetros estão fora dos limites aceitáveis
  bool ph_out_of_range = (ph_value < PH_MIN || ph_value > PH_MAX);
  bool turbidity_out_of_range = (turbidity_value > TURBIDITY_MAX);
  
  // Aciona a válvula se necessário
  if (ph_out_of_range || turbidity_out_of_range) {
    digitalWrite(PIN_VALVE_RELAY, HIGH); // Fecha a válvula
    valve_state = true;
    Serial.println("ALERTA: Parâmetros fora do limite! Válvula fechada.");
  } else {
    digitalWrite(PIN_VALVE_RELAY, LOW); // Abre a válvula
    valve_state = false;
  }
}

// Função para publicar dados via MQTT
void publish_data() {
  // Converte valores para strings
  char ph_str[10];
  char turbidity_str[10];
  char valve_str[6];
  
  dtostrf(ph_value, 4, 2, ph_str);
  dtostrf(turbidity_value, 6, 2, turbidity_str);
  strcpy(valve_str, valve_state ? "FECHADA" : "ABERTA");
  
  // Publica os dados nos tópicos MQTT
  client.publish(mqtt_topic_ph, ph_str);
  client.publish(mqtt_topic_turbidity, turbidity_str);
  client.publish(mqtt_topic_valve, valve_str);
  
  Serial.println("Dados publicados via MQTT");
}

// Função para exibir dados no monitor serial
void print_data() {-
  Serial.println("----------------------------------------");
  Serial.println("LEITURA DE SENSORES:");
  Serial.print("pH: ");
  Serial.println(ph_value, 2);
  Serial.print("Turbidez: ");
  Serial.print(turbidity_value, 2);
  Serial.println(" NTU");
  Serial.print("Estado da válvula: ");
  Serial.println(valve_state ? "FECHADA" : "ABERTA");
  Serial.println("----------------------------------------");
}

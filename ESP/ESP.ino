#include <WiFi.h>
#include <Arduino.h>

// Configuración de tu red Wi-Fi
const char* ssid = "IoT-B19";     
const char* password = "lcontrol2020*"; 

// Pines de entrada y salida
const int input_pins[] = {13, 12};   
const int output_pins[] = {2, 4};    
const int analog_pin = 14;           

// Estados previos de los pines de entrada
int previous_states[] = {LOW, LOW};  

WiFiServer server(5001);  

void setup() {
  pinMode(input_pins[0], INPUT);
  pinMode(input_pins[1], INPUT);
  pinMode(output_pins[0], OUTPUT);
  pinMode(output_pins[1], OUTPUT);
  pinMode(analog_pin, OUTPUT);  

  Serial.begin(115200);
  Serial.println("Iniciando...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a Wi-Fi...");
  }
  Serial.println("Conectado a Wi-Fi.");
  Serial.print("Dirección IP del ESP32: ");
  Serial.println(WiFi.localIP()); 
  server.begin();
}

void loop() {
  read_switches();  
  handle_client();  
  delay(1000);  
}

void read_switches() {
  String state_message = "";

  for (int i = 0; i < 2; i++) {
    int current_state = digitalRead(input_pins[i]);
    if (current_state != previous_states[i]) {  
      previous_states[i] = current_state;  
      // Cambiar el mensaje a un formato más simple
      String message = "Pin:" + String(input_pins[i]) + ":" + String(current_state);
      Serial.println(message);  

      if (state_message.length() > 0) {
        state_message += ",";  
      }
      state_message += message;  
    }
  }

  if (state_message.length() > 0) {
    send_state(state_message);  
  }
}

void send_state(String state_message) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    if (client.connect("192.168.100.111", 5000)) {  
      // Enviar la solicitud GET con el nuevo formato
      client.print("GET /update?state=" + state_message + " HTTP/1.1\r\n");
      client.println("Host: 192.168.100.111");
      client.println("Connection: close");
      client.println();  // Línea en blanco para terminar la solicitud
    } else {
      Serial.println("Error al conectar al servidor Flask");
    }
  } else {
    Serial.println("WiFi no conectado");
  }
}

void toggle_led(int pin) {
  digitalWrite(pin, !digitalRead(pin));  
  Serial.print("LED en pin ");
  Serial.print(pin);
  Serial.print(" cambiado a ");
  Serial.println(digitalRead(pin) == HIGH ? "Encendido" : "Apagado");
}

void handle_client() {
  WiFiClient client = server.available();  

  if (client) {
    Serial.println("Cliente conectado");

    while (client.connected()) {
      if (client.available()) {
        String command = client.readStringUntil('\n');
        command.trim();  

        if (command.startsWith("TOGGLE")) {
          int pin = command.substring(7).toInt();
          toggle_led(pin);  
        }

        if (command.startsWith("ANALOG")) {
          int value = command.substring(7).toInt();
          int analog_value = map(value, 0, 100, 0, 255);  
          analogWrite(analog_pin, analog_value);  
          Serial.print("Potencia del LED en pin ");
          Serial.print(analog_pin);
          Serial.print(" ajustada a ");
          Serial.println(value);
        }
      }
    }

    client.stop();  
    Serial.println("Cliente desconectado");
  }
}


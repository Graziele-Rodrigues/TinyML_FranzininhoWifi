#include "DHT.h"
#define FREQUENCY_HZ        50
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ + 1))

#define DHTPIN 15     // Pino digital conectado ao sensor DHT
#define DHTTYPE DHT11 // Tipo de sensor DHT
DHT dht(DHTPIN, DHTTYPE);

static unsigned long last_interval_ms = 0;

void setup() {
  Serial.begin(115200);
  Serial.println(F("Inicializando DHT..."));
  dht.begin();
}

void loop() {
   if (millis() > last_interval_ms + INTERVAL_MS) {
        last_interval_ms = millis();

        // Ler a umidade e a temperatura
        float h = dht.readHumidity();
        float t = dht.readTemperature();

        // Verifica se a leitura falhou
        if (isnan(h) || isnan(t)) {
          Serial.println(F("Falha ao ler o sensor DHT!"));
          return;
        }

        // Obter o timestamp atual em milissegundos
        unsigned long timestamp = millis();

        // Enviar os dados para a Edge Impulse via serial com timestamp
        Serial.print(timestamp);
        Serial.print(",");
        Serial.print(t);
        Serial.print(",");
        Serial.println(h);

        // Exemplo de saída: "123456,23.40,56.00"
  }
}

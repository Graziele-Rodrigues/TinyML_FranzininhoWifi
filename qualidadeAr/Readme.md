# Projeto de Inferência com Sensor DHT e Edge Impulse

Este projeto demonstra a integração entre um sensor DHT e o Edge Impulse para realizar inferências baseadas em dados de temperatura e umidade. O

## Hardware Necessário

- Sensor DHT11
- Placa de microcontrolador (Franzininho Wifi Lab)

## Bibliotecas Necessárias

- [DHT Sensor Library](https://github.com/adafruit/DHT-sensor-library) - para leitura do sensor DHT.
- [Edge Impulse Arduino Library](https://docs.edgeimpulse.com/docs/run-inference/arduino-library) - para comunicação com o Edge Impulse.

## Configuração do Hardware

1. Conecte o pino de dados do sensor DHT ao pino digital 15 do microcontrolador.
2. Certifique-se de conectar os pinos de alimentação e terra do sensor adequadamente.

## Código

### Código de Coleta de Dados

Este código realiza a leitura dos dados do sensor DHT e envia os dados com timestamps pela serial para o Edge Impulse, usando a api explicada aqui https://docs.edgeimpulse.com/docs/tools/edge-impulse-cli/cli-installation.

```cpp
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

        float h = dht.readHumidity();
        float t = dht.readTemperature();

        if (isnan(h) || isnan(t)) {
          Serial.println(F("Falha ao ler o sensor DHT!"));
          return;
        }

        unsigned long timestamp = millis();

        Serial.print(timestamp);
        Serial.print(",");
        Serial.print(t);
        Serial.print(",");
        Serial.println(h);
  }
}
```

### Código Principal

Este código realiza a leitura dos dados do sensor DHT e executa a inferência usando o modelo treinado no Edge Impulse.

```cpp
#include <DHT.h>
#include <graziele_rodrigues-project-1_inferencing.h>

#define DHTPIN 15     // Pino digital conectado ao sensor DHT
#define DHTTYPE DHT11 // Tipo de sensor DHT
DHT dht(DHTPIN, DHTTYPE);

#define FREQUENCY_HZ        50
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ + 1))

static bool debug_nn = false;
static unsigned long last_interval_ms = 0;

void setup()
{
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Edge Impulse Inferencing Demo");

    Serial.println(F("Inicializando DHT..."));
    dht.begin();

    if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 2) {
        ei_printf("ERR: EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME deve ser igual a 2 (temperatura e umidade)\n");
        return;
    }
}

void loop()
{
    if (millis() > last_interval_ms + INTERVAL_MS) {
        last_interval_ms = millis();

        ei_printf("\nIniciando inferência...\n");

        float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };

        for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += 3) {
          buffer[ix + 0] = (float)millis();
          float h = dht.readHumidity();
          float t = dht.readTemperature();

          if (isnan(h) || isnan(t)) {
              ei_printf("Falha ao ler o sensor DHT!\n");
              return;
          }

          buffer[ix + 1] = t;
          buffer[ix + 2] = h;
      }

        signal_t signal;
        int err = numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
        if (err != 0) {
            ei_printf("Falha ao criar sinal do buffer (%d)\n", err);
            return;
        }

        ei_impulse_result_t result = { 0 };

        err = run_classifier(&signal, &result, debug_nn);
        if (err != EI_IMPULSE_OK) {
            ei_printf("ERR: Falha ao executar classificador (%d)\n", err);
            return;
        }

        ei_printf("Timestamp: %lu ms\n", millis());
        ei_printf("Previsões ");
        ei_printf("(DSP: %d ms., Classificação: %d ms., Anomalia: %d ms.)",
            result.timing.dsp, result.timing.classification, result.timing.anomaly);
        ei_printf(": \n");
        for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
            ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
        }
    }
}
```

## Como Executar

1. Carregue o código principal no seu microcontrolador.
2. Conecte o microcontrolador ao seu computador e abra o monitor serial.
3. O microcontrolador irá começar a capturar dados e realizar inferências com base no modelo treinado.

## Contribuição

Sinta-se à vontade para contribuir com melhorias ou sugestões para o projeto!

## Licença

Este projeto é licenciado sob a [MIT License](LICENSE).
```

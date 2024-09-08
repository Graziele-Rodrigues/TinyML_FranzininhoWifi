/* Includes ---------------------------------------------------------------- */
#include <DHT.h>
#include <graziele_rodrigues-project-1_inferencing.h>

#define DHTPIN 15     // Pino digital conectado ao sensor DHT
#define DHTTYPE DHT11 // Tipo de sensor DHT
DHT dht(DHTPIN, DHTTYPE);

#define FREQUENCY_HZ        50
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ + 1))

/* Private variables ------------------------------------------------------- */
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
static unsigned long last_interval_ms = 0;


void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    // comment out the below line to cancel the wait for USB connection (needed for native USB)
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

        // Allocate a buffer here for the values we'll read from the DHT sensor
        float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };

        for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += 3) {
          // Captura o timestamp
          buffer[ix + 0] = (float)millis();
          // Obter a próxima leitura de umidade e temperatura
          float h = dht.readHumidity();
          float t = dht.readTemperature();

          // Verifica se a leitura falhou
          if (isnan(h) || isnan(t)) {
              ei_printf("Falha ao ler o sensor DHT!\n");
              return;
          }

          // Preencher o buffer com os valores de temperatura e umidade
          buffer[ix + 1] = t;
          buffer[ix + 2] = h;
      }

        // Transformar o buffer bruto em um sinal para classificar
        signal_t signal;
        int err = numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
        if (err != 0) {
            ei_printf("Falha ao criar sinal do buffer (%d)\n", err);
            return;
        }

        // Executar o classificador
        ei_impulse_result_t result = { 0 };

        err = run_classifier(&signal, &result, debug_nn);
        if (err != EI_IMPULSE_OK) {
            ei_printf("ERR: Falha ao executar classificador (%d)\n", err);
            return;
        }

        // Imprimir as previsões com timestamp
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


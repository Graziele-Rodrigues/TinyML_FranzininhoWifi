#include <clima_inferencing.h>

#include <DHT.h>
#include <Wire.h> 
#include <Adafruit_BMP085.h>

static bool debug_nn = true; // true para conseguir visualizar as features geradas


#define DHTPIN 15     // Pino digital conectado ao sensor DHT
#define DHTTYPE DHT11 // Tipo de sensor DHT
DHT dht(DHTPIN, DHTTYPE);
 
//Adafruit_BMP085 bmp; //Adafruit_BMP085 (I2C)

float sensacao_termica(float temperatura, float umidade){
  float sensacao = temperatura -((0.55-0.0055*umidade)*(temperatura-14.5));
  return sensacao;
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Inicializando DHT..."));
  dht.begin();
  //if (!bmp.begin()) {
    //Serial.println("Nao foi possivel iniciar bmp, verifique os jumpers");
    //while (1) {}
  //}
  if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 5) {
      ei_printf("ERRO!)\n");
      return;
    }

}

void loop() {

  float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };
  for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += 5) {
    ei_printf("\nInicia em 60 segundos...\n");
    delay(60000);
    // Captura o timestamp
    buffer[ix + 0] = (float)millis();

    // Leitura da temperatura e umidade
    float t = dht.readTemperature();
    float u = dht.readHumidity();
    // Calcula sensacao termica
    float s = sensacao_termica(t, u);
    // Leitura pressao
    float p =bmp.readSealevelPressure()/100.0;
    
    // Dados da base de dados que indicam chuva 
    //float t = random(165, 167+ 1)/10.0;
    //float u = random(901, 905+ 1)/10.0;
    //float s = random(166, 168+ 1)/10.0;
    //float p = random(101490, 101520 + 1)/100.0;

    // Preencher o buffer 
    buffer[ix + 1] = t;
    buffer[ix + 2] = s;
    buffer[ix + 3] = u;
    buffer[ix + 4] = p;

    // Transformar o buffer bruto em um sinal para classificar
    signal_t signal;
    int err = numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) {
      ei_printf("Falha ao criar sinal do buffer (%d)\n", err);
      return;
    }

    // Roda classificador
    ei_impulse_result_t result = { 0 };

    err = run_classifier(&signal, &result, debug_nn);
    if (err != EI_IMPULSE_OK) {
      ei_printf("ERR: Failed to run classifier (%d)\n", err);
      return;
    }

    // Predicoes
    ei_printf("Predictions ");
    ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
    result.timing.dsp, result.timing.classification, result.timing.anomaly);
    ei_printf(": \n");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
      ei_printf("%s: %.3f\n", result.classification[ix].label, result.classification[ix].value);
    }

  }

}

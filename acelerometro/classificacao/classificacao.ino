#include <accel_inferencing.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal

#define LIS3DH_CLK 36
#define LIS3DH_MISO 37
#define LIS3DH_MOSI 35
#define LIS3DH_CS 34


#define LED_VERMELHO 14  // LED vermelho para "caminhando"
#define LED_AZUL 12      // LED azul para "pulando"
#define LED_VERDE 13     // LED verde para "parado"

// software SPI
Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS, LIS3DH_MOSI, LIS3DH_MISO, LIS3DH_CLK);

void setup(void) {
  Serial.begin(115200);

  // Inicializa LEDs
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  
  // Apaga todos os LEDs inicialmente
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(LED_AZUL, LOW);
  digitalWrite(LED_VERDE, LOW);

  if (!lis.begin(0x18)) {   
    Serial.println("Couldn't start");
    while (1) yield();
  }
  lis.setRange(LIS3DH_RANGE_2_G);   // 2, 4, 8 or 16 G
  lis.setPerformanceMode(LIS3DH_MODE_NORMAL); //LIS3DH_MODE_LOW_POWER, LIS3DH_MODE_NORMAL, LIS3DH_MODE_HIGH_RESOLUTION
  lis.setDataRate(LIS3DH_DATARATE_50_HZ); //1Hz (1 leitura por segundo), 10Hz, 25Hz, 50Hz, 100Hz, 200Hz, 400Hz, POWERDOWN, LOWPOWER_5KHZ, LOWPOWER_1K6HZ
  
  if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 3) {
    ei_printf("ERR: EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME should be equal to 3 (the 3 sensor axes)\n");
    return;
  }

}

void loop() {

  ei_printf("\nClassificacao inicia em 2 segundos...\n");
  delay(2000);

  ei_printf("Sampling...\n");

  // buffer para leitura acelerometro
  float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };
  for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += 3) {
    // proximo next tick 
    uint64_t next_tick = micros() + (EI_CLASSIFIER_INTERVAL_MS * 1000);
    // captura accel X, Y e Z em m/s^2
    sensors_event_t event;
    lis.getEvent(&event);
    Serial.println("----------- Aceleracoes lidas -----------");
    Serial.print(event.acceleration.x);
    Serial.print(",");
    Serial.print(event.acceleration.y);
    Serial.print(",");
    Serial.println(event.acceleration.z);
    // coloca leituras no buffer 
    buffer[ix + 0] = event.acceleration.x;
    buffer[ix + 1] = event.acceleration.y;
    buffer[ix + 2] = event.acceleration.z;
    delayMicroseconds(next_tick - micros());
  }

  // Turn the raw buffer in a signal which we can the classify
  signal_t signal;
  int err = numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  if (err != 0) {
    ei_printf("Failed to create signal from buffer (%d)\n", err);
    return;
  }

  // roda classificador
  ei_impulse_result_t result = { 0 };

  err = run_classifier(&signal, &result, debug_nn);
  if (err != EI_IMPULSE_OK) {
    ei_printf("ERR: Failed to run classifier (%d)\n", err);
    return;
  }

  // predicoes
  ei_printf("Predictions ");
  ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
  result.timing.dsp, result.timing.classification, result.timing.anomaly);
  ei_printf(": \n");
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
  }

  // Controle dos LEDs com base nas predições
  if (result.classification[0].value > 0.8) {  // Label "caminhando"
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, HIGH);
    digitalWrite(LED_AZUL, LOW);
  } else if (result.classification[1].value > 0.8) {  // Label "parado"
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_AZUL, LOW);
  } else if (result.classification[2].value > 0.8) {  // Label "pulando"
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_AZUL, HIGH);
  } else {
    // Se nenhuma predição for suficientemente alta, desliga todos os LEDs
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_AZUL, LOW);
  }

}

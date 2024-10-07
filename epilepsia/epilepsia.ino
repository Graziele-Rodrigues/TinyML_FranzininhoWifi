#include <epilepsia_inferencing.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <UrlEncode.h>

const char* ssid = "seu_ssid";
const char* password = "sua_password";

static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal

#define LIS3DH_CLK 36
#define LIS3DH_MISO 37
#define LIS3DH_MOSI 35
#define LIS3DH_CS 34
// software SPI
Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS, LIS3DH_MOSI, LIS3DH_MISO, LIS3DH_CLK);

String phoneNumber = "+55seu_telefone";
String apiKey = "1628715";

void sendMessage(String message) {
  // Data to send with HTTP POST
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
  HTTPClient http;
  http.begin(url);
  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200) {
    Serial.print("Mensagem enviada com sucesso");
  } else {
    Serial.println("Erro no envio da mensagem");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}

void setup(void) {
  Serial.begin(115200);

  // configuracao acelerometro
  if (!lis.begin(0x18)) {   
    Serial.println("Couldn't start");
    while (1) yield();
  }
  lis.setRange(LIS3DH_RANGE_2_G);   // 2, 4, 8 or 16 G
  lis.setPerformanceMode(LIS3DH_MODE_NORMAL); //LIS3DH_MODE_LOW_POWER, LIS3DH_MODE_NORMAL, LIS3DH_MODE_HIGH_RESOLUTION
  lis.setDataRate(LIS3DH_DATARATE_50_HZ); //1Hz (1 leitura por segundo), 10Hz, 25Hz, 50Hz, 100Hz, 200Hz, 400Hz, POWERDOWN, LOWPOWER_5KHZ, LOWPOWER_1K6HZ
  
  // verificacao numero samples
  if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 6) {
    ei_printf("ERR: EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME should be equal to 6 (the 6 sensor axes)\n");
    return;
  }

  // configuracao conexao wifi
  WiFi.begin(ssid, password);
  Serial.println("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado ao WiFi neste IP ");
  Serial.println(WiFi.localIP());

}

void loop() {

  ei_printf("\nClassificacao inicia em 10 segundos...\n");
  delay(10000);

  ei_printf("Sampling...\n");

  // buffer para leitura acelerometro
  float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };
  for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += 6) {
    // proximo next tick 
    uint64_t next_tick = micros() + (EI_CLASSIFIER_INTERVAL_MS * 1000);
    // captura accel X, Y e Z em m/s^2
    sensors_event_t event;
    lis.getEvent(&event);
    // Calcular o ângulo em relação aos eixos X (roll), Y (pitch) e Z (yaw)
    float roll = (atan2(event.acceleration.y, event.acceleration.z) * 180 / M_PI)*0.1;
    float pitch = (atan2(-event.acceleration.x, sqrt(event.acceleration.y * event.acceleration.y + event.acceleration.z * event.acceleration.z)) * 180 / M_PI)*0.1;
    // Aproximação do ângulo yaw (rotação em torno do eixo Z) usando atan2 com X e Y
    float yaw = (atan2(event.acceleration.y, event.acceleration.x) * 180 / M_PI)*0.1;
    // coloca leituras no buffer 
    buffer[ix + 0] = roll;
    buffer[ix + 1] = pitch;
    buffer[ix + 2] = yaw;
    buffer[ix + 3] = event.acceleration.x;
    buffer[ix + 4] = event.acceleration.y;
    buffer[ix + 5] = event.acceleration.z;
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
  if (result.classification[0].value > 0.8) {  // sem epilepsia
     Serial.println("Sem Epilepsia");
  } else if (result.classification[1].value > 0.8) {  // com epilepsia"
     Serial.println("Epilepsia identificada");
     sendMessage("Olá, você está no contato de emergência de Graziele. Ela está passando por uma crise epiléptica e precisa de ajuda urgente. Por favor, procure-a imediatamente!");
  } else {
    // Se nenhuma predição for suficientemente 
    Serial.println("Nao eh possivel determinar");
  }

}

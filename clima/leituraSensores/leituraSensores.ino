#include <DHT.h>
#include <Wire.h> 
#include <Adafruit_BMP085.h>


#define DHTPIN 15     // Pino digital conectado ao sensor DHT
#define DHTTYPE DHT11 // Tipo de sensor DHT
DHT dht(DHTPIN, DHTTYPE);
 
Adafruit_BMP085 bmp; //Adafruit_BMP085 (I2C)

float sensacao_termica(float temperatura, float umidade){
  float sensacao = temperatura -((0.55-0.0055*umidade)*(temperatura-14.5));
  return sensacao;
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Inicializando DHT..."));
  dht.begin();
  if (!bmp.begin()) {
	  Serial.println("Nao foi possivel iniciar bmp, verifique os jumpers");
	  while (1) {}
  }
}

void loop() {

  // Leitura da temperatura e umidade
  float t = dht.readTemperature();
  float u = dht.readHumidity();
  // Calcula sensacao termica
  float s = sensacao_termica(t, u);

  // Leitura pressao em mb
  float p = (bmp.readSealevelPressure())/100.0;

  // Obter o timestamp atual em milissegundos
  unsigned long timestamp = millis();
  // Imprime Serial
  Serial.print(timestamp);
  Serial.print(",");
  Serial.print(t);
  Serial.print(",");
  Serial.print(s);
  Serial.print(",");
  Serial.print(u);
  Serial.print(",");
  Serial.println(p);
  delay(10000);
}

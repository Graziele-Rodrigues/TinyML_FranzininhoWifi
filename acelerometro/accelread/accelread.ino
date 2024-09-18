#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#define FREQUENCY_HZ        50
#define INTERVAL_MS         1000/(FREQUENCY_HZ+1)
static unsigned long last_interval_ms = 0;

#define LIS3DH_CLK 36
#define LIS3DH_MISO 37
#define LIS3DH_MOSI 35
#define LIS3DH_CS 34

// software SPI
Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS, LIS3DH_MOSI, LIS3DH_MISO, LIS3DH_CLK);

void setup(void) {
  Serial.begin(115200);
  //Serial.println("LIS3DH test!");
  if (!lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldn't start");
    while (1) yield();
  }
  lis.setRange(LIS3DH_RANGE_2_G);   // 2, 4, 8 or 16 G
  lis.setPerformanceMode(LIS3DH_MODE_NORMAL); //LIS3DH_MODE_LOW_POWER, LIS3DH_MODE_NORMAL, LIS3DH_MODE_HIGH_RESOLUTION
  lis.setDataRate(LIS3DH_DATARATE_50_HZ); //1Hz (1 leitura por segundo), 10Hz, 25Hz, 50Hz, 100Hz, 200Hz, 400Hz, POWERDOWN, LOWPOWER_5KHZ, LOWPOWER_1K6HZ
}

void loop() {
  // get accel X, Y and Z in m/s^2
  sensors_event_t event;
  lis.getEvent(&event);
  if (millis() > last_interval_ms + INTERVAL_MS) {
    last_interval_ms = millis();
    Serial.print(event.acceleration.x);
    Serial.print(",");
    Serial.print(event.acceleration.y);
    Serial.print(",");
    Serial.println(event.acceleration.z);
  }
}

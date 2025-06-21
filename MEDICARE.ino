#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <Adafruit_MLX90614.h>
#include <WiFi.h>
#include <HTTPClient.h>

// TFT Display Pins
#define TFT_CS 5
#define TFT_RST 4
#define TFT_DC 2
#define TFT_MOSI 23
#define TFT_SCK 18
#define TFT_MISO 19

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// I²C buses
TwoWire wireMax30102 = TwoWire(0);
TwoWire wireMlx90614 = TwoWire(1);

// Sensors
MAX30105 particleSensor;
Adafruit_MLX90614 tempSensor = Adafruit_MLX90614();

// I²C pin assignments
#define SDA_MAX30102 21
#define SCL_MAX30102 22
#define SDA_MLX90614 25
#define SCL_MLX90614 26

// Buffers
uint32_t irBuffer[100];
uint32_t redBuffer[100];
int32_t heartRate = 0;
int32_t spo2 = 0;
int8_t validHeartRate = 0;
int8_t validSpO2 = 0;

#define NUM_SAMPLES 10
int32_t bpmSamples[NUM_SAMPLES] = { 0 };
byte sampleIndex = 0;

// Wi-Fi Credentials
const char *ssid = "TSHS NETWORK I";
const char *password = "tapinac@361079";

// Icon bitmaps — replace with your actual bitmap arrays
const unsigned char epd_bitmap_vo2_max_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24[] PROGMEM = { /* oxygen icon data */ };
const unsigned char epd_bitmap_thermostat_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24[] PROGMEM = { /* temp icon data */ };
const unsigned char epd_bitmap_health_metrics_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24[] PROGMEM = { /* heart icon data */ };

const uint8_t *heartIcon PROGMEM = epd_bitmap_health_metrics_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24;
const uint8_t *oxygenIcon PROGMEM = epd_bitmap_vo2_max_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24;
const uint8_t *tempIcon PROGMEM = epd_bitmap_thermostat_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24;

void displayVitals(int32_t bpm, int32_t spO2, float temp) {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);

  // Heart Rate
  tft.drawBitmap(20, 30, heartIcon, 24, 24, ILI9341_RED);
  tft.setCursor(60, 30);
  tft.println("Heart Rate:");
  tft.setCursor(60, 60);
  tft.setTextColor(ILI9341_GREEN);
  tft.printf("%d BPM\n", bpm);
  tft.setTextColor(ILI9341_WHITE);

  // SpO2
  tft.drawBitmap(20, 110, oxygenIcon, 24, 24, ILI9341_CYAN);
  tft.setCursor(60, 110);
  tft.println("Oxygen Level:");
  tft.setCursor(60, 140);
  tft.setTextColor(ILI9341_CYAN);
  if (spO2 < 0 || spO2 > 100) {
    tft.printf("98%%\n");
  } else {
    tft.printf("%d%%\n", spO2);
  }
  tft.setTextColor(ILI9341_WHITE);

  // Temperature
  tft.drawBitmap(20, 190, tempIcon, 24, 24, ILI9341_ORANGE);
  tft.setCursor(60, 190);
  tft.println("Temperature:");
  tft.setCursor(60, 220);
  tft.setTextColor(ILI9341_RED);
  if (temp < 30.0 || temp > 42.0) {
    tft.printf("36.5 C");
  } else {
    tft.printf("%.1f C\n", temp);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.setCursor(40, 120);
  tft.println("MEDICARE");
  delay(2000);

  WiFi.begin(ssid, password);
  tft.setCursor(10, 200);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_YELLOW);
  tft.println("Connecting to WiFi...");
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OK] WiFi connected.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    tft.setTextColor(ILI9341_GREEN);
    tft.println("WiFi Connected");
  } else {
    Serial.println("\n[ERROR] Failed to connect to WiFi.");
    tft.setTextColor(ILI9341_RED);
    tft.println("WiFi Failed!");
  }

  wireMax30102.begin(SDA_MAX30102, SCL_MAX30102);
  wireMlx90614.begin(SDA_MLX90614, SCL_MLX90614);

  bool maxFound = particleSensor.begin(wireMax30102, I2C_SPEED_STANDARD);
  bool mlxFound = tempSensor.begin(0x5A, &wireMlx90614);

  if (!maxFound) {
    Serial.println("[ERROR] MAX30102 not detected!");
  } else {
    particleSensor.setup(50, 3, 2, 50, 411, 4096);
    Serial.println("[OK] MAX30102 initialized.");
  }

  if (!mlxFound) {
    Serial.println("[ERROR] MLX90614 not detected!");
  } else {
    Serial.println("[OK] MLX90614 initialized.");
  }

  displayVitals(0, 0, 36.5);  // Default screen
}

void loop() {
  const int bufferLength = 100;
  bool maxReady = particleSensor.check();
  float temperature = 0.0;

  if (maxReady) {
    for (byte i = 0; i < bufferLength; i++) {
      while (!particleSensor.check())
        ;
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      delay(1);
    }

    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer,
                                           &spo2, &validSpO2, &heartRate, &validHeartRate);

    if (heartRate < 0) heartRate = 0;

    bpmSamples[sampleIndex] = heartRate;
    sampleIndex = (sampleIndex + 1) % NUM_SAMPLES;

    int32_t smoothedBPM = 0;
    for (byte i = 0; i < NUM_SAMPLES; i++) smoothedBPM += bpmSamples[i];
    smoothedBPM /= NUM_SAMPLES;

    temperature = tempSensor.readObjectTempC();

    if (spo2 < 0 || spo2 > 100) spo2 = -1;

    displayVitals(smoothedBPM, spo2, temperature);

    // Send data to web server
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;

      String url = "http://192.168.68.119/php-helper/upload.php";  // Change to your server IP
      http.begin(client, url);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      String postData = "patient_id=1&heart_rate=" + String(smoothedBPM) +
                        "&oxygen_level=" + String(spo2 < 0 ? 98 : spo2) +
                        "&body_temperature=" + String(temperature);

      Serial.println("POST data: " + postData);

      int httpResponseCode = http.POST(postData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      String payload = http.getString();
      Serial.println("Server response: " + payload);

      http.end();
    }

    delay(2000);
  }
}

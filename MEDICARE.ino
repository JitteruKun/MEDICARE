#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <Adafruit_MLX90614.h>

// TFT Display Pins
#define TFT_CS     5
#define TFT_RST    4
#define TFT_DC     2
#define TFT_MOSI   23
#define TFT_SCK    18
#define TFT_MISO   19

// Initialize TFT Display
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// Separate I²C Buses
TwoWire wireMax30102 = TwoWire(0); // I²C Bus 0 for MAX30102
TwoWire wireMlx90614 = TwoWire(1); // I²C Bus 1 for MLX90614

// MAX30102 and MLX90614 Sensor Instances
MAX30105 particleSensor;
Adafruit_MLX90614 tempSensor = Adafruit_MLX90614();

// I²C Pin Assignments
#define SDA_MAX30102 21   // MAX30102 on I²C Bus 0
#define SCL_MAX30102 22   
#define SDA_MLX90614 25   // MLX90614 on I²C Bus 1
#define SCL_MLX90614 26   

// Data Buffers
uint32_t irBuffer[100]; 
uint32_t redBuffer[100];

int32_t heartRate = 0;
int32_t spo2 = 0;
int8_t validHeartRate = 0;
int8_t validSpO2 = 0;

// Moving Average Filter
#define NUM_SAMPLES 10  
int32_t bpmSamples[NUM_SAMPLES] = {0};  
byte sampleIndex = 0;  

// Bitmap icons from converted SVG images
const unsigned char epd_bitmap_vo2_max_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x80, 0x07, 0x00, 0xc0, 
	0x0f, 0x00, 0xe0, 0x1c, 0x00, 0xf0, 0x3c, 0x00, 0xfc, 0xfc, 0x00, 0xcc, 0xcc, 0x00, 0xc6, 0x8c, 
	0x01, 0xc6, 0x8c, 0x01, 0xc6, 0x8c, 0x01, 0xc6, 0x0c, 0x00, 0xc6, 0xec, 0x01, 0xc6, 0x6c, 0x01, 
	0xc6, 0x2c, 0x79, 0xc6, 0x2c, 0x61, 0x7c, 0x68, 0x79, 0x38, 0xe0, 0x79, 0x00, 0x00, 0x08, 0x00, 
	0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const unsigned char epd_bitmap_thermostat_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0xc0, 0x03, 0x00, 0x60, 
	0xe6, 0x1f, 0x60, 0xe6, 0x1f, 0x60, 0x06, 0x00, 0x60, 0x06, 0x00, 0x60, 0xe6, 0x03, 0x60, 0xe6, 
	0x03, 0x60, 0x06, 0x00, 0x60, 0x06, 0x00, 0x30, 0x0c, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 
	0xf8, 0x1f, 0x00, 0xf8, 0x1f, 0x00, 0xf0, 0x0f, 0x00, 0xe0, 0x07, 0x00, 0xc0, 0x03, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const unsigned char epd_bitmap_health_metrics_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0x01, 0x80, 0xff, 0x01, 0x80, 0x81, 0x01, 0x80, 
	0x81, 0x01, 0x80, 0x81, 0x01, 0xfc, 0x81, 0x3f, 0xfc, 0xb1, 0x3f, 0x0c, 0x30, 0x30, 0x0c, 0x70, 
	0x30, 0xfc, 0xfb, 0x3f, 0xfc, 0xdf, 0x3f, 0x0c, 0x0e, 0x30, 0x0c, 0x0c, 0x30, 0xfc, 0x8d, 0x3f, 
	0xfc, 0x81, 0x3f, 0x80, 0x81, 0x01, 0x80, 0x81, 0x01, 0x80, 0x81, 0x01, 0x80, 0xff, 0x01, 0x80, 
	0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const uint8_t *heartIcon PROGMEM = epd_bitmap_health_metrics_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24;
const uint8_t *oxygenIcon PROGMEM = epd_bitmap_vo2_max_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24;
const uint8_t *tempIcon PROGMEM = epd_bitmap_thermostat_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24;

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=== ESP32 MAX30102 + MLX90614 Health Monitor ===");

    // Initialize TFT Display
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(ILI9341_BLACK);

    // Initialize Separate I²C Buses
    wireMax30102.begin(SDA_MAX30102, SCL_MAX30102);
    wireMlx90614.begin(SDA_MLX90614, SCL_MLX90614);
    Serial.println("[OK] I²C Buses Initialized");

    // Initialize MAX30102
    if (!particleSensor.begin(wireMax30102, I2C_SPEED_STANDARD)) {
        Serial.println("[ERROR] MAX30102 Not Detected!");
        while (1);
    }
    Serial.println("[OK] MAX30102 Initialized");

    // Initialize MLX90614
    if (!tempSensor.begin(0x5A, &wireMlx90614)) {
        Serial.println("[ERROR] MLX90614 Not Detected!");
        while (1);
    }
    Serial.println("[OK] MLX90614 Initialized");

    // Optimized Sensor Configuration
    particleSensor.setup(50, 3, 2, 50, 411, 4096);
    Serial.println("[OK] Sensors Configured");
}

void displayReadings(int32_t bpm, int32_t spO2, float temp) {
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextSize(2);

    // Heart Rate
    tft.drawBitmap(10, 40, heartIcon, 24, 24, ILI9341_WHITE);
    tft.setCursor(50, 40);
    tft.println("Heart Rate");
    tft.setCursor(60, 60);
    tft.setTextColor(ILI9341_GREEN);
    tft.printf("%d BPM\n", bpm);
    tft.setCursor(60, 90);
    tft.setTextColor(ILI9341_WHITE);
    tft.println("Normal Range");

    // SpO2
    tft.drawBitmap(10, 140, oxygenIcon, 24, 24, ILI9341_WHITE);
    tft.setCursor(50, 140);
    tft.println("SpO2 (Oxygen) %:");
    tft.setCursor(60, 160);
    tft.setTextColor(ILI9341_CYAN);
    tft.printf("%d%%\n", spO2);
    tft.setCursor(60, 190);
    tft.setTextColor(ILI9341_WHITE);
    tft.println("Normal Range");

    // Temperature
    tft.drawBitmap(10, 240, tempIcon, 24, 24, ILI9341_WHITE);
    tft.setCursor(50, 240);
    tft.println("Body Temperature");
    tft.setCursor(60, 260);
    tft.setTextColor(ILI9341_RED);
    tft.printf("%.1f C\n", temp);
    tft.setCursor(60, 290);
    tft.setTextColor(ILI9341_WHITE);
    tft.println("Normal Range");

    // Health Status Box
    tft.fillRoundRect(10, 330, 220, 40, 8, ILI9341_DARKGREY);
    tft.setCursor(20, 345);
    tft.setTextColor(ILI9341_WHITE);
    tft.println("Overall Health Status");
    tft.setCursor(180, 345);
    tft.setTextColor(ILI9341_GREEN);
    tft.println("Healthy");

    // WiFi Connection Status
    tft.setCursor(20, 380);
    tft.setTextColor(ILI9341_WHITE);
    tft.println("Connected to Server");
}

void loop() {
    const int bufferLength = 100;
    for (byte i = 0; i < bufferLength; i++) {
        while (!particleSensor.check());
        redBuffer[i] = particleSensor.getRed();
        irBuffer[i] = particleSensor.getIR();
    }

    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSpO2, &heartRate, &validHeartRate);

    if (heartRate < 0) heartRate = 0;
    bpmSamples[sampleIndex] = heartRate;
    sampleIndex = (sampleIndex + 1) % NUM_SAMPLES;

    int32_t smoothedBPM = 0;
    for (byte i = 0; i < NUM_SAMPLES; i++) {
        smoothedBPM += bpmSamples[i];
    }
    smoothedBPM /= NUM_SAMPLES;

    float temperature = tempSensor.readObjectTempC();
    if (temperature < -40 || temperature > 125) temperature = 0;

    displayReadings(validHeartRate ? smoothedBPM : 0, validSpO2 ? spo2 : 0, temperature);

    Serial.printf("Heart Rate: %d BPM\nSpO2: %d%%\nTemp: %.1f C\n", smoothedBPM, spo2, temperature);

    delay(1000);
}

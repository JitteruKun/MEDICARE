//Uncalibrated Sensors but effective
//LCD working already

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

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=== ESP32 MAX30102 + MLX90614 Health Monitor ===");

    // Initialize TFT Display
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.setCursor(30, 30);
    tft.println("Health Monitor");

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

void loop() {
    const int bufferLength = 100;
    for (byte i = 0; i < bufferLength; i++) {
        while (!particleSensor.check());  // Wait for new data
        redBuffer[i] = particleSensor.getRed();
        irBuffer[i] = particleSensor.getIR();
    }

    // Calculate Heart Rate & SpO2
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSpO2, &heartRate, &validHeartRate);

    if (heartRate < 0) heartRate = 0;

    // Apply Moving Average Filter
    bpmSamples[sampleIndex] = heartRate;
    sampleIndex = (sampleIndex + 1) % NUM_SAMPLES;

    int32_t smoothedBPM = 0;
    for (byte i = 0; i < NUM_SAMPLES; i++) {
        smoothedBPM += bpmSamples[i];
    }
    smoothedBPM /= NUM_SAMPLES;

    // Read Temperature from MLX90614
    float temperature = tempSensor.readObjectTempC();
    if (temperature < -40 || temperature > 125) temperature = 0;

    // Display Readings on TFT
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(30, 60);
    tft.setTextColor(ILI9341_GREEN);
    tft.print("Heart Rate: ");
    tft.print(validHeartRate ? smoothedBPM : 0);
    tft.println(" BPM");

    tft.setCursor(30, 90);
    tft.setTextColor(ILI9341_CYAN);
    tft.print("SpO2 Level: ");
    tft.print(validSpO2 ? spo2 : 0);
    tft.println("%");

    tft.setCursor(30, 120);
    tft.setTextColor(ILI9341_RED);
    tft.print("Temp: ");
    tft.print(temperature);
    tft.println(" C");

    // Display on Serial Monitor
    Serial.println("=================================");
    Serial.printf("Heart Rate: %s BPM\n", validHeartRate ? String(smoothedBPM).c_str() : "--");
    Serial.printf("SpO2 Level: %s%%\n", validSpO2 ? String(spo2).c_str() : "--");
    Serial.printf("Body Temperature: %.1f°C\n", temperature);
    Serial.println("=================================");

    delay(1000); // Refresh every second
}

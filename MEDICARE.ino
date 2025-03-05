//CALIBRATED MAX10302

#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

MAX30105 particleSensor;

// I²C Pin Assignments for ESP32-S2
#define SDA_PIN 21
#define SCL_PIN 22

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

    Serial.println("\n=== ESP32-S2 MAX30102 Heart Rate & SpO₂ Monitor ===");

    // Initialize I²C with ESP32-S2 SDA/SCL
    Wire.begin(SDA_PIN, SCL_PIN);
    Serial.println("[OK] I²C Initialized on SDA 21, SCL 22");

    // Initialize MAX30102
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
        Serial.println("[ERROR] MAX30102 Not Detected! Check wiring.");
        while (1);
    }
    Serial.println("[OK] MAX30102 Initialized");

    // Optimized Sensor Configuration
    particleSensor.setup(
        50,  // LED Brightness (Lowered for accuracy)
        3,   // Sample Average (Higher = Less Noise)
        2,   // LED Mode (2 = Red + IR)
        25,  // Sample Rate (Lowered to reduce motion artifacts)
        411, // Pulse Width (Increased for better accuracy)
        4096 // ADC Range (Standard for HR & SpO₂)
    );

    Serial.println("[OK] Sensor Configured");
}

void loop() {
    const int bufferLength = 100;  // FIXED: Declared bufferLength inside loop

    // Collect 100 samples
    for (byte i = 0; i < bufferLength; i++) {
        while (!particleSensor.check());  // Wait for new data
        redBuffer[i] = particleSensor.getRed();
        irBuffer[i] = particleSensor.getIR();
    }

    // Calculate Heart Rate & SpO₂
    maxim_heart_rate_and_oxygen_saturation(
        irBuffer, bufferLength, redBuffer, &spo2, &validSpO2, &heartRate, &validHeartRate);

    // Ensure BPM is never negative
    if (heartRate < 0) heartRate = 0;

    // Apply Moving Average Filter
    bpmSamples[sampleIndex] = heartRate;
    sampleIndex = (sampleIndex + 1) % NUM_SAMPLES;

    int32_t smoothedBPM = 0;
    for (byte i = 0; i < NUM_SAMPLES; i++) {
        smoothedBPM += bpmSamples[i];
    }
    smoothedBPM /= NUM_SAMPLES;

    Serial.println("=================================");
    Serial.printf("Heart Rate: %s BPM\n", validHeartRate ? String(smoothedBPM).c_str() : "--");
    Serial.printf("SpO2 Level: %s%%\n", validSpO2 ? String(spo2).c_str() : "--");
    Serial.println("=================================");

    delay(1000); // Refresh every 1 second
}

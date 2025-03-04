#include <Wire.h>
#include <TFT_eSPI.h>          
#include <Adafruit_MLX90614.h>
#include <MAX30105.h>
#include <heartRate.h>
#include <spo2_algorithm.h>

//  Define I²C Pins for ESP32-S2 (Single Bus for Both Sensors)
#define SDA_PIN 21
#define SCL_PIN 22

//  Initialize Peripherals
TFT_eSPI tft = TFT_eSPI();       // TFT Display
MAX30105 heartSensor;            // MAX30102 Heart Rate & SpO₂ Sensor
Adafruit_MLX90614 tempSensor = Adafruit_MLX90614();  // MLX90614 Temperature Sensor

void setup() {
    Serial.begin(115200);
    delay(1000); // Allow serial monitor to start

    Serial.println("\n=================================");
    Serial.println("      MEDICARE MONITOR");
    Serial.println("=================================");

    //  Initialize I²C Bus
    Wire.begin(SDA_PIN, SCL_PIN);
    Serial.println("[OK] I²C Initialized.");

    //  Initialize TFT Display
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    Serial.println("[OK] TFT Display Initialized.");

    //  Initialize MAX30102 (Heart Rate Sensor)
    if (!heartSensor.begin()) {  
        Serial.println("[ERROR] MAX30102 Not Detected!");
        tft.setCursor(10, 20);
        tft.println("MAX30102 ERROR!");
        while (1);
    }
    Serial.println("[OK] MAX30102 Initialized.");

    //  Initialize MLX90614 (Temperature Sensor)
    if (!tempSensor.begin()) {
        Serial.println("[ERROR] MLX90614 Not Detected!");
        tft.setCursor(10, 50);
        tft.println("MLX90614 ERROR!");
        while (1);
    }
    Serial.println("[OK] MLX90614 Initialized.");

    Serial.println("=================================");
}

void loop() {
    //  Read Heart Rate and SpO₂
    float irValue = heartSensor.getIR();
    float heartRate = (irValue > 50000) ? (irValue / 1000.0) : 0; // adjust the value. Recommended to lower it
    float spo2 = (irValue > 50000) ? (99 - (irValue / 20000.0)) : 0;

    //  Read Temperature from MLX90614
    float temperature = tempSensor.readObjectTempC();
    if (temperature < -40 || temperature > 125) temperature = 0;  

    //  Display Data on TFT
    updateDisplay(heartRate, spo2, temperature);

    //  Print Data to Serial Monitor
    Serial.printf("\nHeart Rate: %.1f BPM\n", heartRate);
    Serial.printf("SpO2 Level: %.1f %%\n", spo2);
    Serial.printf("Temperature: %.1f °C\n", temperature);
    Serial.println("=================================");

    delay(5000); // Wait 5 seconds before next reading
}

// Function to Update TFT Display
void updateDisplay(float heartRate, float spo2, float temperature) {
    tft.fillRect(10, 20, 220, 80, TFT_BLACK); // Clear only text area

    tft.setCursor(10, 20);
    tft.printf("HR: %.1f BPM", heartRate);

    tft.setCursor(10, 50);
    tft.printf("SpO2: %.1f%%", spo2);

    tft.setCursor(10, 80);
    tft.printf("Temp: %.1f C", temperature);
}

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <DHT.h>
#include <BH1750.h>
#include <FS.h>
#include <SPIFFS.h>
#include <math.h>
#include "index.h"

const char* ssid     = "TP Link C64";
const char* password = "Netgear@shihab1209";

// Static Host Routing Configuration
IPAddress local_IP(192, 168, 0, 150); 
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   

// HW Interfaces Configuration Mapping
#define DHTPIN       4
#define DHTTYPE      DHT22
#define MQ135_PIN    34
#define MAX9814_PIN  35
#define I2C_SDA      21
#define I2C_SCL      22
#define STATUS_LED   2     // Built-in blue pin on DevKit boards
#define BUZZER_PIN   12    // Output pin for the active buzzer

// DHT22 Error Offset Calibration Adjustment Vector
const float HUMIDITY_OFFSET = -8.0; // Shifting factor adjusting high room bias downward

// Logarithmic Conversion Factors (Air Quality Curve)
#define RL_BOARD     10.0  
#define RZERO_CLEAN  76.63 
#define CO2_A_FACTOR 116.602
#define CO2_B_FACTOR -2.769

DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;
AsyncWebServer server(80);

// Data Tracking Variables
float temperature = 0.0;
float humidity = 0.0;
float lux = 0.0;
int mq135_raw = 0;
float mq135_ppm = 0.0;
int sound_peak_to_peak = 0;

// Filter Matrices (Moving Window Configurations)
const int FILTER_SIZE = 10;
int mq135History[FILTER_SIZE] = {0};
int soundHistory[FILTER_SIZE] = {0};
int filterIndex = 0;

unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 2000; 

// Helper function to trigger a short alert beep pattern
void triggerBuzzerBeep(int count, int durationMs) {
    for (int i = 0; i < count; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(durationMs);
        digitalWrite(BUZZER_PIN, LOW);
        if (i < count - 1) delay(durationMs); 
    }
}

// Moving average calculation function
float processWindowFilter(int reading, int* historyBuffer) {
    historyBuffer[filterIndex] = reading;
    long calculationAccumulator = 0;
    for (int i = 0; i < FILTER_SIZE; i++) {
        calculationAccumulator += historyBuffer[i];
    }
    return (float)calculationAccumulator / FILTER_SIZE;
}

float calculateMQ135PPM(float smoothADC) {
    if (smoothADC <= 0) return 400.0; 
    float v_out = (smoothADC / 4095.0) * 3.3;
    if (v_out >= 3.3) v_out = 3.29; 
    float rs = ((3.3 - v_out) / v_out) * RL_BOARD;
    float ratio = rs / RZERO_CLEAN;
    float ppm = CO2_A_FACTOR * pow(ratio, CO2_B_FACTOR);
    if(ppm < 400.0) return 400.0;
    return ppm;
}

int getSoundLevel() {
    unsigned long startMillis = millis();
    int maxAmp = 0;
    int minAmp = 4095;

    while (millis() - startMillis < 50) {
        int sample = analogRead(MAX9814_PIN);
        if (sample < 4095) { 
            if (sample > maxAmp) maxAmp = sample;
            if (sample < minAmp) minAmp = sample;
        }
    }
    int rawPToP = maxAmp - minAmp;
    
    // Suppress small idle fluctuations below the 12-bit analog noise threshold
    if(rawPToP < 45) rawPToP = 0; 
    return (int)processWindowFilter(rawPToP, soundHistory);
}

void setup() {
    pinMode(STATUS_LED, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(STATUS_LED, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    Serial.begin(115200);
    
    // Boot alert confirmation
    triggerBuzzerBeep(2, 60);

    Wire.begin(I2C_SDA, I2C_SCL);
    dht.begin();
    lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
    analogReadResolution(12);

    if (!SPIFFS.begin(true)) Serial.println("SPIFFS error.");
    if (!SPIFFS.exists("/neuro_dataset.txt")) {
        File f = SPIFFS.open("/neuro_dataset.txt", FILE_WRITE);
        if(f) { f.println("Epoch_Seconds,Temperature_C,Humidity_Pct,Luminance_Lux,MQ135_Raw,AirQuality_PPM,Noise_PToP"); f.close(); }
    }

    WiFi.config(local_IP, gateway, subnet, primaryDNS);
    WiFi.begin(ssid, password);
    
    Serial.print("Connecting to network");
    // Fast flash pattern while connecting to Wi-Fi
    while (WiFi.status() != WL_CONNECTED) {
        digitalWrite(STATUS_LED, HIGH); delay(100);
        digitalWrite(STATUS_LED, LOW);  delay(100);
        Serial.print(".");
    }
    
    // Connection success signal
    digitalWrite(STATUS_LED, HIGH); // Solid blue indicates an active connection
    triggerBuzzerBeep(2, 90); 
    Serial.printf("\nSystem Hosted: http://%s\n", WiFi.localIP().toString().c_str());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ request->send(200, "text/html", index_html); });
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "{";
        json += "\"temperature\":" + String(temperature, 1) + ",";
        json += "\"humidity\":" + String(humidity, 1) + ",";
        json += "\"lux\":" + String(lux, 1) + ",";
        json += "\"mq135_raw\":" + String(mq135_raw) + ",";
        json += "\"mq135_ppm\":" + String(mq135_ppm, 1) + ",";
        json += "\"sound\":" + String(sound_peak_to_peak);
        json += "}";
        request->send(200, "application/json", json);
    });
    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){ request->send(SPIFFS, "/neuro_dataset.txt", "text/plain"); });
    server.on("/clear", HTTP_GET, [](AsyncWebServerRequest *request){
        SPIFFS.remove("/neuro_dataset.txt");
        File f = SPIFFS.open("/neuro_dataset.txt", FILE_WRITE);
        f.println("Epoch_Seconds,Temperature_C,Humidity_Pct,Luminance_Lux,MQ135_Raw,AirQuality_PPM,Noise_PToP"); f.close();
        request->send(200, "text/plain", "Cleared");
    });

    server.begin();
}

void loop() {
    if (millis() - lastSensorRead >= sensorInterval) {
        lastSensorRead = millis();

        float t = dht.readTemperature();
        float h = dht.readHumidity();
        
        // Error monitoring and validation block
        if (isnan(t) || isnan(h)) {
            Serial.println("[CRITICAL ERROR] DHT22 Unresponsive!");
            triggerBuzzerBeep(3, 40); // 3 rapid beeps flag a sensor read failure
        } else {
            temperature = t;
            humidity = h + HUMIDITY_OFFSET; // Apply calibration offset
            if (humidity > 100.0) humidity = 100.0;
            if (humidity < 0.0) humidity = 0.0;
        }

        lux = lightMeter.readLightLevel();
        
        int rawGas = analogRead(MQ135_PIN);
        float smoothGas = processWindowFilter(rawGas, mq135History);
        mq135_raw = (int)smoothGas;
        mq135_ppm = calculateMQ135PPM(smoothGas);
        
        sound_peak_to_peak = getSoundLevel();

        // Increment moving average window configuration indices
        filterIndex = (filterIndex + 1) % FILTER_SIZE;

        // Write out structured data row to SPIFFS log file
        File logFile = SPIFFS.open("/neuro_dataset.txt", FILE_APPEND);
        if(logFile) {
            logFile.printf("%lu,%.1f,%.1f,%.1f,%d,%.1f,%d\n", millis()/1000, temperature, humidity, lux, mq135_raw, mq135_ppm, sound_peak_to_peak);
            logFile.close();
        }

        Serial.printf("[SYSTEM LOG] T:%.1fC | H:%.1f%% | L:%.1flx | MQ_PPM:%.1f | Sound:%d\n", temperature, humidity, lux, mq135_ppm, sound_peak_to_peak);
    }
}
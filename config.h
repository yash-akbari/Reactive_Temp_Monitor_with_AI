#ifndef CONFIG_H
#define CONFIG_H

#include "mbed.h"

// --- User-Defined Temperature Thresholds ---
const float UPPER_THRESHOLD = 35.0f; // Celsius
const float LOWER_THRESHOLD = 15.0f; // Celsius

// --- System Configuration ---
const int SAMPLE_INTERVAL_MS = 5000; // Read sensors every 5 seconds
const int SAMPLES_PER_HOUR = (3600 * 1000) / SAMPLE_INTERVAL_MS;

// --- Pin Definitions ---
// I2C Pins for HTS221 & LPS22HB sensors
#define I2C_SDA PB_11
#define I2C_SCL PB_10
// Warning LED Pin
#define WARNING_LED LED2

// --- AI Anomaly Detection Configuration ---
const int RATE_BUFFER_SIZE = 60; // Learns from the last 60 samples (5 minutes)
const float ANOMALY_Z_SCORE_THRESHOLD = 3.0f; // 3-Sigma rule

// --- WiFi Credentials ---
#define WIFI_SSID "Your_WiFi_SSID"
#define WIFI_PASSWORD "Your_WiFi_Password"
#define WIFI_SECURITY NSAPI_SECURITY_WPA_WPA2 // Or NSAPI_SECURITY_NONE, etc.

// --- MQTT Broker Configuration ---
#define MQTT_BROKER_HOSTNAME "your_mqtt_broker.com" // e.g., "test.mosquitto.org"
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID "STM32L4-TempWarn" // Must be unique
#define MQTT_TOPIC_DATA "sensor/stm32l4/data"
#define MQTT_TOPIC_STATUS "sensor/stm32l4/status"
// Optional MQTT Authentication
#define MQTT_USERNAME "" // Leave empty if no auth
#define MQTT_PASSWORD "" // Leave empty if no auth

#endif // CONFIG_H
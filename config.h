#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
#include "mbed.h"

// Undefine default Arduino I2C pins to avoid redefinition warnings
#ifdef I2C_SDA
#undef I2C_SDA
#endif
#ifdef I2C_SCL
#undef I2C_SCL
#endif

#endif // __cplusplus


// --- User-Defined Temperature Thresholds ---
// All values are in degrees Celsius (°C)

// (HIGH) Threshold for triggering a "High Temperature" warning.
#define TEMP_THRESHOLD_HIGH 30.0f

// (CRITICAL) Threshold for triggering a "Critical Temperature" warning.
#define TEMP_THRESHOLD_CRITICAL 35.0f

// (LOW) Threshold for triggering a "Low Temperature" warning.
#define TEMP_THRESHOLD_LOW 5.0f

// Aliases for display and warnings modules
#define LOWER_THRESHOLD TEMP_THRESHOLD_LOW
#define UPPER_THRESHOLD TEMP_THRESHOLD_HIGH


// --- Pin Definitions ---
// I2C Pins for HTS221 & LPS22HB sensors
#define I2C_SDA PB_11
#define I2C_SCL PB_10

// Warning LED Pin (for temperature/anomaly alerts)
// Using PB_13 as default - adjust if using different pin on your board
#define WARNING_LED LED1

// --- Sensor Update Interval ---
// Defines how often (in milliseconds) the sensors are read.
#define SENSOR_UPDATE_INTERVAL_MS 2000
#define SAMPLE_INTERVAL_MS SENSOR_UPDATE_INTERVAL_MS

// --- Temperature Tracking ---
// Number of samples per hour (for rolling 1-hour statistics)
// At 2000ms intervals: 60 minutes * 60 seconds / 2 seconds = 1800 samples per hour
// Using a practical value: 1800 samples = exactly 1 hour at 2000ms intervals
#define SAMPLES_PER_HOUR 1800

// --- MQTT Configuration ---
#define MQTT_BROKER_HOSTNAME "192.168.29.45"
#define MQTT_BROKER_PORT 1883

// --- MQTT Topics ---
// Topic for publishing sensor data (temperature, humidity, pressure).
#define MQTT_TOPIC_DATA "iot-temp-monitor/data"

// Topic for publishing system status and alerts (e.g., "High Temp", "OK").
#define MQTT_TOPIC_STATUS "iot-temp-monitor/status"

// Topic for publishing AI-detected anomalies.
#define MQTT_TOPIC_ANOMALY "iot-temp-monitor/anomaly"

// --- Anomaly Detection ---
// The number of data points to use for the Simple Moving Average (SMA).
#define SMA_WINDOW_SIZE 10

// Buffer size for rate of change measurements (used in anomaly detector)
#define RATE_BUFFER_SIZE SMA_WINDOW_SIZE

// The standard deviation multiplier. A data point is an anomaly if it is
// 'STD_DEV_MULTIPLIER' standard deviations away from the SMA.
#define STD_DEV_MULTIPLIER 2.5f

// Z-score threshold for anomaly detection (typically 2.0 to 3.0 sigma)
#define ANOMALY_Z_SCORE_THRESHOLD STD_DEV_MULTIPLIER

// --- WiFi Configuration ---
// SSID and password for WiFi network
#define WIFI_SSID "Jio"
#define WIFI_PASSWORD "ram@123456"
// WiFi Security type: 0 = NSAPI_SECURITY_NONE, 1 = NSAPI_SECURITY_WEP,
// 2 = NSAPI_SECURITY_WPA, 3 = NSAPI_SECURITY_WPA2 (recommended)
#define WIFI_SECURITY NSAPI_SECURITY_WPA2

// --- MQTT Client Configuration ---
// Client ID for MQTT (unique identifier for this device)
#define MQTT_CLIENT_ID "temp_monitor_device_001"

// MQTT authentication credentials (leave empty strings for anonymous)
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""

#endif // CONFIG_H
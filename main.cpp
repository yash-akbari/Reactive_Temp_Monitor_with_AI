/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * This file is a solution for the Temperature Warning System coursework.
 * It includes all functional requirements and the AI Anomaly Detection task.
 *
 * Hardware Used:
 * - HTS221 (Temp/Humidity Sensor) on I2C
 * - LPS22HB (Pressure Sensor) on I2C
 * - LED2 (PB_14) as the "Warning Sound" (visual warning)
 * - Serial-over-USB (printf) as the "LCD Screen"
 */

#include "mbed.h"
#include "HTS221Sensor.h"
#include "LPS22HBSensor.h"
#include <cmath> // For sqrtf() and fabsf()
#include <limits> // For infinity()

// --- User-Defined Temperature Thresholds ---
// This section meets the "User-Defined" requirement.
// The user can set these values before compiling.
const float UPPER_THRESHOLD = 35.0f; // (in Celsius)
const float LOWER_THRESHOLD = 15.0f; // (in Celsius)
// -------------------------------------------

// --- System Configuration ---
const int SAMPLE_INTERVAL_MS = 1000; // Read sensors every 1 second
const int SAMPLES_PER_HOUR = (3600 * 1000) / SAMPLE_INTERVAL_MS;

// --- AI Anomaly Detection Configuration ---
const int RATE_BUFFER_SIZE = 60; // "Learns" from the last 60 samples (1 minute)
const float ANOMALY_Z_SCORE_THRESHOLD = 3.0f; // 3-Sigma rule for anomaly

// --- Hardware Initialization ---

// I2C1: Temperature, Humidity (HTS221) and Pressure (LPS22HB)
// Both sensors are on the same I2C bus (PB_11, PB_10)
static DevI2C devI2c(PB_11, PB_10);
static HTS221Sensor hts221_sensor(&devI2c);
static LPS22HBSensor lps22hb_sensor(&devI2c);

// Output: "Warning Sound"
// We use the User LED (LED2) as a visual warning, as no buzzer is on board.
// PwmOut allows us to create different flashing patterns.
static PwmOut warning_led(LED2);

// --- Global State Variables ---

// 1. Min/Max Tracking
float max_temp_1h = -std::numeric_limits<float>::infinity();
float min_temp_1h = std::numeric_limits<float>::infinity();
int sample_counter_1h = 0;

// 2. Anomaly Detection
float rate_buffer[RATE_BUFFER_SIZE] = {0.0f};
int buffer_index = 0;
float mean = 0.0f;
float std_dev = 0.0f;
float last_temp = 0.0f;
bool is_first_reading = true;


/**
 * @brief Resets the 1-hour min/max temperature tracking.
 * This is called once per hour.
 */
void reset_min_max() {
    max_temp_1h = -std::numeric_limits<float>::infinity();
    min_temp_1h = std::numeric_limits<float>::infinity();
    printf("--- HOURLY MIN/MAX RESET ---\n");
}

/**
 * @brief Updates the 1-hour min/max values with the current temperature.
 */
void update_1h_minmax(float temp) {
    if (temp > max_temp_1h) max_temp_1h = temp;
    if (temp < min_temp_1h) min_temp_1h = temp;
}

/**
 * @brief Adds a new rate-of-change value to the circular buffer.
 * This is a key efficient embedded structure.
 */
void update_rate_buffer(float new_rate) {
    rate_buffer[buffer_index] = new_rate;
    buffer_index = (buffer_index + 1) % RATE_BUFFER_SIZE;
}

/**
 * @brief "Trains" the AI model by recalculating statistics.
 * This finds the "normal" mean and standard deviation of the rate of change.
 */
void calculate_statistics() {
    // 1. Calculate Mean
    float sum = 0.0f;
    for (int i = 0; i < RATE_BUFFER_SIZE; i++) {
        sum += rate_buffer[i];
    }
    mean = sum / RATE_BUFFER_SIZE;

    // 2. Calculate Standard Deviation
    float sum_sq_diff = 0.0f;
    for (int i = 0; i < RATE_BUFFER_SIZE; i++) {
        sum_sq_diff += (rate_buffer[i] - mean) * (rate_buffer[i] - mean);
    }
    std_dev = sqrtf(sum_sq_diff / RATE_BUFFER_SIZE);
}

/**
 * @brief The core AI "Inference" function.
 * Calculates the rate of change, checks it against the "normal" model,
 * and returns 'true' if an anomaly is detected.
 */
bool process_anomaly_detection(float current_temp) {
    if (is_first_reading) {
        last_temp = current_temp; // Initialize
        return false; // Can't calculate a rate yet
    }

    // 1. Calculate the feature: "rate of change"
    float new_rate = current_temp - last_temp;
    last_temp = current_temp;

    bool is_anomalous = false;

    // 2. Perform "Inference" (Detect Anomaly)
    // Only check if model is "trained" (std_dev is not zero)
    if (std_dev > 0.001f) {
        float z_score = (new_rate - mean) / std_dev;

        // 3. The AI Decision!
        if (fabsf(z_score) > ANOMALY_Z_SCORE_THRESHOLD) {
            is_anomalous = true;
        }
    }

    // 4. "Re-Train" the model with the new data
    update_rate_buffer(new_rate);
    calculate_statistics();

    return is_anomalous;
}

/**
 * @brief Controls the warning LED based on system state.
 * This provides immediate feedback as required.
 */
void handle_warnings(float current_temp, bool is_anomalous) {
    if (is_anomalous) {
        // ANOMALY: Very fast flash (10Hz)
        warning_led.period_ms(100);
        warning_led.write(0.5f); // 50% duty cycle
    } else if (current_temp > UPPER_THRESHOLD) {
        // HIGH TEMP: Fast flash (2Hz)
        warning_led.period_ms(500);
        warning_led.write(0.5f);
    } else if (current_temp < LOWER_THRESHOLD) {
        // LOW TEMP: Slow flash (1Hz)
        warning_led.period_ms(1000);
        warning_led.write(0.5f);
    } else {
        // NORMAL: LED Off
        warning_led.write(0.0f); // 0% duty cycle
    }
}

/**
 * @brief Simulates the LCD screen output via the Serial Monitor.
 * Uses ANSI escape codes to clear the screen and refresh the display.
 */
void update_lcd_display(float current_temp, float pressure, bool is_anomalous) {
    // ANSI escape codes: Clear screen and move cursor to top-left
    printf("\033[2J\033[H");
    
    printf("--- Temperature Warning System ---\n");
    printf("\n");
    printf("Current Temp:  %.2f C\n", current_temp);
    printf("Max (Last 1h): %.2f C\n", max_temp_1h);
    printf("Min (Last 1h): %.2f C\n", min_temp_1h);
    printf("\n");
    printf("AI Status:     %s\n", is_anomalous ? "ANOMALY!" : "Normal");
    printf("\n");
    printf("----------------------------------\n");
    printf("Thresholds: %.1f C / %.1f C\n", LOWER_THRESHOLD, UPPER_THRESHOLD);
    printf("Bonus Info (Pressure): %.2f hPa\n", pressure);
    
    // Debug info for AI model
    // printf("AI Model: Mean=%.3f, SD=%.3f\n", mean, std_dev);
}

// --- Main Program ---
int main()
{
    printf("--- Mbed OS Temperature Warning System ---\n");

    // Initialize HTS221 (Temperature and Humidity)
    hts221_sensor.init(nullptr);
    hts221_sensor.enable();

    // Initialize LPS22HB (Pressure)
    lps22hb_sensor.init(nullptr);
    lps22hb_sensor.enable();

    // Initialize warning LED to OFF
    warning_led.period_ms(1000); // Set a default period
    warning_led.write(0.0f);     // Set duty cycle to 0 (off)

    printf("System configured with thresholds: %.1fC (Low) / %.1fC (High)\n", LOWER_THRESHOLD, UPPER_THRESHOLD);
    printf("AI Anomaly Detection is active.\n");
    printf("Starting sensor readings...\n\n");
    
    ThisThread::sleep_for(2000ms); // Give user time to read intro

    while (true) {
        // 1. Read Sensor Data
        float current_temp, humidity, pressure;
        hts221_sensor.get_temperature(&current_temp);
        hts221_sensor.get_humidity(&humidity); // Read humidity (optional)
        lps22hb_sensor.get_pressure(&pressure);

        // 2. Handle First-Run Initialization
        if (is_first_reading) {
            // Set initial min/max to the first reading
            max_temp_1h = current_temp;
            min_temp_1h = current_temp;
            is_first_reading = false;
        }

        // 3. Process Data
        update_1h_minmax(current_temp);
        bool is_anomalous = process_anomaly_detection(current_temp);

        // 4. Update Outputs
        handle_warnings(current_temp, is_anomalous);
        update_lcd_display(current_temp, pressure, is_anomalous);

        // 5. Handle 1-Hour Reset Logic
        sample_counter_1h++;
        if (sample_counter_1h >= SAMPLES_PER_HOUR) {
            reset_min_max();
            sample_counter_1h = 0;
            
            // Re-seed the new hour's min/max
            max_temp_1h = current_temp;
            min_temp_1h = current_temp;
        }

        // 6. Wait for next sample interval
        ThisThread::sleep_for(chrono::milliseconds(SAMPLE_INTERVAL_MS));
    }
}
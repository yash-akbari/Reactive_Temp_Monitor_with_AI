#include "display.h"
#include "config.h"

void display_update(const SensorData& current_data, const TempStats1Hour& stats, const AnomalyStatus& anomaly_status) {
    // ANSI escape codes: Clear screen and move cursor to top-left
    printf("\033[2J\033[H");

    printf("--- IoT Temperature Monitor ---\n\n");

    printf("Current Readings:\n");
    printf("  Temp:     %.2f C %s\n", current_data.temperature, current_data.temp_valid ? "" : "(Invalid)");
    printf("  Humidity: %.2f %% %s\n", current_data.humidity, current_data.humidity_valid ? "" : "(Invalid)");
    printf("  Pressure: %.2f hPa %s\n", current_data.pressure, current_data.pressure_valid ? "" : "(Invalid)");
    printf("\n");

    printf("Stats (Last Hour):\n");
    if (stats.valid) {
        printf("  Max Temp: %.2f C\n", stats.max_temp);
        printf("  Min Temp: %.2f C\n", stats.min_temp);
    } else {
        printf("  (Waiting for first hour to complete)\n");
    }
    printf("\n");

    printf("System Status:\n");
    printf("  AI Status: %s\n", anomaly_status.is_anomalous ? "🚨 ANOMALY DETECTED! 🚨" : "✅ Normal");
    // Optional: Display AI model stats for debugging
    // printf("  AI Model (Rate of Change): Mean=%.3f, SD=%.3f\n", anomaly_status.current_mean, anomaly_status.current_std_dev);
    printf("\n");

    printf("----------------------------------\n");
    printf("Thresholds: Low=%.1f C / High=%.1f C\n", LOWER_THRESHOLD, UPPER_THRESHOLD);
}
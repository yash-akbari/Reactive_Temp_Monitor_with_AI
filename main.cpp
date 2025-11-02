#include "mbed.h"
#include "config.h"
#include "sensors.h"
#include "anomaly_detector.h"
#include "temp_tracker.h"
#include "warnings.h"
#include "display.h"
#include "network_manager.h"
#include "mqtt_handler.h"

int main()
{
    printf("\n--- IoT Temperature Warning System Starting ---\n");

    // Initialize modules
    sensors_init();
    anomaly_detector_init();
    temp_tracker_init();
    warnings_init();

    // Initialize Network and MQTT
    NetworkInterface* net = nullptr;
    if (network_init() == NSAPI_ERROR_OK) {
        net = network_get_interface();
        if (!mqtt_init(net)) {
            printf("Error: Failed to initialize MQTT handler.\n");
            // Decide how to proceed - maybe run offline?
        } else {
            // Attempt initial MQTT connection (will retry in loop if fails)
            mqtt_connect();
            mqtt_publish_status("System Booted");
        }
    } else {
        printf("Error: Failed to initialize network. Running in offline mode.\n");
    }

    printf("\n--- Starting Main Loop ---\n");

    while (true) {
        // 1. Read Sensor Data
        SensorData current_sensor_data = sensors_read();

        // 2. Process Data
        temp_tracker_update(current_sensor_data.temperature);
        AnomalyStatus current_anomaly_status = anomaly_detector_process(current_sensor_data.temperature);
        TempStats1Hour current_stats = temp_tracker_get_stats();

        // 3. Update Local Outputs
        warnings_update(current_sensor_data.temperature, current_anomaly_status.is_anomalous);
        display_update(current_sensor_data, current_stats, current_anomaly_status);

        // 4. Handle Network & MQTT Tasks
        if (net) { // Only if network was initialized successfully
            if (!mqtt_is_connected()) {
                printf("MQTT disconnected. Attempting reconnect...\n");
                // Optional: Check WiFi status before attempting MQTT reconnect
                // if(net->get_connection_status() != NSAPI_STATUS_GLOBAL_UP) { ... }
                mqtt_connect(); // Attempt to reconnect
                if(mqtt_is_connected()) {
                    mqtt_publish_status("System Reconnected");
                }
            } else {
                // Publish data if connected
                mqtt_publish_data(current_sensor_data, current_stats, current_anomaly_status);

                // Allow MQTT client to process keep-alives, etc.
                mqtt_yield(100); // Allow 100ms for MQTT background tasks
            }
        }

        // 5. Wait for next sample interval
        // Adjust sleep time to account for MQTT yield time if necessary
        int sleep_time = SAMPLE_INTERVAL_MS;
        if (net && mqtt_is_connected()) {
           sleep_time = max(100, SAMPLE_INTERVAL_MS - 100); // Ensure at least 100ms sleep
        }
         ThisThread::sleep_for(chrono::milliseconds(sleep_time));
    }
}
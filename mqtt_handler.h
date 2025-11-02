#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include "NetworkInterface.h"
#include "sensors.h"
#include "temp_tracker.h"
#include "anomaly_detector.h"
#include <stdbool.h>

// Function prototypes
bool mqtt_init(NetworkInterface* network_interface);
bool mqtt_connect();
bool mqtt_publish_data(const SensorData& data, const TempStats1Hour& stats, const AnomalyStatus& anomaly);
bool mqtt_publish_status(const char* status_message);
bool mqtt_is_connected();
void mqtt_yield(int timeout_ms = 100); // Process MQTT messages
void mqtt_disconnect();

#endif // MQTT_HANDLER_H
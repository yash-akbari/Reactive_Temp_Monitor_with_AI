#ifndef SENSORS_H
#define SENSORS_H

// Sensor data structure
typedef struct {
    float temperature;
    float humidity;
    float pressure;
    bool temp_valid;
    bool humidity_valid;
    bool pressure_valid;
} SensorData;

// Function prototypes
void sensors_init();
SensorData sensors_read();

#endif // SENSORS_H
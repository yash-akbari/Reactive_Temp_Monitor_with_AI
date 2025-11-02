#include "sensors.h"
#include "config.h"
#include "HTS221Sensor.h"
#include "LPS22HBSensor.h"

// Sensor driver objects
static DevI2C devI2c(I2C_SDA, I2C_SCL);
static HTS221Sensor hts221_sensor(&devI2c);
static LPS22HBSensor lps22hb_sensor(&devI2c);

void sensors_init() {
    printf("Initializing Sensors...\n");
    // Initialize HTS221 (Temperature and Humidity)
    hts221_sensor.init(nullptr);
    hts221_sensor.enable();

    // Initialize LPS22HB (Pressure)
    lps22hb_sensor.init(nullptr);
    lps22hb_sensor.enable();

    uint8_t id;
    hts221_sensor.read_id(&id);
    printf("HTS221 ID: 0x%X\n", id);
    lps22hb_sensor.read_id(&id);
    printf("LPS22HB ID: 0x%X\n", id);
    printf("Sensors Initialized.\n");
}

SensorData sensors_read() {
    SensorData data = {0.0f, 0.0f, 0.0f, false, false, false};

    if (hts221_sensor.get_temperature(&data.temperature) == 0) {
        data.temp_valid = true;
    } else {
        printf("Error reading temperature!\n");
    }

    if (hts221_sensor.get_humidity(&data.humidity) == 0) {
        data.humidity_valid = true;
    } else {
        printf("Error reading humidity!\n");
    }

    if (lps22hb_sensor.get_pressure(&data.pressure) == 0) {
        data.pressure_valid = true;
    } else {
        printf("Error reading pressure!\n");
    }

    return data;
}
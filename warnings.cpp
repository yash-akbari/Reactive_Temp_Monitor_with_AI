#include "warnings.h"
#include "config.h"

// Warning LED object
static PwmOut warning_led(WARNING_LED);

void warnings_init() {
    warning_led.period_ms(1000); // Default period
    warning_led.write(0.0f);     // Start with LED off
    printf("Warning System Initialized.\n");
}

void warnings_update(float current_temp, bool is_anomalous) {
    if (is_anomalous) {
        // ANOMALY: Very fast flash (10Hz)
        warning_led.period_ms(100);
        warning_led.pulsewidth_ms(50); // 50% duty cycle
    } else if (current_temp > UPPER_THRESHOLD) {
        // HIGH TEMP: Fast flash (2Hz)
        warning_led.period_ms(500);
        warning_led.pulsewidth_ms(250);
    } else if (current_temp < LOWER_THRESHOLD) {
        // LOW TEMP: Slow flash (1Hz)
        warning_led.period_ms(1000);
        warning_led.pulsewidth_ms(500);
    } else {
        // NORMAL: LED Off
        warning_led.write(0.0f); // Set duty cycle to 0
    }
}
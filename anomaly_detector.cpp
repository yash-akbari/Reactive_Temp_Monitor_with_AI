#include "anomaly_detector.h"
#include "config.h"
#include <cmath> // For sqrtf() and fabsf()

// Internal state for anomaly detection
static float rate_buffer[RATE_BUFFER_SIZE] = {0.0f};
static int buffer_index = 0;
static float current_mean = 0.0f;
static float current_std_dev = 0.0f;
static float last_temp_reading = 0.0f;
static bool is_first_temp_reading = true;

// --- Helper Functions ---
static void update_rate_buffer(float new_rate) {
    rate_buffer[buffer_index] = new_rate;
    buffer_index = (buffer_index + 1) % RATE_BUFFER_SIZE;
}

static void calculate_statistics() {
    // 1. Calculate Mean
    float sum = 0.0f;
    for (int i = 0; i < RATE_BUFFER_SIZE; i++) {
        sum += rate_buffer[i];
    }
    current_mean = sum / RATE_BUFFER_SIZE;

    // 2. Calculate Standard Deviation
    float sum_sq_diff = 0.0f;
    for (int i = 0; i < RATE_BUFFER_SIZE; i++) {
        sum_sq_diff += (rate_buffer[i] - current_mean) * (rate_buffer[i] - current_mean);
    }
    // Use N instead of N-1 for population standard deviation on the rolling window
    current_std_dev = sqrtf(sum_sq_diff / RATE_BUFFER_SIZE);
}
// -----------------------

void anomaly_detector_init() {
    // Initialize buffer and state variables
    memset(rate_buffer, 0, sizeof(rate_buffer));
    buffer_index = 0;
    current_mean = 0.0f;
    current_std_dev = 0.0f;
    last_temp_reading = 0.0f;
    is_first_temp_reading = true;
    printf("Anomaly Detector Initialized.\n");
}

AnomalyStatus anomaly_detector_process(float current_temp) {
    AnomalyStatus status = {false, current_mean, current_std_dev};

    if (is_first_temp_reading) {
        last_temp_reading = current_temp; // Initialize
        is_first_temp_reading = false;
        return status; // Can't calculate a rate yet
    }

    // 1. Calculate the feature: "rate of change"
    float new_rate = current_temp - last_temp_reading;
    last_temp_reading = current_temp;

    // 2. Perform "Inference" (Detect Anomaly)
    // Only check if model is "trained" enough (std_dev is not near zero)
    if (current_std_dev > 0.001f) {
        float z_score = (new_rate - current_mean) / current_std_dev;

        // 3. The AI Decision!
        if (fabsf(z_score) > ANOMALY_Z_SCORE_THRESHOLD) {
            status.is_anomalous = true;
        }
    } else {
        // Model is still stabilizing, don't flag anomalies yet
        status.is_anomalous = false;
    }

    // 4. "Re-Train" the model with the new data
    update_rate_buffer(new_rate);
    calculate_statistics();

    // Update status with the latest stats
    status.current_mean = current_mean;
    status.current_std_dev = current_std_dev;

    return status;
}
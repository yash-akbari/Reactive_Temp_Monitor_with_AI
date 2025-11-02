#ifndef ANOMALY_DETECTOR_H
#define ANOMALY_DETECTOR_H

#include <stdbool.h>

typedef struct {
    bool is_anomalous;
    float current_mean;
    float current_std_dev;
} AnomalyStatus;

void anomaly_detector_init();
AnomalyStatus anomaly_detector_process(float current_temp);

#endif // ANOMALY_DETECTOR_H
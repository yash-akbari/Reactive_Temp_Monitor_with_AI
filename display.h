#ifndef DISPLAY_H
#define DISPLAY_H

#include "sensors.h"
#include "temp_tracker.h"
#include "anomaly_detector.h"
#include <stdbool.h>

void display_update(const SensorData& current_data, const TempStats1Hour& stats, const AnomalyStatus& anomaly_status);

#endif // DISPLAY_H
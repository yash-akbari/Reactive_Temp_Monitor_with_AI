#ifndef TEMP_TRACKER_H
#define TEMP_TRACKER_H

typedef struct {
    float min_temp;
    float max_temp;
    bool valid; // Becomes true after the first hour
} TempStats1Hour;

void temp_tracker_init();
void temp_tracker_update(float current_temp);
TempStats1Hour temp_tracker_get_stats();

#endif // TEMP_TRACKER_H
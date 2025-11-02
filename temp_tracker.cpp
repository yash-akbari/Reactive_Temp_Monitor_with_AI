#include "temp_tracker.h"
#include "config.h"
#include <limits> // For infinity()

static float max_temp_current_hour = -std::numeric_limits<float>::infinity();
static float min_temp_current_hour = std::numeric_limits<float>::infinity();
static int sample_count_current_hour = 0;
static bool stats_are_valid = false;
static bool first_reading_in_hour = true;

void temp_tracker_init() {
    max_temp_current_hour = -std::numeric_limits<float>::infinity();
    min_temp_current_hour = std::numeric_limits<float>::infinity();
    sample_count_current_hour = 0;
    stats_are_valid = false;
    first_reading_in_hour = true;
    printf("Temperature Tracker Initialized.\n");
}

void temp_tracker_update(float current_temp) {
    if (first_reading_in_hour) {
        // Seed the min/max with the first reading of the hour
        min_temp_current_hour = current_temp;
        max_temp_current_hour = current_temp;
        first_reading_in_hour = false;
    } else {
        // Update normally
        if (current_temp > max_temp_current_hour) max_temp_current_hour = current_temp;
        if (current_temp < min_temp_current_hour) min_temp_current_hour = current_temp;
    }

    sample_count_current_hour++;

    // Check if the hour has passed
    if (sample_count_current_hour >= SAMPLES_PER_HOUR) {
        printf("--- HOURLY MIN/MAX RESET ---\n");
        // Reset counters and stats for the new hour
        sample_count_current_hour = 0;
        stats_are_valid = true; // Stats from the *previous* full hour are now valid
        first_reading_in_hour = true; // Next reading will seed the new hour

        // Note: min/max are reset implicitly by the first_reading_in_hour flag logic
    }
}

TempStats1Hour temp_tracker_get_stats() {
    TempStats1Hour stats;
    stats.min_temp = min_temp_current_hour;
    stats.max_temp = max_temp_current_hour;
    stats.valid = stats_are_valid; // Report if we have completed at least one full hour
    return stats;
}
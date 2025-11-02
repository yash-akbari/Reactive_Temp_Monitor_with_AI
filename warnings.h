#ifndef WARNINGS_H
#define WARNINGS_H

#include <stdbool.h>

void warnings_init();
void warnings_update(float current_temp, bool is_anomalous);

#endif // WARNINGS_H
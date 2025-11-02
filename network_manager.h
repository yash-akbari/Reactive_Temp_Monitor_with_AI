#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "NetworkInterface.h"

// Function prototypes
nsapi_error_t network_init(); // Initializes and connects WiFi
NetworkInterface* network_get_interface(); // Returns the network interface pointer
void network_disconnect();

#endif // NETWORK_MANAGER_H
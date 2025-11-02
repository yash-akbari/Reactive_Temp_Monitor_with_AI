#include "network_manager.h"
#include "config.h"
#include "ISM43362Interface.h" // Include the specific WiFi driver header

// WiFi Interface object
// Note: MBED_CONF_ISM43362_WIFI_xxx pins are usually defined via mbed_app.json or target config
static ISM43362Interface wifi_interface(false); // Pass 'true' to enable driver debug messages

nsapi_error_t network_init() {
    printf("Initializing Network...\n");
    printf("Connecting to WiFi network: %s\n", WIFI_SSID);

    // Set WiFi credentials
    nsapi_error_t result = wifi_interface.set_credentials(WIFI_SSID, WIFI_PASSWORD, WIFI_SECURITY);
    if (result != NSAPI_ERROR_OK) {
        printf("Error: set_credentials failed! (%d)\n", result);
        return result;
    }

    // Attempt to connect
    result = wifi_interface.connect();
    if (result != NSAPI_ERROR_OK) {
        printf("Error: connect failed! (%d)\n", result);
        return result;
    }

    // Print connection details
    SocketAddress addr;
    wifi_interface.get_ip_address(&addr);
    printf("WiFi Connected. IP Address: %s\n", addr.get_ip_address());

    return NSAPI_ERROR_OK;
}

NetworkInterface* network_get_interface() {
    return &wifi_interface;
}

void network_disconnect() {
    printf("Disconnecting WiFi...\n");
    wifi_interface.disconnect();
    printf("WiFi Disconnected.\n");
}
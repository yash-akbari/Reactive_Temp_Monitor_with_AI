#include "network_manager.h"
#include "config.h"
#include "ISM43362Interface.h"

// WiFi Interface object
static ISM43362Interface wifi_interface(false);

nsapi_error_t network_init() {
    printf("\n=== WiFi Network Initialization ===\n");
    printf("SSID: %s\n", WIFI_SSID);
    const char* security_type = "Unknown";
    if (WIFI_SECURITY == NSAPI_SECURITY_NONE) {
        security_type = "Open (No Security)";
    } else if (WIFI_SECURITY == NSAPI_SECURITY_WEP) {
        security_type = "WEP";
    } else if (WIFI_SECURITY == NSAPI_SECURITY_WPA) {
        security_type = "WPA";
    } else if (WIFI_SECURITY == NSAPI_SECURITY_WPA2) {
        security_type = "WPA2-Personal";
    }
    printf("Security: %s\n", security_type);

    // Set WiFi credentials
    printf("Setting WiFi credentials...\n");
    nsapi_error_t result = wifi_interface.set_credentials(WIFI_SSID, WIFI_PASSWORD, WIFI_SECURITY);
    if (result != NSAPI_ERROR_OK) {
        printf("ERROR: set_credentials failed with code: %d\n", result);
        printf("  This usually means the WiFi SSID/password format is invalid\n");
        wait_us(100000);
        return result;
    }
    printf("Credentials set successfully.\n");

    // Attempt to connect with retry
    printf("Attempting to connect to WiFi (timeout ~10-15 seconds)...\n");
    result = wifi_interface.connect();
    if (result != NSAPI_ERROR_OK) {
        printf("\nERROR: WiFi connection failed with code: %d\n", result);
        switch(result) {
            case NSAPI_ERROR_AUTH_FAILURE:
                printf("  Code: Authentication failure - check password\n");
                printf("  Possible causes:\n");
                printf("    1. Password is incorrect\n");
                printf("    2. WiFi network security type mismatch (expected WPA2)\n");
                break;
            case NSAPI_ERROR_NO_SSID:
                printf("  Code: SSID not found - check network name\n");
                printf("  Possible causes:\n");
                printf("    1. WiFi SSID '%s' not found in range\n", WIFI_SSID);
                break;
            case NSAPI_ERROR_TIMEOUT:
                printf("  Code: Connection timeout - network may be busy\n");
                break;
            default:
                printf("  See Mbed OS documentation for error code %d\n", result);
        }
        wait_us(100000);
        return result;
    }

    // Print connection details
    printf("WiFi connection successful!\n");
    const char *ip = wifi_interface.get_ip_address();
    if (ip) {
        printf("IP Address: %s\n", ip);
    } else {
        printf("IP Address: (DHCP in progress)\n");
    }
    printf("=== WiFi Ready ===\n\n");
    wait_us(100000);
    return NSAPI_ERROR_OK;
}

NetworkInterface* network_get_interface() {
    return &wifi_interface;
}

void network_disconnect() {
    printf("Disconnecting WiFi...\n");
    wifi_interface.disconnect();
    wait_us(100000);
    printf("WiFi Disconnected.\n");
}
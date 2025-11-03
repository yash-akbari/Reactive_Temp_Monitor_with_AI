#include "mqtt_handler.h"
#include "config.h"
#include "MQTTClientMbedOs.h" // Include the MQTT library header
#include "TCPSocket.h"
#include "SocketAddress.h"

static NetworkInterface* _network_interface = nullptr;
static MQTTClient* _mqtt_client = nullptr;
static TCPSocket* _mqtt_socket = nullptr;
static bool _is_connected = false;

// Buffer for MQTT messages
static char mqtt_payload_buffer[256];

bool mqtt_init(NetworkInterface* network_interface) {
    if (!network_interface) {
        printf("MQTT Error: Network interface is null!\n");
        return false;
    }
    _network_interface = network_interface;

    // Allocate TCPSocket
    _mqtt_socket = new TCPSocket();
    if (!_mqtt_socket) {
        printf("MQTT Error: Failed to allocate TCPSocket!\n");
        return false;
    }

    // Open socket with network interface
    nsapi_error_t sock_result = _mqtt_socket->open(_network_interface);
    if (sock_result != NSAPI_ERROR_OK) {
        printf("MQTT Error: Failed to open socket! (%d)\n", sock_result);
        delete _mqtt_socket;
        _mqtt_socket = nullptr;
        return false;
    }

    // Allocate MQTT client with the socket
    _mqtt_client = new MQTTClient(_mqtt_socket);
    if (!_mqtt_client) {
        printf("MQTT Error: Failed to allocate MQTTClient!\n");
        _mqtt_socket->close();
        delete _mqtt_socket;
        _mqtt_socket = nullptr;
        return false;
    }

    printf("MQTT Handler Initialized.\n");
    return true;
}

bool mqtt_connect() {
    if (!_mqtt_client || !_mqtt_socket) {
        printf("MQTT Error: Not initialized!\n");
        return false;
    }

    if (_is_connected) {
        return true; // Already connected
    }

    printf("Connecting to MQTT broker: %s:%d\n", MQTT_BROKER_HOSTNAME, MQTT_BROKER_PORT);

    // Resolve hostname and create socket address
    SocketAddress broker_addr;
    nsapi_error_t dns_result = _network_interface->gethostbyname(MQTT_BROKER_HOSTNAME, &broker_addr);
    if (dns_result != NSAPI_ERROR_OK) {
        printf("MQTT Error: DNS lookup failed for broker (%d)\n", dns_result);
        _is_connected = false;
        return false;
    }
    broker_addr.set_port(MQTT_BROKER_PORT);

    // Connect the socket with the resolved address
    nsapi_error_t socket_result = _mqtt_socket->connect(broker_addr);
    if (socket_result != NSAPI_ERROR_OK) {
        printf("MQTT Error: Socket connection failed! (%d)\n", socket_result);
        _is_connected = false;
        return false;
    }

    // Set up MQTT connection options
    MQTTPacket_connectData options = MQTTPacket_connectData_initializer;
    options.MQTTVersion = 4; // Use MQTT 3.1.1
    options.clientID.cstring = (char*)MQTT_CLIENT_ID;

    // Add authentication if configured
    if (strlen(MQTT_USERNAME) > 0) {
        options.username.cstring = (char*)MQTT_USERNAME;
    }
    if (strlen(MQTT_PASSWORD) > 0) {
        options.password.cstring = (char*)MQTT_PASSWORD;
    }
    options.keepAliveInterval = 60; // Keep alive interval in seconds
    options.cleansession = 1;

    // Attempt MQTT connection (this returns nsapi_error_t)
    nsapi_error_t mqtt_result = _mqtt_client->connect(options);
    if (mqtt_result != NSAPI_ERROR_OK) {
        printf("MQTT Error: MQTT Connection failed! (%d)\n", mqtt_result);
        _is_connected = false;
        _mqtt_socket->close();
        return false;
    }

    printf("MQTT Connected Successfully!\n");
    _is_connected = true;
    return true;
}

bool mqtt_publish_data(const SensorData& data, const TempStats1Hour& stats, const AnomalyStatus& anomaly) {
    if (!_is_connected || !_mqtt_client) {
        printf("MQTT Error: Not connected, cannot publish data.\n");
        return false;
    }

    // Format data into JSON payload
    // Note: Using snprintf for safety against buffer overflows
    int len = snprintf(mqtt_payload_buffer, sizeof(mqtt_payload_buffer),
                       "{\"temp\":%.2f, \"humidity\":%.2f, \"pressure\":%.2f, "
                       "\"min_1h\":%.2f, \"max_1h\":%.2f, \"anomaly\":\"%s\"}",
                       data.temperature, data.humidity, data.pressure,
                       stats.min_temp, stats.max_temp,
                       anomaly.is_anomalous ? "true" : "false");

    if (len < 0 || len >= (int)sizeof(mqtt_payload_buffer)) {
        printf("MQTT Error: Payload buffer too small or snprintf error!\n");
        return false;
    }

    // Create MQTT message
    MQTT::Message message;
    message.qos = MQTT::QOS0; // Quality of Service 0 (at most once)
    message.retained = false;
    message.dup = false;
    message.payload = (void*)mqtt_payload_buffer;
    message.payloadlen = len;

    // Publish the message (returns nsapi_error_t)
    nsapi_error_t rc = _mqtt_client->publish(MQTT_TOPIC_DATA, message);
    if (rc != NSAPI_ERROR_OK) {
        printf("MQTT Error: Failed to publish data! (Code %d)\n", rc);
        // If socket error, mark as disconnected
        if (rc == NSAPI_ERROR_DEVICE_ERROR || rc == NSAPI_ERROR_CONNECTION_LOST) {
            _is_connected = false;
        }
        return false;
    }

    // printf("MQTT: Published data to %s\n", MQTT_TOPIC_DATA); // Optional debug print
    return true;
}

bool mqtt_publish_status(const char* status_message) {
    if (!_is_connected || !_mqtt_client) {
        printf("MQTT Error: Not connected, cannot publish status.\n");
        return false;
    }

    // Format data into simple JSON payload
    int len = snprintf(mqtt_payload_buffer, sizeof(mqtt_payload_buffer),
                       "{\"status\":\"%s\"}", status_message);

    if (len < 0 || len >= (int)sizeof(mqtt_payload_buffer)) {
        printf("MQTT Error: Payload buffer too small or snprintf error for status!\n");
        return false;
    }

    // Create MQTT message
    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = true; // Retain the last status message on the broker
    message.dup = false;
    message.payload = (void*)mqtt_payload_buffer;
    message.payloadlen = len;

    // Publish the message (returns nsapi_error_t)
    nsapi_error_t rc = _mqtt_client->publish(MQTT_TOPIC_STATUS, message);
    if (rc != NSAPI_ERROR_OK) {
        printf("MQTT Error: Failed to publish status! (Code %d)\n", rc);
        if (rc == NSAPI_ERROR_DEVICE_ERROR || rc == NSAPI_ERROR_CONNECTION_LOST) {
            _is_connected = false;
        }
        return false;
    }

    printf("MQTT: Published status '%s' to %s\n", status_message, MQTT_TOPIC_STATUS);
    return true;
}


bool mqtt_is_connected() {
    // Check internal flag first
    if (!_is_connected || !_mqtt_client) return false;

    // Optionally add a check using the client's isConnected method,
    // but be mindful this might involve network I/O.
    // The yield function updates the internal state more reliably.
    return _mqtt_client->isConnected();
}

void mqtt_yield(int timeout_ms) {
    if (_mqtt_client && _network_interface) {
        // Yield allows the MQTT client to process incoming messages (like PINGRESP)
        // and manage keep-alive packets.
        nsapi_error_t rc = _mqtt_client->yield(timeout_ms);
        if (rc != NSAPI_ERROR_OK && rc != NSAPI_ERROR_WOULD_BLOCK) {
            printf("MQTT Warning: Yield returned error %d\n", rc);
            // If yield indicates disconnection, update our state
            if (!_mqtt_client->isConnected()) {
                printf("MQTT Disconnected during yield.\n");
                _is_connected = false;
            }
        }
    }
}

void mqtt_disconnect() {
    if (_is_connected && _mqtt_client) {
        printf("Disconnecting MQTT...\n");
        nsapi_error_t rc = _mqtt_client->disconnect();
        if (rc != NSAPI_ERROR_OK) {
            printf("MQTT Warning: Disconnect failed! (Code %d)\n", rc);
        } else {
            printf("MQTT Disconnected.\n");
        }
    }
    _is_connected = false;

    // Close the socket if open
    if (_mqtt_socket) {
        _mqtt_socket->close();
    }

    // Cleanup resources - consider if you want to reuse the client/socket later
    // If not reusing, uncomment the delete lines:
    // if (_mqtt_client) {
    //    delete _mqtt_client;
    //    _mqtt_client = nullptr;
    // }
    // if (_mqtt_socket) {
    //    delete _mqtt_socket;
    //    _mqtt_socket = nullptr;
    // }
    // _network_interface = nullptr; // Clear the network interface pointer
}
#include "inc/mqtt.h"
#include "inc/wifi.h"
#include "inc/display.h"
#include "inc/config.h"
#include "inc/timertc.h"
#include "inc/flash.h"
#include "lwip/dns.h"

// Structure to store the MQTT client information
const struct mqtt_connect_client_info_t client_info = {
    .client_id = "Sensor1", // Client ID
    .client_user = NULL,        // User name
    .client_pass = NULL,        // Password
    .keep_alive = 60,           // Keep alive time
};

// MQTT Client
mqtt_client_t *global_mqtt_client = NULL;
ip_addr_t broker_ip; // Global variable to store the broker IP address

static void dns_function_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg){

    LWIP_UNUSED_ARG(name);
    LWIP_UNUSED_ARG(callback_arg);

    if (ipaddr != NULL)
    {
        printf("DNS resolvido: %s\n", ipaddr_ntoa(ipaddr));
        ip_addr_copy(broker_ip, *ipaddr); // Copy the resolved IP address to the global variable

        mqtt_client_connect(global_mqtt_client, &broker_ip, MQTT_PORT, mqtt_connection_cb, NULL, &client_info); // Connect to the broker
    }
    else
    {
        printf("Falha ao resolver DNS (callback retornou NULL).\n");
    }
    
}

void resolve_broker_dns(ip_addr_t *broker_ip) {

    ip_addr_t dns_server_addr;

    ipaddr_aton("8.8.8.8", &dns_server_addr);
    dns_setserver(0, &dns_server_addr);

    err_t err = dns_gethostbyname(MQTT_BROKER, broker_ip, dns_function_callback, NULL);
}

/**
 * @brief Callback function for MQTT connection events.
 *
 * @param client MQTT client instance.
 * @param arg User-defined argument (unused in this case).
 * @param status Connection status.
 *
 * This function is called when an MQTT connection attempt is completed.
 * It prints whether the connection was successful or failed.
 *
 * It also tries to reconnect if the connection failed.
 *
 */

// Callback function for MQTT connection events
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{

    // Check if the connection was successful
    if (status == MQTT_CONNECT_ACCEPTED)
    {
        printf("Conexão MQTT bem-sucedida!\n"); // Debug message

        resend_saved_data(); // Resend any saved data from flash storage
    }
    else
    {
        printf("Falha na conexão MQTT: %d\n", status); // Debug message
    }
}

/**
 * @brief Initializes and starts the MQTT client.
 *
 * This function creates a new MQTT client, assigns a broker IP,
 * and attempts to establish a connection.
 *
 */

void start_mqtt_client(void)
{

    global_mqtt_client = mqtt_client_new(); // Create a new MQTT client

    // Check if the client was created successfully
    if (!global_mqtt_client)
    {
        printf("Falha ao criar cliente MQTT\n"); // Debug message
        return;
    }

    resolve_broker_dns(&broker_ip); // Resolve the broker DNS to get the IP address

}

/**
 * @brief Publishes sound level data to an MQTT topic.
 *
 * @param micdata Pointer to a micdata_t structure containing the sound level data and other metadata.
 *
 * This function checks if the MQTT client/Wi-Fi is connected, formats the information into a JSON payload and
 * attempts to publish it to the MQTT topic.
 *
 * If the data is published successfully, it displays a debug message.
 * If there is an error, it displays an error message.
 *
 */

// Função de publicação MODIFICADA
void publish_db_to_mqtt(micdata_t *micdata) {

    char payload[256];
    char timestamp[32];
    datetime_t now;

    rtc_get_datetime(&now);

    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d",
             now.year, now.month, now.day, now.hour-=GMT_M_3, now.min, now.sec);

    // >>> CRIA O PAYLOAD ANTES DE VERIFICAR A CONEXÃO <<<
    snprintf(payload, sizeof(payload),
             "{\"id\":\"%d\", \"avgdB\":\"%.2f\", \"mindB\": \"%.2f\", \"maxdB\": \"%.2f\", \"latitude\":%.6f, \"longitude\":%.6f, \"timestamp\":\"%s\"}",
             micdata->sensor_id, micdata->average, micdata->mindB, micdata->maxdB, micdata->latitude, micdata->longitude, timestamp);

    // Verifica se está conectado
    if (global_mqtt_client && mqtt_client_is_connected(global_mqtt_client) && is_wifi_connected()) {
        err_t err = mqtt_publish(global_mqtt_client, MQTT_TOPIC, payload, strlen(payload), 1, 0, NULL, NULL); // QoS 1 para maior garantia

        if (err == ERR_OK) {
            printf("Dados enviados via MQTT: %s\n", payload);
        } else {
            printf("Erro ao publicar via MQTT: %d. Salvando em flash.\n", err);
            save_payload_to_flash(payload); // Salva se a publicação falhar
        }
    } else {
        // >>> SE ESTIVER OFFLINE, CHAMA A FUNÇÃO PARA SALVAR <<<
        save_payload_to_flash(payload);
    }
}

/**
 * @brief Checks the MQTT connection status and displays it on the OLED display.
 *
 * This function checks if the MQTT client is connected and if Wi-Fi is connected.
 * If the connection status has changed, it displays the new status on the OLED display.
 * 
 * If the MQTT client is not connected but Wi-Fi is connected, it attempts to reconnect to the MQTT broker.
 *
 */

void check_mqtt_connection()
{
    // Check if the MQTT client is created/connected, and check if Wi-Fi is connected
    bool is_connected = (global_mqtt_client && mqtt_client_is_connected(global_mqtt_client) && is_wifi_connected());

    // Last MQTT connection status
    static bool last_mqtt_status = false;
    static bool first_run = true;

    if (first_run || is_connected != last_mqtt_status)
    {
        ssd1306_clear_area(&disp, 0, 30, 128, 32);

        if (is_connected)
        {
            ssd1306_draw_string(&disp, 0, 30, 1, "MQTT: Connected");
        }
        else
        {
            ssd1306_draw_string(&disp, 0, 30, 1, "Local Running");
            ssd1306_draw_string(&disp, 0, 45, 1, "No MQTT Connection");
        }
        last_mqtt_status = is_connected;
        first_run = false;
    }

    if(!is_connected && is_wifi_connected())
    {
        // Try to reconnect if the MQTT client is not connected but Wi-Fi is connected
        mqtt_client_connect(global_mqtt_client, &broker_ip, MQTT_PORT, mqtt_connection_cb, NULL, &client_info);
    }
}
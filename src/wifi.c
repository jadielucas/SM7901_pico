#include <stdio.h>
#include "inc/wifi.h"
#include "inc/display.h"
#include "inc/mqtt.h"

volatile bool toggle_wifi = false;
bool wifi_reconnecting = true;

/**
 * @brief Initializes the Wi-Fi connection.
 *
 * This function initializes the Wi-Fi module and attempts to connect to a Wi-Fi network.
 *
 * If initialization or connection fails, it prints an error message and returns 1.
 * If the connection is successful, it prints a confirmation message.
 * It also clears "SilentPico" logo from the display.
 *
 * @return int Returns 0 on success, 1 on failure.
 */

int wifi_init()
{
    // Initialize the Wi-Fi module
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_BRAZIL))
    {
        printf("Erro ao inicializar o Wi-Fi\n"); // Debug message
        return 1;
    }

    cyw43_arch_enable_sta_mode();       // Enable the station mode
    printf("Conectando ao Wi-Fi...\n"); // Debug message

    // Connect to the Wi-Fi network
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 8000))
    {
        ssd1306_clear(&disp); // Clear "SilentPico" logo from the display

        printf("Falha ao conectar ao Wi-Fi\n"); // Debug message

        return 1;
    }
    else
    {
        ssd1306_clear(&disp); // Clear "SilentPico" logo from the display

        printf("Wi-Fi conectado!\n"); // Debug message

        return 0;
    }
}

/**
 * @brief Checks if the Wi-Fi connection is active.
 *
 * This function checks if the Wi-Fi connection is active by calling the cyw43_wifi_link_status function.
 *
 * If the connection is active, it clears the display and returns true.
 * If the connection is not active, it returns false.
 * It also resets the reconnection attempts counter.
 *
 * @return bool Returns true if the Wi-Fi connection is active, false otherwise.
 */

bool is_wifi_connected()
{
    int status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA); // Get the Wi-Fi connection status

    // Check if the connection is active
    if (status == CYW43_LINK_JOIN || status == CYW43_LINK_UP)
    {
        return true; // Return true if the connection is active
    }
    else
    {
        return false; // Return false if the connection is not active
    }
}

/**
 * @brief Checks the Wi-Fi connection status and attempts to reconnect if necessary.
 *
 * This function checks the Wi-Fi connection status by calling the is_wifi_connected function.
 *
 * If the connection is not active, it attempts to reconnect to the Wi-Fi network.
 * If the reconnection fails after 6 attempts, it prints an error message and returns.
 */

void check_wifi_connection()
{
    static uint64_t last_attempt_time = 0;
    uint64_t current_time = time_us_64();

    if (toggle_wifi == true && !is_wifi_connected())
    {

        printf("Wi-Fi desconectado. Tentando reconectar...\n"); // Debug message

        // Try reconnecting to the Wi-Fi network
        if (cyw43_arch_wifi_connect_async(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK) == 0)
        {
            printf("Reconectando...\n"); // Debug message

            toggle_wifi = false;

            wifi_reconnecting = true;
        }
        else
        {
            toggle_wifi = false;

            printf("Erro ao iniciar reconexão\n"); // Debug message
        }
    }

    else if (toggle_wifi == true && is_wifi_connected())
    {
        cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA); // Disconnect from the Wi-Fi network

        toggle_wifi = false;

        wifi_reconnecting = false;
    }

    else if (toggle_wifi == false && !is_wifi_connected() && wifi_reconnecting == true)
    {

        if (last_attempt_time == 0 || current_time - last_attempt_time >= 60000000)
        {
            printf("Wi-Fi desconectado. Tentando reconectar...\n"); // Debug message

            // Try reconnecting to the Wi-Fi network
            if (cyw43_arch_wifi_connect_async(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK) == 0)
            {
                printf("Reconectando...\n"); // Debug message
            }
            else
            {
                printf("Erro ao iniciar reconexão\n"); // Debug message
            }

            last_attempt_time = current_time;
        }
    }
    else
    {
        return;
    }
}
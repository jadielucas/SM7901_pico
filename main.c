#include "inc/mic.h"                   // Library for microphone data collection
#include "inc/display.h"               // Library for display manipulation
#include "inc/wifi.h"                  // Library for Wi-Fi connection
#include "inc/mqtt.h"                  // Library for MQTT protocol communication
#include "inc/buttons.h"               // Library for button control
#include "inc/timertc.h"               // Library for timer and RTC (Real-Time Clock) management
#include "inc/flash.h"                 // Library for flash memory operations
#include "pico/stdlib.h"              // Standard library for Raspberry Pi Pico

//Main function of the program
int main()
{
    stdio_init_all();                // Initialize standard serial communication
    init_filesystem();               // Initialize the filesystem for data storage
    setup_display();                 // Initialize the OLED display
    setup_buttons();                 // Configure buttons for user input 
    uart_modbus_config();            // Configure UART for Modbus communication
    wifi_init();                     // Initialize the Wi-Fi module
    start_mqtt_client();             // Start the MQTT client for remote communication
    init_and_sync_rtc();             // Configure the date and time settings

    uint8_t device_address = 0x01;      // SM7901 Microphone Modbus address
    uint16_t start_address = 0x0000;    // Start address for Modbus registers
    uint16_t num_registers = 1;         // Number of registers to read
    uint8_t response[7];                // Buffer to store Modbus response
    micdata.sensor_id = SENSOR_ID;      // Set the sensor ID for the microphone
    micdata.latitude = MAP_LATITUDE;    // Set the latitude of the microphone
    micdata.longitude = MAP_LONGITUDE;  // Set the longitude of the microphone

    // Main loop of the program
    while (true) {

        modbus_read_registers(device_address, start_address, num_registers); // Read Modbus registers from the microphone

        parse_decibel_value(response);                         // Parse the response to get the decibel value

        get_media_min_max_dB(&micdata);                        // Calculate the average dB value

        update_display_db_value(&micdata);                     // Update dB value on the display

        check_wifi_connection();                               // Check the Wi-Fi connection status

        check_mqtt_connection();                               // Check the MQTT connection status

        sleep_ms(300);                 // Wait 300ms before the next iteration

    }

    return 0; // Only for compilation purposes
}
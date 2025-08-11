#include "inc/mic.h"                   // Library for microphone data collection
#include "inc/display.h"               // Library for display manipulation
#include "inc/wifi.h"                  // Library for Wi-Fi connection
#include "inc/mqtt.h"                  // Library for MQTT protocol communication
#include "inc/timertc.h"               // Library for timer and RTC (Real-Time Clock) management
#include "inc/flash.h"                 // Library for flash memory operations
#include "pico/multicore.h"            // Library for multi-core operations on Raspberry Pi Pico

micdata_t micdata;                // Global variable to hold microphone data

void core1_entry(){

    while(true){

        check_wifi_connection();                                                                        // Check the Wi-Fi connection status
        check_mqtt_connection();                                                                        // Check the MQTT connection status

        modbus_read_registers(micdata.device_address, micdata.start_address, micdata.num_registers);    // Read Modbus registers from the microphone

        uint16_t decibel_value = parse_decibel_value(micdata.response);                                    // Parse the response to get the decibel value

        multicore_fifo_push_blocking(decibel_value);                                               // Push the converted value to the FIFO for core 0 to process

        sleep_ms(300);                                                                                  // Sleep for 300 milliseconds before the next iteration

    }
}

//Main function of the program
int main()
{
    stdio_init_all();                // Initialize standard serial communication
    init_filesystem();               // Initialize the filesystem for data storage
    setup_display();                 // Initialize the OLED display
    uart_modbus_config();            // Configure UART for Modbus communication
    wifi_init();                     // Initialize the Wi-Fi module
    start_mqtt_client();             // Start the MQTT client for remote communication
    init_and_sync_rtc();             // Configure the date and time settings

    micdata.device_address = 0x01;      // SM7901 Microphone Modbus address
    micdata.start_address = 0x0000;    // Start address for Modbus registers
    micdata.num_registers = 1;         // Number of registers to read
    micdata.sensor_id = SENSOR_ID;      // Set the sensor ID for the microphone
    micdata.latitude = MAP_LATITUDE;    // Set the latitude of the microphone
    micdata.longitude = MAP_LONGITUDE;  // Set the longitude of the microphone

    multicore_launch_core1(core1_entry); // Launch core 1 for multi-core processing

    // Main loop of the program
    while (true) {

        // CHECK CORE 1 LOGIC IF YOU NEED TO UNDERSTAND HOW IT WORKS

        uint16_t decibel_value = multicore_fifo_pop_blocking(); // Pop the decibel value from the FIFO

        micdata.dB = decibel_value / 10.0F;                      // Convert the decibel value to float and store it in micdata

        get_media_min_max_dB(&micdata);                         // Calculate the average dB value, max dB, and min dB. MQTT Publish function it's called here.

        update_display_db_value(&micdata);                      // Update dB value on the display
        
    }

    return 0; // Only for compilation purposes
}
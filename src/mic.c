//#include <math.h>         // Math library for square root and logarithm functions
#include <stdio.h>        // Standard library for Raspberry Pi Pico
//#include "hardware/adc.h" // Hardware ADC library for Raspberry Pi Pico
#include "inc/mic.h"      // Library for microphone data collection
#include "inc/mqtt.h"
#include "inc/config.h"   // Configuration library for constants and settings

// Structure to store microphone data
micdata_t micdata;

/**
 * @brief Initializes the ADC for microphone input.
 *
 * This function initializes the ADC module, configures the GPIO pin
 * for the microphone input, and selects the appropriate ADC channel.
 */

/*void setup_mic()
{
    adc_init();                    // Initialize the ADC module
    adc_gpio_init(MIC_PIN);        // Configure the microphone pin for ADC
    adc_select_input(ADC_CHANNEL); // Select ADC channel 2 for input
}*/

/**
 * @brief Collects microphone samples and stores them in the micdata structure.
 *
 * This function reads ADC values and stores them in the micdata.samples array.
 * A small delay is added between each sample to control the sampling rate.
 */

/*void collect_samples()
{
    // Read SAMPLE_COUNT samples from the ADC
    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
        micdata.samples[i] = adc_read(); // Read ADC value and store it
        sleep_us(100);                   // Delay for sampling rate control
    }
}*/

/**
 * @brief Converts digital ADC values to decibels (dB).
 *
 * @param micdata Pointer to the microphone data structure containing samples.
 * @param n_samples Number of samples to process.
 * @return nothing
 *
 * This function computes the Root Mean Square (RMS) value of the sampled data,
 * applies a logarithmic transformation, and store in dB variable the result in decibels.
 */

/*void digital_to_dB(micdata_t *micdata, uint8_t n_samples)
{

    float sum = 0.0; // Variable to store the sum of squared differences

    // Compute the sum of squared differences from the offset
    for (int i = 0; i < n_samples; i++)
    {
        float value = micdata->samples[i] - OFFSET; // Calculate the difference from the offset
        sum += value * value;                       // Add the squared difference to the sum
    }

    // Calculate the RMS value
    float rms = sqrt(sum / n_samples);

    // Convert RMS value to decibels (dB)
    micdata->dB = 20 * log10(rms / RMS_REFERENCE) + 24;


}*/

/**
 * @brief Configures the UART for Modbus communication.
 * 
 * This function initializes the UART interface with the specified baud rate,
 * sets the TX and RX pins to their respective UART functions, and configures the
 * UART format to 8 data bits, 1 stop bit, and no parity.
 */

void uart_modbus_config(){
    uart_init(UART_ID, BAUD_RATE); // Initialize UART with specified baud rate
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART); // Set TX pin function to UART
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART); // Set RX pin function to UART
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE); // Set UART format: 8 data bits, 1 stop bit, no parity
}

/**
 * @brief Calculates the Modbus CRC16 checksum for a given buffer.
 * 
 * @param buf Pointer to the buffer containing the data.
 * @param len Length of the buffer.
 * 
 * This function computes the CRC16 checksum for the provided buffer using the Modbus CRC algorithm.
 */

uint16_t modbus_crc16(uint8_t *buf, int len){
    uint16_t crc = 0xFFFF;
    for (int pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos];
        for (int i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else
                crc >>= 1;
        }
    }
    return crc;
}

/**
 * @brief Sends a Modbus request to read registers from the microphone sensor.
 * 
 * @param device_address The Modbus address of the microphone sensor.
 * @param start_address The starting address of the registers to read.
 * @param num_registers The number of registers to read.
 * 
 * This function constructs a Modbus request frame and sends it over UART to the microphone sensor.
 */

void modbus_read_registers(uint8_t device_address, uint16_t start_address, uint16_t num_registers){
    uint8_t request[8];
    request[0] = device_address;
    request[1] = 0x03; // Function code for reading holding registers
    request[2] = (start_address >> 8) & 0xFF;
    request[3] = start_address & 0xFF;
    request[4] = (num_registers >> 8) & 0xFF;
    request[5] = num_registers & 0xFF;
    uint16_t crc = modbus_crc16(request, 6);
    request[6] = crc & 0xFF;
    request[7] = (crc >> 8) & 0xFF;

    uart_write_blocking(UART_ID, request, 8);
}

/**
 * @brief Reads the Modbus response from the microphone sensor.
 * 
 * @param response Pointer to the buffer where the response will be stored.
 * @param length Length of the expected response.
 * 
 * This function reads the Modbus response from the UART interface.
 * It waits for the specified length of bytes and verifies the CRC of the response.
 * 
 * @return true if the response is valid and CRC matches, false otherwise.
 */

bool modbus_read_response(uint8_t *response, int length){
    int bytes_read = 0;
    absolute_time_t timeout = make_timeout_time_ms(1000); // Set a timeout of 1000 ms

    while (bytes_read < length && absolute_time_diff_us(get_absolute_time(), timeout) > 0) {
        if (uart_is_readable(UART_ID)) {
            response[bytes_read++] = uart_getc(UART_ID);
        }
    }

    if (bytes_read != length) {
        return false; // Timeout or not enough bytes read
    }

    //CRC verification
    uint16_t received_crc = (response[length - 1] << 8) | response[length - 2];
    uint16_t calculated_crc = modbus_crc16(response, length - 2);
    return received_crc == calculated_crc;
}

void parse_decibel_value(uint8_t *response){
    if (modbus_read_response(response, 7)) {
            uint16_t raw_value = (response[3] << 8) | response[4];
            float decibels = raw_value / 10.0;
            micdata.dB = decibels;
    } else {
        printf("Erro na leitura do sensor.\n");
    }
}

/**
 * @brief Calculates the average, maximum, and minimum dB values over a 60-second interval.
 * 
 * @param micdata Pointer to the microphone data structure containing the current dB value.
 * 
 * This function maintains a rolling average of the dB values collected over the last 60 seconds.
 * It also tracks the maximum and minimum dB values during this period. Every 60 seconds
 * it calculates the average dB, updates the max and min dB values, and publishes them to the MQTT broker.
 */

void get_media_min_max_dB(micdata_t *micdata){
    
    uint64_t current_time = time_us_64();
    static uint64_t last_attempt_time = 0;
    static float sum = 0.0;
    static uint8_t count = 0;
    static float max_dB = 0.0;
    static float min_dB = 0.0;

    // Update max and min dB values
    if (count == 0) {
        max_dB = micdata->dB;
        min_dB = micdata->dB;
    } else {
        if (micdata->dB > max_dB) max_dB = micdata->dB;
        if (micdata->dB < min_dB) min_dB = micdata->dB;
    }

    sum += micdata->dB;
    count++;

    if (current_time - last_attempt_time >= 60000000) {

        // Calculate the average dB value over the last 60 seconds
        if (count > 0) {
            micdata->average = sum / count;
            micdata->maxdB = max_dB;
            micdata->mindB = min_dB;
            
            publish_db_to_mqtt(micdata);                          // Publish the dB values to the MQTT broker
        }

        // Reset the values for the next interval
        last_attempt_time = current_time;
        sum = 0.0;
        count = 0;
        max_dB = 0.0;
        min_dB = 0.0;
    }
}
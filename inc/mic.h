#ifndef MIC_H
#define MIC_H

#include "pico/stdlib.h"   // Standard library for Raspberry Pi Pico
#include "hardware/uart.h"  // Hardware UART library for Raspberry Pi Pico
#include "inc/config.h"

/**
 * @brief Structure to hold microphone data.
 *
 * This structure stores raw samples from the microphone and the calculated
 * decibel (dB) value.
 */

typedef struct {
    uint8_t sensor_id;        ///< Unique identifier for the sensor
    //uint16_t samples[SAMPLE_COUNT];  ///< Array to store raw microphone samples
    volatile float dB;                ///< Calculated decibel level from the samples
    float average;                     ///< Average decibel level
    float maxdB;                       ///< Maximum decibel level
    float mindB;                       ///< Minimum decibel level
    float latitude;                    ///< Latitude of the microphone location
    float longitude;                   ///< Longitude of the microphone location

    uint8_t device_address;
    uint16_t start_address;
    uint16_t num_registers;
    uint8_t response[7];
    
} micdata_t;

// External declaration of the microphone data instance
extern micdata_t micdata;

/**
 * @brief Initializes the microphone.
 *
 * This function configures the necessary hardware and settings to start
 * collecting data from the microphone.
 */
//void setup_mic();

/**
 * @brief Collects samples from the microphone.
 *
 * This function reads SAMPLE_COUNT values from the microphone and stores
 * them in the micdata structure for further processing.
 */
//void collect_samples();

/**
 * @brief Converts raw microphone samples to decibel (dB) values.
 *
 * @param micdata Pointer to the micdata_t structure containing the samples.
 * @param n_samples Number of samples to process for dB calculation.
 */
//void digital_to_dB(micdata_t *micdata, uint8_t n_samples);

/**
 * @brief Calculates the average decibel level from the microphone samples.
 *
 * @param micdata Pointer to the micdata_t structure containing the samples.
 */

void uart_modbus_config();

uint16_t modbus_crc16(uint8_t *buf, int len);

void modbus_read_registers(uint8_t device_address, uint16_t start_address, uint16_t num_registers);

bool modbus_read_response(uint8_t *response, int length);

uint16_t parse_decibel_value(uint8_t *response);

void get_media_min_max_dB(micdata_t *micdata);

#endif
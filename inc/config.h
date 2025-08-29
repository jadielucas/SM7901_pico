#ifndef CONFIG_H
#define CONFIG_H

//Buttons configuration
#define BUTTON_PIN 5
#define DEBOUNCE_DELAY_MS 50

//WiFi configuration
#define WIFI_SSID "Galaxy"
#define WIFI_PASSWORD "12345678"

//MQTT configuration
#define MQTT_TOPIC "sensor/sound/pico"

#define MQTT_BROKER "test.mosquitto.org"

//Sensor configuration
//#define SAMPLE_COUNT 100   // Number of samples to collect from the microphone
//#define DB_THRESHOLD 70    // Decibel threshold for signal processing or triggering events
//#define OFFSET 2048       // Offset value for the ADC
//#define RMS_REFERENCE 1.0 // Reference value for RMS calculation
//#define MIC_PIN 28        // GPIO pin for the microphone input
//#define ADC_CHANNEL 2     // ADC channel for the microphone input

#define UART_ID uart1
#define BAUD_RATE 9600
#define UART_TX_PIN 8
#define UART_RX_PIN 9
#define MAP_LATITUDE -3.743987 // Latitude of the microphone location-3.7439874257589585, -38.53626710073022
#define MAP_LONGITUDE -38.536267 // Longitude of the microphone location
#define SENSOR_ID 1        // Unique identifier for the sensor
#define GMT_M_3 3         // GMT offset for the sensor location (e.g., GMT-3)
//Display configuration
#define SDA_PIN 14      // GPIO pin for the SDA line of the I2C interface
#define SCL_PIN 15      // GPIO pin for the SCL line of the I2C interface

//LED configuration
#define RED_LED 13   // GPIO pin for the red LED
#define GREEN_LED 11 // GPIO pin for the green LED

#endif
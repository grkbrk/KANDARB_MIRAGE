#ifndef MULTIPLEXER_SENSOR_TEST_H
#define MULTIPLEXER_SENSOR_TEST_H

#include <stdint.h>
#include <stdbool.h>

// I2C configuration
#define I2C_MASTER_NUM I2C_NUM_0
#define MULTIPLEXER_ADDR 0x70  // TCA9548A

// Sensor I2C addresses
#define SHT3X_ADDR_1 0x44  // First SHT3x
#define SHT3X_ADDR_2 0x44  // Second SHT3x (or 0x45 if different)
#define DS3231_ADDR 0x68   // DS3231 RTC

// Multiplexer channels
#define CHANNEL_SHT3X_1 0
#define CHANNEL_SHT3X_2 1
#define CHANNEL_DS3231  2

// SHT3x commands
#define SHT3X_CMD_READ_STATUS 0xF32D
#define SHT3X_CMD_MEASURE_HIGH 0x2400

// Function to select multiplexer channel
void select_mux_channel(uint8_t channel);

// Read SHT3x sensor (temperature and humidity)
bool read_sht3x(uint8_t addr, float *temperature, float *humidity);

// Read RTC
bool read_ds3231(uint8_t *second, uint8_t *minute, uint8_t *hour, float *temp_c);

#endif // MULTIPLEXER_SENSOR_TEST_H
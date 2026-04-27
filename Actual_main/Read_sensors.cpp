#include <stdio.h>
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Sensors_pins.h"


// SHT3x commands
#define SHT3X_CMD_READ_STATUS 0xF32D
#define SHT3X_CMD_MEASURE_HIGH 0x2400

// Function to select multiplexer channel
void select_mux_channel(uint8_t channel)
{
    uint8_t channel_select = (1 << channel);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (Multiplexer_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, channel_select, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    printf("1.");
}

// Read SHT3x sensor (temperature and humidity)
bool read_sht3x(uint8_t addr, float *temperature, float *humidity)
{
    // Trigger measurement
    uint8_t cmd_msb = (SHT3X_CMD_MEASURE_HIGH >> 8) & 0xFF;
    uint8_t cmd_lsb = SHT3X_CMD_MEASURE_HIGH & 0xFF;
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, cmd_msb, true);
    i2c_master_write_byte(cmd, cmd_lsb, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) return false;
    
    // Wait for measurement
    vTaskDelay(pdMS_TO_TICKS(20));
    
    // Read 6 bytes (temp high, temp low, temp CRC, hum high, hum low, hum CRC)
    uint8_t data[6];
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, 6, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) return false;
    
    // Calculate temperature (T = -45 + 175 * (raw / 65535))
    uint16_t temp_raw = (data[0] << 8) | data[1];
    *temperature = -45.0f + 175.0f * ((float)temp_raw / 65535.0f);
    
    // Calculate humidity (RH = 100 * (raw / 65535))
    uint16_t hum_raw = (data[3] << 8) | data[4];
    *humidity = 100.0f * ((float)hum_raw / 65535.0f);

    return true;
}

// Read RTC
bool read_ds3231(uint8_t *second, uint8_t *minute, uint8_t *hour, float *temp_c)
{
    // Read time registers (0x00-0x06)
    uint8_t reg_addr = 0x00;
    uint8_t time_data[7];
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (RTC_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (RTC_addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, time_data, 7, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) return false;
    
    // Parse BCD to decimal
    *second = (time_data[0] & 0x0F) + ((time_data[0] >> 4) * 10);
    *minute = (time_data[1] & 0x0F) + ((time_data[1] >> 4) * 10);
    *hour   = (time_data[2] & 0x0F) + ((time_data[2] >> 4) * 10);
    
    // Read temperature (registers 0x11 and 0x12)
    reg_addr = 0x11;
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (RTC_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (RTC_addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, time_data, 2, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret == ESP_OK) {
        *temp_c = (float)time_data[0] + (float)(time_data[1] >> 6) * 0.25f;
    }

    return true;
}
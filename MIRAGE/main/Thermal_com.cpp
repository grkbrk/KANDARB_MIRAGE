#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "Settings.h"
#include "Multiplexer.h"
#include "thermal_com.h"

// Send live sensor data to thermal MCU
bool thermal_send_sensor_data(
    const SensorData* data)
{
    select_mux_channel(
        multiplex_Thermal
    );

    uint8_t packet[13];

    packet[0] = THERMAL_PACKET_DATA;

    memcpy(&packet[1],  &data->Ta1, sizeof(float));
    memcpy(&packet[5],  &data->Ha1, sizeof(float));
    memcpy(&packet[9],  &data->Pa1, sizeof(float));

    esp_err_t err =
        i2c_master_write_to_device(
            I2C_master,
            Thermal_MCU_addr,
            packet,
            sizeof(packet),
            100 / portTICK_PERIOD_MS
        );

    return (err == ESP_OK);
}

// Send temporary command
bool thermal_send_command(
    ThermalCommand cmd)
{
    select_mux_channel(
        multiplex_Thermal
    );

    uint8_t packet[2];

    packet[0] = THERMAL_PACKET_COMMAND;

    packet[1] = cmd;

    esp_err_t err =
        i2c_master_write_to_device(
            I2C_master,
            Thermal_MCU_addr,
            packet,
            sizeof(packet),
            100 / portTICK_PERIOD_MS
        );

    return (err == ESP_OK);
}

// Update persistent regulation setting
bool thermal_update_setting(
    ThermalSetting setting,
    float value)
{
    select_mux_channel(
        multiplex_Thermal
    );

    uint8_t packet[6];

    packet[0] = THERMAL_PACKET_SETTING;

    packet[1] = setting;

    memcpy(
        &packet[2],
        &value,
        sizeof(float)
    );

    esp_err_t err =
        i2c_master_write_to_device(
            I2C_master,
            Thermal_MCU_addr,
            packet,
            sizeof(packet),
            100 / portTICK_PERIOD_MS
        );

    return (err == ESP_OK);
}

// Read thermal MCU status
bool thermal_read_status(
    ThermalStatus* status)
{
    select_mux_channel(
        multiplex_Thermal
    );

    uint8_t data[6];

    esp_err_t err =
        i2c_master_read_from_device(
            I2C_master,
            Thermal_MCU_addr,
            data,
            sizeof(data),
            100 / portTICK_PERIOD_MS
        );

    if (err != ESP_OK)
    {
        status->online = false;

        return false;
    }

    status->online = true;

    status->state =
        data[0];

    status->error =
        data[1];

    memcpy(
        &status->internal_temperature,
        &data[2],
        sizeof(float)
    );

    return true;
}

// Hardware reset of thermal MCU
void thermal_reset()
{
    gpio_set_level(
        Thermal_reset_PIN,
        0
    );

    vTaskDelay(
        pdMS_TO_TICKS(100)
    );

    gpio_set_level(
        Thermal_reset_PIN,
        1
    );
}
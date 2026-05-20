#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "Settings.h"
#include "Multiplexer.h"
#include "thermal_com.h"

// Select slave mcu
static bool select_slave(
    SlaveDevice slave,
    gpio_num_t* reset_pin)
{
    switch(slave)
    {
        case SLAVE_THERMAL:

            select_mux_channel(
                multiplex_Thermal
            );

            *reset_pin =
                Thermal_reset_PIN;

            return true;


        case SLAVE_PRESSURE:

            select_mux_channel(
                multiplex_Pressure
            );

            *reset_pin =
                Pressure_reset_PIN;

            return true;
    }

    return false;
}

// Send telemetry/data to thermal MCU
bool thermal_send_data(
    DataID id,
    float value)
{
    select_mux_channel(

    );

    uint8_t packet[6];

    // Packet type
    packet[0] = THERMAL_PACKET_DATA;

    // Which telemetry this is
    packet[1] = id;

    // Copy float into packet
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


// ==================================================
// Send temporary runtime command
// ==================================================

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


// ==================================================
// Update persistent regulation setting
// ==================================================

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


// ==================================================
// Read status from thermal MCU
// ==================================================

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

    // Thermal MCU state
    status->state =
        data[0];

    // Error flags
    status->error =
        data[1];

    // Internal temperature
    memcpy(
        &status->internal_temperature,
        &data[2],
        sizeof(float)
    );

    return true;
}


// ==================================================
// Hardware reset thermal MCU
// ==================================================

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
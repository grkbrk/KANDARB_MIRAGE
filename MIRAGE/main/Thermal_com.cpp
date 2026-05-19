#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h"
#include "driver/gpio.h"

#include "Settings.h"
#include "Multiplexer.h"
#include "thermal_com.h"


// ==================================================
// Packet types
// ==================================================

#define PACKET_COMMAND    0x01
#define PACKET_SETTING    0x02


// ==================================================
// Example command IDs
// ==================================================

#define CMD_PUMPS_ON          0x01
#define CMD_PUMPS_OFF         0x02

#define CMD_OPEN_SHUTTERS     0x03
#define CMD_CLOSE_SHUTTERS    0x04

#define CMD_AUTONOMOUS_MODE   0x05


// ==================================================
// Example setting IDs
// ==================================================

#define SET_TARGET_TEMP       0x01


// ==================================================
// Send setting/value to thermal MCU
// Example:
// target temperature = 20°C
// ==================================================

bool thermal_send_data(
    uint8_t setting_id,
    float value)
{
    select_mux_channel(
        multiplex_Thermal
    );

    uint8_t packet[6];

    packet[0] = PACKET_SETTING;

    packet[1] = setting_id;

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
// Send command to thermal MCU
// ==================================================

bool thermal_send_command(
    uint8_t command)
{
    select_mux_channel(
        multiplex_Thermal
    );

    uint8_t packet[2];

    packet[0] = PACKET_COMMAND;

    packet[1] = command;

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
// Reset thermal MCU
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


// ==================================================
// Thermal status structure
// ==================================================

typedef struct
{
    bool online;

    uint8_t state;

    uint8_t error;

} ThermalStatus;


// ==================================================
// Read thermal MCU status
// ==================================================

bool thermal_read_status(
    ThermalStatus* status)
{
    select_mux_channel(
        multiplex_Thermal
    );

    uint8_t data[2];

    esp_err_t err =
        i2c_master_read_from_device(
            I2C_master,
            Thermal_MCU_addr,
            data,
            2,
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

    return true;
}
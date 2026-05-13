#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "Settings.h"
#include "uart.h"

void K96_on()
{
    gpio_set_level(K96_EN_PIN, 1);
}

void K96_off()
{
    gpio_set_level(K96_EN_PIN, 0);
}

// Creates checksum for Modbus, used by K96 for error checking
static uint16_t modbus_crc16(
    const uint8_t* data,
    uint16_t length)
{
    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < length; i++)
    {
        crc ^= data[i];

        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}


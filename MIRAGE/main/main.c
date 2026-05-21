#include <stdio.h>
#include <string.h>

#include "Settings.h"
#include "ConnectionLoss.h"
#include "EthernetCom.h"
#include "Humidity.h"
#include "Initialize.h"
#include "Multiplexer.h"
#include "Neopixel.h"
#include "read_sensors.h"
#include "SDCard.h"
#include "Uart.h"
#include "watchdog.h"

bool loop_exp = true;

/* Modes
 * 1: Test Loop
 * 2: Standby
 * 3: Meassurement
 * 4: Humidity
 */
int mode = DEFAULT_MODE // 1

    // Watchdog
    bool system_ok;

// Ethernet
uint8_t ethernet_recieve_buf[ETHERNET_BUF_SIZE] = {0};
size_t ethernet_recieve_buf_size = ETHERNET_BUF_SIZE;
size_t ethernet_recieve_buf_bytes_read = 0;

static const char *TAG = "main";

/*  Put somewhere. For ConnectionLoss
 *   volatile int64_t loss_timestamp_us = -1;
 *   volatile bool    con_lost          = false;
 *   int              mode              = 0;
 *   bool             terminated        = false;
 */

// ESP-IDF expects main in C
extern "C" void app_main()
{
    // This should be one func in Initialize instead?
    init_gpio_pins();
    init_spi();
    init_i2c();
    init_uart_k96();
    init_sensors();

    while (loop_exp == true)
    {
        loop();
    }
}

void loop()
{
    // Common actions
    feed_watchdog(system_ok);
    // Check for commands
    esp_err_status = wiz_receive(ethernet_recieve_buf, ethernet_recieve_buf_size, &ethernet_recieve_buf_bytes_read);

    // Measure time of loop
    // Realtidsklockan instead?
    TickType_t currentTime_start = xTaskGetTickCount();

    if (esp_err_status == ESP_ERR_NOT_FOUND)
    {
        // No data in buffer = no command from ground
    }
    else if (esp_err_status == ESP_OK)
    {
        // COmmand recieved
        if (mode == 1)
        {
            mode = 2;
        }
        handle_command(); //use a dictionairy of commands??
    }
    else if (esp_err_status == ESP_FAIL)
    {
        // Error when retrieving data
        // What to do here?
    }
    else
    {
        // Unknown return
    }

    // Collec I2C data
    if (mode != 1)
    {
        read_sensors();
        //buffer_SD_data_binary(); //est time: 1.5 ms
        //buffer_SD_data_csv();      //est time: 3 ms
        //buffer_SD_data_binary_large(); //est time: 1.5 ms every 8th loop
        buffer_SD_data_csv(sensor_data);      //est time: 3 ms every 8th loop    
    }

    

    // Mode dependent actions
    switch (mode)
    {
    // Should this be "default:"?
    // Test loop
    case 1:

        // Bark at subsystems
        //!!!!!!!!!!

        //!!!!!
        // Enter IP when given by ESA
        esp_err_status = wiz_ping(uint8_t *target_ip, "No command recieved. Status: OK.");

        break;

    // Standby
    case 2:
        /* code */

        break;

    // Measurement
    case 3:
        /* code */
        break;

    // Leave for now as stated by Anna
    // Humidity
    case 4:
        /* code */
        ESP_LOGI(TAG, "Humidity loop not implemented");
        break;

    default:
        mode = 1;
        wiz_send(uint8_t *target_ip, "Unknown mode. Returning to test loop.");
        break;
    }

    // Wait until loop has taken 100 ms.
    TickType_t current_time_stop = xTaskGetTickCount();
    time_loop = pdMS_TO_TICKS(100) - (current_time_stop - current_time_start) if (time_loop > 0)
    {
        vTaskDelay(time_loop);
    }
}

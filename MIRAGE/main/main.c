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
        buffer_SD_data_csv_large();      //est time: 3 ms every 8th loop    
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

// ===================================================================
// Option 1: Binary buffer (fastest, smallest)
// ===================================================================
void buffer_SD_data_binary()
{
    uint8_t buf[sizeof(sensor_data)];
    memcpy(buf, &sensor_data, sizeof(buf));
    sd_write("sensor_data.bin", buf, sizeof(buf));
    ESP_LOGI(TAG, "Buffered %zu bytes (binary)", sizeof(buf));
}

// ===================================================================
// Option 2: CSV text format (readable, larger)
// ===================================================================
void buffer_SD_data_csv()
{
    char line[512];
    int n = snprintf(line, sizeof(line),
        "%u,%u,%u,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%u\n",
        sensor_data.hours,
        sensor_data.minutes,
        sensor_data.seconds,
        sensor_data.Tp1,
        sensor_data.Tp2,
        sensor_data.Tp3,
        sensor_data.Tp6,
        sensor_data.Pp3,
        sensor_data.Tp4,
        sensor_data.Pp1,
        sensor_data.Pa1,
        sensor_data.Ta1,
        sensor_data.Ta2,
        sensor_data.Ha1,
        sensor_data.Tp5,
        sensor_data.Pp2,
        sensor_data.Tt1,
        sensor_data.Tt2,
        sensor_data.Tt3,
        sensor_data.K96_CO2,
        sensor_data.K96_pressure,
        sensor_data.K96_temperature,
        sensor_data.K96_humidity,
        sensor_data.K96_error
    );
    sd_write("sensor_data.csv", (const uint8_t *)line, n);
    ESP_LOGI(TAG, "Buffered %d bytes (CSV)", n);
}

// Buffer configuration
#define SD_BUFFER_SIZE 4096
#define SENSOR_READING_SIZE sizeof(SensorData)
#define READINGS_PER_BUFFER (SD_BUFFER_SIZE / SENSOR_READING_SIZE)

static uint8_t SD_buffer[SD_BUFFER_SIZE];
static size_t SD_buffer_offset = 0;

// ===================================================================
// Option 1: Binary buffer large (fastest, smallest)
// ===================================================================
void buffer_SD_data_binary_large()
{
    // Check if there's space for another reading
    if (SD_buffer_offset + SENSOR_READING_SIZE <= SD_BUFFER_SIZE)
    {
        // Copy current sensor reading into buffer
        memcpy(&SD_buffer[SD_buffer_offset], &sensor_data, SENSOR_READING_SIZE);
        SD_buffer_offset += SENSOR_READING_SIZE;
    }
    
    // Write to SD when buffer is full
    if (SD_buffer_offset >= SD_BUFFER_SIZE)
    {
        esp_err_t err = sd_write("sensor_data.bin", SD_buffer, SD_BUFFER_SIZE);
        if (err == ESP_OK)
        {
            ESP_LOGI(TAG, "Wrote %d readings (%zu bytes) to SD", 
                     READINGS_PER_BUFFER, SD_BUFFER_SIZE);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to write buffer to SD");
        }
        SD_buffer_offset = 0;  // Reset for next batch
    }
}

void buffer_SD_data_csv_large()
{
    char line[512];
    int n = snprintf(line, sizeof(line),
        "%u,%u,%u,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%u\n",
        sensor_data.hours,
        sensor_data.minutes,
        sensor_data.seconds,
        sensor_data.Tp1,
        sensor_data.Tp2,
        sensor_data.Tp3,
        sensor_data.Tp6,
        sensor_data.Pp3,
        sensor_data.Tp4,
        sensor_data.Pp1,
        sensor_data.Pa1,
        sensor_data.Ta1,
        sensor_data.Ta2,
        sensor_data.Ha1,
        sensor_data.Tp5,
        sensor_data.Pp2,
        sensor_data.Tt1,
        sensor_data.Tt2,
        sensor_data.Tt3,
        sensor_data.K96_CO2,
        sensor_data.K96_pressure,
        sensor_data.K96_temperature,
        sensor_data.K96_humidity,
        sensor_data.K96_error
    );

    // If the line doesn't fit, flush current buffer first
    // Extra check since csv can be variable length and might exceed buffer size on its own,
    // in that case we should write it directly instead of trying to buffer it
    if ((size_t)n + SD_buffer_offset >= SD_BUFFER_SIZE)
    {
        esp_err_t err = sd_write("sensor_data.csv", SD_buffer, SD_buffer_offset);
        if (err == ESP_OK)
        {
            ESP_LOGI(TAG, "Flushed %zu bytes CSV to SD", SD_buffer_offset);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to flush CSV buffer to SD");
        }
        SD_buffer_offset = 0;
    }

    // Append the line bytes into the buffer
    memcpy(&SD_buffer[SD_buffer_offset], line, (size_t)n);
    SD_buffer_offset += (size_t)n;

    // If buffer full after append, write it out
    if (SD_buffer_offset >= SD_BUFFER_SIZE)
    {
        esp_err_t err = sd_write("sensor_data.csv", SD_buffer, SD_BUFFER_SIZE);
        if (err == ESP_OK)
        {
            ESP_LOGI(TAG, "Wrote %zu bytes CSV to SD", SD_BUFFER_SIZE);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to write CSV buffer to SD");
        }
        SD_buffer_offset = 0;  // Reset for next batch
    }
}
// Flush remaining data (call before shutdown)
void buffer_SD_data_binary_flush()
{
    if (SD_buffer_offset > 0)
    {
        esp_err_t err = sd_write("sensor_data.bin", SD_buffer, SD_buffer_offset);
        if (err == ESP_OK)
        {
            ESP_LOGI(TAG, "Flushed %zu bytes to SD", SD_buffer_offset);
        }
        SD_buffer_offset = 0;
    }
}

// CSV-specific flush (call if using CSV accumulator)
void buffer_SD_data_csv_flush()
{
    if (SD_buffer_offset > 0)
    {
        esp_err_t err = sd_write("sensor_data.csv", SD_buffer, SD_buffer_offset);
        if (err == ESP_OK)
        {
            ESP_LOGI(TAG, "Flushed %zu bytes CSV to SD", SD_buffer_offset);
        }
        SD_buffer_offset = 0;
    }
}
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Multiplexer_sensor_test.h"

// Define pins
#define LED_GPIO GPIO_NUM_14
#define MULTIPLEXER_SDA GPIO_NUM_26
#define MULTIPLEXER_SCL GPIO_NUM_25

// I2C configuration
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000

extern "C" void app_main(void)
{
    // Configure GPIO pin 14 as output
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    int counter = 0;

    //////////////////////////////////Sensor/multiplexer////////////////////////////////////////
    // // Configure I2C'
    // i2c_config_t conf = {
    //     .mode = I2C_MODE_MASTER,
    //     .sda_io_num = MULTIPLEXER_SDA,
    //     .scl_io_num = MULTIPLEXER_SCL,
    //     .sda_pullup_en = GPIO_PULLUP_ENABLE,
    //     .scl_pullup_en = GPIO_PULLUP_ENABLE,
    //     .master = {
    //         .clk_speed = I2C_MASTER_FREQ_HZ,
    //     },
    //     .clk_flags = 0,
    // };
    
    // i2c_param_config(I2C_MASTER_NUM, &conf);
    // i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    //////////////////////////////////Sensor/multiplexer////////////////////////////////////////

    while (1) {
        // Toggle the LED
        gpio_set_level(LED_GPIO, counter % 2);
        
        // Delay for 1 second
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        counter++;
        // Every 10 seconds, print sensor data with timestamp
        printf("\nCounter: %d", counter);
        if (counter % 10 == 0) {
            printf("\n");
            counter = 0;








        //////////////////////////////////Sensor/multiplexer////////////////////////////////////////
        //     time_t now;
        //     time(&now);
        //     struct tm *timeinfo = localtime(&now);
        //     char timeStr[64];
        //     strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);
            
        //     printf("\n=== Sensor Data at %s ===\n", timeStr);
            
        //     // Read SHT3x #1 on channel 0
        //     select_mux_channel(CHANNEL_SHT3X_1);
        //     float temp1, hum1;
        //     if (read_sht3x(SHT3X_ADDR_1, &temp1, &hum1)) {
        //         printf("SHT3X #1: Temp=%.2f C, Humidity=%.2f %%\n", temp1, hum1);
        //     } else {
        //         printf("SHT3X #1: FAILED to read\n");
        //     }
            
        //     // Read SHT3x #2 on channel 1
        //     select_mux_channel(CHANNEL_SHT3X_2);
        //     float temp2, hum2;
        //     if (read_sht3x(SHT3X_ADDR_2, &temp2, &hum2)) {
        //         printf("SHT3X #2: Temp=%.2f C, Humidity=%.2f %%\n", temp2, hum2);
        //     } else {
        //         printf("SHT3X #2: FAILED to read\n");
        //     }
            
        //     // Read DS3231 on channel 2
        //     select_mux_channel(CHANNEL_DS3231);
        //     uint8_t sec, min, hr;
        //     float temp_rtc;
        //     if (read_ds3231(&sec, &min, &hr, &temp_rtc)) {
        //         printf("DS3231 RTC: %02d:%02d:%02d, Temp=%.2f C\n", hr, min, sec, temp_rtc);
        //     } else {
        //         printf("DS3231 RTC: FAILED to read\n");
        //     }
            
        //     printf("================================\n");
        //////////////////////////////////Sensor/multiplexer////////////////////////////////////////
        }
    }
}

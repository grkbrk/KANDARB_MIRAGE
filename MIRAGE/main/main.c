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

bool command_received = false;

bool connection_lost = false; // To track connection status
int64_t loss_timestamp_us = -1; // To track when connection was lost for termination
int loops_since_connection = 0; // To buffer short con losses for stable running

//Pressure
bool shutters_open = false; // To track if shutters are open

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
    loops_since_connection++; //Will be reset in handle_ethernet_data if connection is ok
    esp_err_status = wiz_receive(ethernet_recieve_buf, ethernet_recieve_buf_size, &ethernet_recieve_buf_bytes_read);
    handle_ethernet_data(esp_err_status);


    // Collec I2C data
    if (mode != 1)
    {
        read_sensors();
        //buffer_SD_data_binary_single(); //est time: 1.5 ms
        //buffer_SD_data_csv_single();      //est time: 3 ms
        //buffer_SD_data_binary(sensor_data); //4k - est time: 1.5 ms every 8th loop
        buffer_SD_data_csv(sensor_data);      //4k - est time: 3 ms every 8th loop    
    }



    // Mode dependent actions
    switch (mode)
    {
    // Test loop
    case 1:

        // Bark at subsystems
        //!!!!!!!!!!

        //!!!!!
        // Enter IP when given by ESA
        // Ping ground that status is OK.
        esp_err_status = wiz_ping(uint8_t *target_ip, "No command recieved. Status: OK.");

        break;

    // Standby
    case 2:
        //Reset overrides

        //Pressure communication
        
        //Thermal communication

        break;

    // Measurement
    case 3:
        // Humidity check here if any 

        // Elevation check in terms of pressure
        if (sensor_data.Pa1 < P_STRATOSPHERE)
        {
            if loops_since_connection > LOOP_WO_CONNECTION // If connection lost for more than 10 loops, enter safe mode
            {
                mode = 2; // Standby
                connection_lost = true;
                connection_lost(&connection_lost, &loss_timestamp_us);
                
                wiz_ping(uint8_t *target_ip, "Connection lost. Entering safe mode.");
                break;
            }
            //else: high altidude but have connection.
        }
        //Check if pressure in chamber is below threshold, if so, increase pressure first.
        if (sensor_data.Pp2 < CHAMBER_P_SHUTTER_THRESHOLD)
        {
            //Pressure communication: increase p in chamber.
            break;
        } 
        //Open shutters if close
        //Skip data collection to ensure proper pressure in chamber.
        if (shutters_open == false)
        {
            //Pressure communication: open shutters
            shutters_open = true;
            break;
        }
        //Check if pressure in chamber is above threshold, if so, take meassurements.
        if (sensor_data.Pp2 < CHAMBER_P_CHAMBER_THRESHOLD)
        {
            //Pressure communication: increase p in chamber.
            break;
        }
        // Check if inlet temperature is above threshold, if so, take meassurements. If not, decrease inlet temperature.
        if (sensor_data.Tt3 < INLET_TEMPERATURE_THRESHOLD)
        {
            //Thermal communication: increase inlet temperature
            break;
        }
        // Take meassurements!!!
        readK96();
        buffer_SD_data_csv(sensor_data); 
        break;

    // Leave for now as stated by Anna
    // Humidity
    case 4:
        
        ESP_LOGI(TAG, "Humidity loop not implemented");
        break;

    default:
        mode = 1;
        wiz_send(uint8_t *target_ip, "Unknown mode. Returning to test loop.");
        break;
    }

    //Transmit data over E-Link
    uint8_t ethernet_send_buf[sizeof(sensor_data)];
    uint8_t satus_message[64] = ("Status: %d. Command recieved: %d. Mode: %d.",status_ok,command_received,mode);
    memcpy(ethernet_send_buf, &sensor_data, sizeof(ethernet_send_buf));
    memcpy(ethernet_send_buf, &satus_message, sizeof(ethernet_send_buf));
    wiz_send(ethernet_send_buf, sizeof(ethernet_send_buf));

    // Wait until loop has taken 100 ms.
    TickType_t current_time_stop = xTaskGetTickCount();
    time_loop = pdMS_TO_TICKS(100) - (current_time_stop - current_time_start) if (time_loop > 0)
    {
        vTaskDelay(time_loop);
    }
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void handle_ethernet_data(esp_err_t esp_err_status)
{
    switch (esp_err_status)
    {
    case ESP_ERR_NOT_FOUND:
        // No data in buffer = no command from ground
        command_received = false;
        break;
    
    case ESP_OK:
        // Command recieved
        if (mode == 1)
        {
            mode = 2;
        }
        handle_command(); 
        command_received = true;
        connection_lost = false;
        loops_since_connection = 0; // Reset connection loss buffer
        connection_reestablished(&connection_lost, &loss_timestamp_us); 
        break;

    case ESP_FAIL:
        // Error when receiving data
        // What to do here?
         break;
    
    default:
        //Unexpected return
        ESP_LOGI(TAG, "Unexpected return from wiz_receive: %d", esp_err_status);
        break;
    }
}

void handle_command()
{
    // Interpret command in ethernet_recieve_buf and act accordingly
    // For now, just log the received command
    ESP_LOGI(TAG, "Received command: %.*s", (int)ethernet_recieve_buf_bytes_read, ethernet_recieve_buf);
}
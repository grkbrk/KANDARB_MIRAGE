#include <stdio.h>

#include "Settings.h"

/* Modes
* 1: Test Loop
* 2: Standby
* 3: Meassurement
* 4: Humidity
*/
int mode = DEFAULT_MODE //1

static const char *TAG = "main"

/*  Put somewhere. For ConnectionLoss
 *   volatile int64_t loss_timestamp_us = -1;
 *   volatile bool    con_lost          = false;
 *   int              mode              = 0;
 *   bool             terminated        = false;
*/

//ESP-IDF expects main in C 
extern "C" void app_main()
{
    //This should be one func in Initialize instead?
    init_gpio_pins();
    init_spi();
    init_i2c();
    init_uart_k96();
    init_sensors();
    
    loop();
}

void loop()
{
    switch (mode)
    {
    //Should this be "default:"?
    //Test loop
    case 1:
        /* code */
        
        break;

    //Standby
    case 2:
        /* code */
        break;

    //Measurement
    case 3:
        /* code */
        break;

    //Leave for now as stated by Anna
    //Humidity
    case 4:
        /* code */
        ESP_LOGI(TAG, "Humidity loop not implemented");
        break;
    
    default:
        break;
    }
}
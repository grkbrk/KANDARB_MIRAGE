//us = microseconds

#include "esp_log.h"

/* Use this instead of external RTC since big time span
 and it will still work in case something would happen to
 the external RTC. 
*/ 
#include "esp_timer.h"   // for esp_timer_get_time()

static const char *TAG = "Connection loss / termination timer";

/* in main or something instead:
// Stores the timestamp (microseconds) when connection was lost. -1 = not running.
static volatile int64_t loss_timestamp_us = -1;
static volatile bool con_lost = false;
*/


// 30 min in microseconds (esp_timer runs on us)
#define TERMINATION_TIMEOUT (30LL * 60LL *1000000LL)

esp_err_t connection_lost(bool *con_lost, int64_t *loss_timestamp_us){
    *loss_timestamp_us = esp_timer_get_time(); //in us
    ESP_LOGI(TAG, "Termination timer started at %lld us", *loss_timestamp_us);

    *con_lost = true;
    ESP_LOGI(TAG, "Connection lost %d", *con_lost);

    return ESP_OK;
}

esp_err_t connection_reestablished(bool *con_lost, int64_t *loss_timestamp_us){
    int64_t elapsed = esp_timer_get_time() - *loss_timestamp_us;
    ESP_LOGI(TAG, "Termination timer stoped at %lld us", elapsed);
    
    *loss_timestamp_us = -1;  //Reset timer

    *con_lost = false;    
    ESP_LOGI(TAG, "Connection lost %d", *con_lost);

    return ESP_OK;
}

esp_err_t timer_status(int64_t *loss_timestamp_us){
    int64_t elapsed = esp_timer_get_time() - *loss_timestamp_us;
    if (elapsed > TERMINATION_TIMEOUT){
        ESP_LOGI(TAG, "Termination timer exceeded 30 min = %lld us", TERMINATION_TIMEOUT);
        
        return terminate_experiment();
    }
    else{
        ESP_LOGI(TAG, "Termination timer status: %lld", elapsed);
        return ESP_OK;
    }
}
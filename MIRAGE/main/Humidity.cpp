#include "esp_log.h"
#include "Settings.h"



static const char *TAG = "Humidity";
static bool humid = true;

/*Helpers*/
esp_err_t compare_humidity(int *humidity)
    if(*humidity>=HUMIDITY_TARGET){
        *humid = true;
        ESP_LOGI(TAG, "Humidity too high at , %zu", *humidity);
        return ESP_OK;
    }    
    else if (*humidity<HUMIDITY_TARGET){
        ESP_LOGI(TAG, "Humidity OK at , %zu", *humidity);
        *humid = false;
        return ESP_OK;
    }

/*API Facing functions*/
esp_err_t check_humidity_restored(int *temp, int *humidity, bool *humidity_setting)
{
    if(*temp<TEMPERATURE_TARGET){
        ESP_LOGI(TAG, "Temperature too low, %zu", *temp);
        return ESP_OK;
    }
    ESP_LOGI(TAG, "Humidity levels restored, %zu", *humidity);
    ESP_LOGI(TAG, "Temperature requirements met, %zu", *temp);
    ESP_LOGI(TAG, "Humidity levels restored, %zu", *humidity_setting);
    return ESP_OK;
}


esp_err_t check_humidity_levels(int *temp, int *humidity, bool *humidity_setting){
    humid = compare_humidity();
    if (humid == false && *humidity_setting == false){
        ESP_LOGI(TAG, "Continue, humidity levels OK");
        return ESP_OK;
    }
    else if (humid == true && *humidity_setting == false){
        ESP_LOGI(TAG, "Humidity too high. Enter humidity setting.");
        *humidity_setting = true;
        return ESP_OK;
    }
    else if (humid == false && *humidity_setting == true){
        if(*temp<TEMPERATURE_TARGET){
            ESP_LOGI(TAG, "Temperature too low: , %zu", *temp);
            return ESP_OK;
        }
        else if(*temp>=TEMPERATURE_TARGET){
            ESP_LOGI(TAG, "Temperature target reached: , %zu", *temp);
            *humidity_setting = false;
            ESP_LOGI(TAG, "Exit humidity setting");
            return ESP_OK;
        }
    }
    else if (humid == true && *humidity_setting == true){
        ESP_LOGI(TAG, "Humidity too high. Wait for lower abient humidity.");
        return ESP_OK;
    }
    return ESP_OK
}
humidty_too_high{
    close_shutter()
    stop_pump()
}
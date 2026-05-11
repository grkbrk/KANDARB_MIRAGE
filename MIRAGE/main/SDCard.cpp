//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//100% Calude.................
// Why use FatFS: Mainly: you can plug it in to a computer. https://docs.espressif.com/projects/esp-idf/en/latest/esp32c5/api-guides/file-system-considerations.html
//                  https://www.engineersgarage.com/esp32-sd-card-emmc-filesystems/
//Inspiration: https://github.com/espressif/esp-idf/blob/526f682397a8cfb74698c601fd2c5b30e1433837/examples/storage/sd_card/main/sd_card_example_main.c
                    https://github.com/espressif/esp-idf/blob/v6.0.1/examples/storage/fatfs/getting_started/main/fatfs_getting_started_main.c
                    
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/**
 * SDCard.cpp
 *
 * SD card specific operations only.
 * Does NOT initialise the SPI bus — that is done once in Initialize.cpp.
 *
 * CS pin: IO9 (CS1)
 */

#include "SDCard.h"

#include <stdio.h>
#include <errno.h>

#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

static const char    *TAG      = "SDCard";
static sdmmc_card_t  *s_card   = NULL;
static bool           s_mounted = false;

// ---------------------------------------------------------------------------
// Mount / unmount  (called from Initialize.cpp, not by application code)
// ---------------------------------------------------------------------------

esp_err_t sd_mount(void)
{
    if (s_mounted) {
        ESP_LOGW(TAG, "Already mounted");
        return ESP_OK;
    }

    // Register the SD card as an SPI device on the existing SPI2 bus
    sdspi_device_config_t dev_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    dev_cfg.gpio_cs = SD_PIN_CS;
    dev_cfg.host_id = SPI2_HOST;

    esp_vfs_fat_sdmmc_mount_config_t mount_cfg = {
        .format_if_mount_failed = false,
        .max_files              = 5,
        .allocation_unit_size   = 16 * 1024,
    };

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;

    esp_err_t ret = esp_vfs_fat_sdspi_mount(SD_MOUNT_POINT, &host,
                                             &dev_cfg, &mount_cfg, &s_card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Mount failed: %s", esp_err_to_name(ret));
        return ret;
    }

    s_mounted = true;
    ESP_LOGI(TAG, "Mounted at %s", SD_MOUNT_POINT);
    sdmmc_card_print_info(stdout, s_card);
    return ESP_OK;
}

void sd_unmount(void)
{
    if (!s_mounted) return;

    esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, s_card);
    s_card    = NULL;
    s_mounted = false;
    ESP_LOGI(TAG, "Unmounted");
}

// ---------------------------------------------------------------------------
// Application-facing functions
// ---------------------------------------------------------------------------

esp_err_t sd_write(const char *filename, const uint8_t *data, size_t length)
{
    if (!s_mounted)           return ESP_ERR_INVALID_STATE;
    if (!filename || !data || length == 0) return ESP_ERR_INVALID_ARG;

    char path[SD_MAX_PATH_LEN];
    snprintf(path, sizeof(path), "%s/%s", SD_MOUNT_POINT, filename);

    // "ab" = append
    FILE *f = fopen(path, "ab");
    if (!f) {
        ESP_LOGE(TAG, "Cannot open %s (errno %d)", path, errno);
        return ESP_FAIL;
    }

    size_t written = fwrite(data, 1, length, f);
    fclose(f);

    if (written != length) {
        ESP_LOGE(TAG, "Wrote %zu/%zu bytes", written, length);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Wrote %zu bytes -> %s", written, path);
    return ESP_OK;
}

esp_err_t sd_read(const char *filename, uint8_t *out_buf,
                  size_t buf_size, size_t *bytes_read)
{
    // if (!s_mounted)                              return ESP_ERR_INVALID_STATE;
    // if (!filename || !out_buf || !bytes_read)    return ESP_ERR_INVALID_ARG;

    char path[SD_MAX_PATH_LEN];
    snprintf(path, sizeof(path), "%s/%s", SD_MOUNT_POINT, filename);

    FILE *f = fopen(path, "rb");
    if (!f) {
        ESP_LOGE(TAG, "Cannot open %s (errno %d)", path, errno);
        return ESP_FAIL;
    }

    *bytes_read = fread(out_buf, 1, buf_size, f);
    fclose(f);

    ESP_LOGI(TAG, "Read %zu bytes <- %s", *bytes_read, path);
    return ESP_OK;
}

esp_err_t sd_wipe_files(void)
{
    DIR *dor = opendir(SD_MOUNT_POINT);
    if (!dir){
        ESP_LOGE(TAG, "Failed to open directory (errno %d)", errno);
        return ESP_FAIL;
    }

    struct dirent *entry;
    int failed = 0;

    while ((entry = readdir(dir)) != NULL) {
        // Skip directories (e.g. "." and "..")
        if (entry->d_type == DT_DIR) continue;
 
        char path[SD_MAX_PATH_LEN];
        snprintf(path, sizeof(path), "%s/%s", SD_MOUNT_POINT, entry->d_name);
 
        if (unlink(path) != 0) {
            ESP_LOGE( TAG, "Failed to delete %s (errno %d)", path, errno);
            failed++;
        } else {
            ESP_LOGI(TAG, "Deleted %s", path);
        }
    }

    closedir(dir);
    
    if (failed > 0) {
    ESP_LOGW(TAG, "%d file(s) could not be deleted", failed);
    return ESP_FAIL;
    }

    ESP_LOGI(TAG, "All files deleted");
    return ESP_OK;
 
}

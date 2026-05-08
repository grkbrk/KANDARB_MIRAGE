//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//100% Calude.................
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

// ---------------------------------------------------------------
// SD card specific pins
// ---------------------------------------------------------------
#define SD_PIN_CS       9       // IO9 - CS1 (microSD)

#define SD_MOUNT_POINT  "/sdcard"
#define SD_MAX_PATH_LEN 64

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Register the SD card on the already-initialised SPI2 bus
 *         and mount the FAT filesystem.
 *         Called by hardware_init() in Initialize.cpp — do not call directly.
 *
 * @return ESP_OK on success, or an esp_err_t error code.
 */
esp_err_t sd_mount(void);

/**
 * @brief  Write a binary buffer to a file on the SD card.
 *         Data is appended if the file already exists.
 *
 * @param[in] filename  File name only (e.g. "data.bin"), not the full path.
 * @param[in] data      Pointer to the buffer to write.
 * @param[in] length    Number of bytes to write.
 *
 * @return ESP_OK on success, or an esp_err_t error code.
 */
esp_err_t sd_write(const char *filename, const uint8_t *data, size_t length);

/**
 * @brief  Read a binary file from the SD card into a caller-supplied buffer.
 *
 * @param[in]  filename    File name only (e.g. "data.bin"), not the full path.
 * @param[out] out_buf     Buffer to receive the data.
 * @param[in]  buf_size    Size of out_buf in bytes.
 * @param[out] bytes_read  Actual number of bytes read.
 *
 * @return ESP_OK on success, or an esp_err_t error code.
 */
esp_err_t sd_read(const char *filename, uint8_t *out_buf,
                  size_t buf_size, size_t *bytes_read);

/**
 * @brief Deleate all files form the SD card. Keep formating and dirs.
 * 
 * @return ESP_OK on success, or an esp_err_t error code.
 */
esp_err_t sd_wipe_files();

/**
 * @brief  Unmount the FAT filesystem and release the SD card SPI device.
 *         Called by hardware_deinit() in Initialize.cpp — do not call directly.
 */
void sd_unmount(void);

#ifdef __cplusplus
}
#endif

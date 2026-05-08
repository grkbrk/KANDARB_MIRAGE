#pragma once
 
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//This should be in initialize.cpp
// ---------------------------------------------------------------
// WIZ850io specific pins (W5500 chip)
// ---------------------------------------------------------------
#define WIZ_PIN_CS      10      // IO10 - CS0 (WIZ850io)
#define WIZ_PIN_RST     8       // IO8  - RST0 (Reset to WIZ850io)
 
// Socket to use for data transmission (0–7)
// Socket 0 gets the full 16KB TX + 16KB RX internal buffer
#define WIZ_SOCKET      0

// ---------------------------------------------------------------
// Dedicated socket for ICMP ping (separate from TCP socket 0)
// ---------------------------------------------------------------
#define WIZ_PING_SOCKET     1   // Uses socket 1, independent of TCP on socket 0
 
// ---------------------------------------------------------------
// Network configuration — update to match your network
// ---------------------------------------------------------------
#define WIZ_MAC         { 0x00, 0x08, 0xDC, 0x01, 0x02, 0x03 }
#define WIZ_IP          { 192, 168, 1, 100 }
#define WIZ_GATEWAY     { 192, 168, 1, 1   }
#define WIZ_SUBNET      { 255, 255, 255, 0 }
#define WIZ_DNS         { 8, 8, 8, 8       }
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 
#ifdef __cplusplus
extern "C" {
#endif
 
/**
 * @brief  Open a TCP connection to a remote host.
 *
 * @param[in] remote_ip    Remote IP as 4-byte array, e.g. {192,168,1,50}
 * @param[in] remote_port  Remote port number.
 *
 * @return ESP_OK on success, or an esp_err_t error code.
 */
esp_err_t wiz_connect(uint8_t *remote_ip, uint16_t remote_port);
 
/**
 * @brief  Send data over the open TCP connection.
 *         Data is written into the W5500's internal TX buffer and
 *         transmitted by the chip's hardwired TCP/IP stack.
 *
 * @param[in] data    Pointer to the buffer to send.
 * @param[in] length  Number of bytes to send.
 *
 * @return ESP_OK on success, or an esp_err_t error code.
 */
esp_err_t wiz_send(const uint8_t *data, size_t length);
 
/**
 * @brief  Receive data from the W5500's internal RX buffer.
 *
 * @param[out] buf         Buffer to write received data into.
 * @param[in]  buf_size    Size of buf in bytes.
 * @param[out] bytes_read  Actual number of bytes received.
 *
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if no data available,
 *         or another esp_err_t on error.
 */
esp_err_t wiz_receive(uint8_t *buf, size_t buf_size, size_t *bytes_read);

/**
 * @brief  Send an ICMP Echo Request and return IMMEDIATELY.
 *         Fire-and-forget — no reply is waited for or checked.
 *         Useful for watchdog-style "I am alive" signals where
 *         the receiver logs arrival but the sender does not block.
 *
 * @param[in] target_ip  IP to ping as 4-byte array, e.g. {192,168,1,1}
 * @param[in] message    Message to send with ping. Max 32 bytes.
 *
 * @return ESP_OK if the packet was handed to the W5500 TX buffer,
 *         or an esp_err_t on error.
 */
esp_err_t wiz_ping(uint8_t *target_ip, const char *message);
 
/**
 * @brief  Close the TCP connection and release the socket.
 */
void wiz_disconnect(void);

#ifdef __cplusplus
}
#endif
 
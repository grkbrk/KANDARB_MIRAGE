#include <string.h> //String and memory functions
#include <math.h> //Mathematical constants and functions
#include "freertos/FreeRTOS.h" //FreeRTOS functionality
#include "freertos/task.h" //FreeRTOS functionality
#include "driver/gpio.h" //Driver for GPIO pins
#include "driver/uart.h" //Driver for UART communication
#include "Settings.h" //Hardware definitions?
#include "read_sensors.h" //Data storage
#include "uart.h" //Initialization/configuration functions


//Turn sensor on
void K96_on()
{
    gpio_set_level(K96_EN_PIN, 1);
}

//Turn sensor off
void K96_off()
{
    gpio_set_level(K96_EN_PIN, 0);
}

// Compute a Modbus CRC checksum used for error checking and
// communication integrity

//data = Byte array to checksum
//length = Number of bytes
// uint16_t = 16-bit CRC value
static uint16_t modbus_crc16(
    const uint8_t *data,
    uint16_t length)
{
    // Initial  value
    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < length; i++)
    {
        // Byte into CRC
        crc ^= data[i];

        for (uint8_t j = 0; j < 8; j++)
        {
            // Check lowest bit
            if (crc & 0x0001)
            {
                // Apply Modbus polynomial
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    // Final CRC
    return crc;
}

// Reads one data point from K96 RAM by calling adress and number of bytes to read,
// returns true if successful
static bool K96_read_ram(
    uint16_t ram_address, // Address to read
    uint8_t num_bytes, // Number of bytes requested
    uint8_t *response) // Where the reply is stored
{
    // Communication packet
    uint8_t frame[7];

    // Device address (K96 default is 0x68)
    frame[0] = 0x68;

    // Function code (0x44 for reading RAM)
    frame[1] = 0x44;

    // RAM address
    frame[2] =
        (ram_address >> 8) & 0xFF; //High

    frame[3] =
        ram_address & 0xFF; // Low

    // Number of bytes to read
    frame[4] = num_bytes;

    // CRC16 checksum stored as low bytes first
    uint16_t crc =
        modbus_crc16(frame, 5);

    frame[5] =
        crc & 0xFF;

    frame[6] =
        (crc >> 8) & 0xFF;

    // Clear old UART RX data
    uart_flush(UART_PORT);

    // Send MODBUS frame
    // Address, Function, RAM high, RAM low, Length, CRC low, CRC high
    uart_write_bytes(
        UART_PORT,
        (const char *)frame,
        sizeof(frame));
    int len =

    // Read response
    // 0 byte = device address
    // 1 byte = function code
    // 2 byte = byte count
    // 3..N byte = data
    // Last 2 byte = CRC
        uart_read_bytes(
            UART_PORT,
            response,
            num_bytes + 5,
            pdMS_TO_TICKS(1000));


/* ---- MODIFICATION TO CRC CALCULATION -----
// Verify correct number of bytes
if (len != (num_bytes + 5))
{
    return false;
}

// CRC calculated, except for received CRC
uint16_t calculated_crc =
    modbus_crc16(response, len - 2);

// CRC received from sensor
uint16_t received_crc =
    response[len - 2] |
    (response[len - 1] << 8);

// Reports if data is corrupt
if (calculated_crc != received_crc)
{
    printf("CRC Error!\n");
    return false;
}

return true;
---------------- */

    // Return true if expected number of bytes were recieved
    // (But does not verify if the data is corrupted)

    return (len == (num_bytes + 5));
}

//Read all sensor values
void read_k96()
{
    uint8_t response[16];

    // CO2 concentration in ppm
    if (K96_read_ram(0x038C, 2, response))
    {
        // Stored as 16-bit integer
        int16_t raw =
            (response[3] << 8) |
            response[4];

        sensor_data.K96_CO2 =
            (float)raw;
    }
    else
    {
        //If reading is invalid
        sensor_data.K96_CO2 = NAN;
    }

    // Pressure
    if (K96_read_ram(0x01D0, 2, response))
    {
        int16_t raw =
            (response[3] << 8) |
            response[4];

        sensor_data.K96_pressure =
            raw * 0.1f; // hPa
    }
    else
    {
        sensor_data.K96_pressure = NAN;
    }

    // Humidity
    if (K96_read_ram(0x01F0, 2, response))
    {
        int16_t raw =
            (response[3] << 8) |
            response[4];

        sensor_data.K96_humidity =
            raw * 0.01f; // %
    }
    else
    {
        sensor_data.K96_humidity = NAN;
    }

    // Temperature
    if (K96_read_ram(0x01F8, 2, response))
    {
        int16_t raw =
            (response[3] << 8) |
            response[4];

        sensor_data.K96_temperature =
            raw * 0.01f; // °C
    }
    else
    {
        sensor_data.K96_temperature = NAN;
    }

    // Error status
    if (K96_read_ram(0x001C, 2, response))
    {
        // 0x0000 = no error
        sensor_data.K96_error =
            (response[3] << 8) |
            response[4];
    }
    else
    {
        sensor_data.K96_error = 0xFFFF;
    }
}

//To verify that the number being decoded is correct:
if (K96_read_ram(0x038C, 2, response))
{
    printf("Response: ");
    for(int i = 0; i < 7; i++)
    {
        printf("%02X ", response[i]);
    }
    printf("\n");

    int16_t raw = (response[3] << 8) | response[4];
    printf("Raw CO2 value = %d\n", raw);
}

//Check if data is missing by checking for NAN
if (isnan(sensor_data.K96_CO2))
{
    printf("CO2 data missing\n");
}
else
{
    printf("CO2 = %.0f ppm\n", sensor_data.K96_CO2);
}

//Check for suspicious values, range can be changed
if (!isnan(sensor_data.K96_CO2) &&
    sensor_data.K96_CO2 >= 0 &&
    sensor_data.K96_CO2 <= 10000)
{
    printf("Valid CO2 reading\n");
}
else
{
    printf("Invalid CO2 reading\n");
}

// Output: 68 44 02 01 F4 XX XX
// 68      Address
// 44      Function
// 02      Number of data bytes
// 01 F4   CO₂ = 500 ppm
// CRC CRC
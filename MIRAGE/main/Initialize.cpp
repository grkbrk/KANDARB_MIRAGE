#include "Settings.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "driver/uart.h"
#include "driver/spi_master.h"  
#include "Multiplexer.h"
#include "initialize.h"
#include "read_sensors.h"
#include <stdio.h>

//Initialize pins
void init_gpio_pins()
{
    gpio_config_t io_conf;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask =
        (1ULL << Watchdog_PIN) |
        (1ULL << Thermal_reset_PIN) |
        (1ULL << Preassure_reset_PIN) |
        (1ULL << K96_EN_PIN) |
        (1ULL << Neo_PIN) |
        (1ULL << Reset_WIZ_PIN) |
        (1ULL << CS_SD_PIN) |
        (1ULL << CS_WIZ_PIN);

    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

    gpio_config(&io_conf);

    // Safe startup states
    gpio_set_level(Watchdog_PIN, 0);        // Pulse to start

    gpio_set_level(Thermal_reset_PIN, 1);   // High for normal operations
    gpio_set_level(Preassure_reset_PIN, 1); // High for normal operations
    gpio_set_level(Reset_WIZ_PIN, 1);       // High for normal operations

    gpio_set_level(K96_EN_PIN, 0);          // Starts as passive, set to 1 to activate

    gpio_set_level(Neo_PIN, 0);             // Start as off

    gpio_set_level(CS_SD_PIN, 1);           // Low to listen/respond, High to ignore
    gpio_set_level(CS_WIZ_PIN, 1);          // Low to listen/respond, High to ignore
}

//Initialize SPI
spi_device_handle_t SD_handle;
spi_device_handle_t WIZ_handle;

void init_spi()
{
    // configure bus
    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = SPI_MOSI_PIN;
    buscfg.miso_io_num = SPI_MISO_PIN;
    buscfg.sclk_io_num = SPI_clk_PIN;
    buscfg.quadwp_io_num = -1;              // Disabling quadpins
    buscfg.quadhd_io_num = -1;              // Disabling quadpins
    

    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

    // Add SD-card 
    spi_device_interface_config_t SD_cfg = {};
    SD_cfg.clock_speed_hz = SD_clk_spd_hz;
    SD_cfg.mode = 0;                            // SPI mode 0
    SD_cfg.spics_io_num = CS_SD_PIN;            // CS pin for SD card
    SD_cfg.queue_size = SD_queue_size;

    spi_bus_add_device(SPI2_HOST, &SD_cfg, &SD_handle);


    // Add Ethernet
    spi_device_interface_config_t WIZ_cfg = {};
    WIZ_cfg.clock_speed_hz = WIZ_clk_spd_hz;
    WIZ_cfg.mode = 0;                            // SPI mode 0
    WIZ_cfg.spics_io_num = CS_WIZ_PIN;           // CS pin for Ethernet
    WIZ_cfg.queue_size = ethernet_queue_size;

    spi_bus_add_device(SPI2_HOST, &WIZ_cfg, &WIZ_handle);
}

//initialize I2c
void init_i2c()
{
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_SDA;
    conf.scl_io_num = I2C_SCL;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_FREQ_HZ;

    i2c_param_config(I2C_master, &conf);
    i2c_driver_install(I2C_master, conf.mode, 0, 0, 0);
}

//Uart initialize
void init_uart()
{
    uart_config_t uart_config = {};

    uart_config.baud_rate = 115200;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity    = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_2;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.source_clk = UART_SCLK_APB;

    // Apply UART configuration
    uart_param_config(UART_PORT, &uart_config);

    // Set UART pins
    uart_set_pin(
        UART_PORT,
        K96_TX_PIN,
        K96_RX_PIN,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    );

    // Install UART driver
    uart_driver_install(
        UART_PORT,
        1024,   // RX buffer
        0,      // TX buffer
        0,      // Event queue size
        NULL,
        0
    );
}

// Starting sensors that needs to be initialized
static void init_sht45_sensor(uint8_t channel)
{
    select_mux_channel(channel);

    uint8_t cmd[2] = {0x27, 0x37};

    i2c_master_write_to_device(
        I2C_master,
        SHT45_addr,
        cmd,
        2,
        100 / portTICK_PERIOD_MS
    );
}

// initioalize MS5803 sensors
static void init_ms5803(uint8_t channel, MS5803_Calibration* cal)
{
    select_mux_channel(channel);

    uint8_t data[2];

    for (uint8_t i = 0; i < 6; i++)
    {
        uint8_t cmd = 0xA2 + (i * 2);

        i2c_master_write_read_device(
            I2C_master,
            MS5803_addr,
            &cmd,
            1,
            data,
            2,
            100 / portTICK_PERIOD_MS
        );

        cal->C[i + 1] =
            (data[0] << 8) | data[1];
    }
}

void init_sensors()
{
    init_sht45_sensor(multiplex_Ambient);
    init_sht45_sensor(multiplex_Tp4_Pp1_Tp5_Pp2);
    init_sht45_sensor(multiplex_Outlet_SD);
    init_sht45_sensor(multiplex_Tt3_devP);
    init_ms5803(multiplex_Ambient, &pa1_cal);
    init_ms5803(multiplex_Tp4_Pp1_Tp5_Pp2, &pp2_cal);
}
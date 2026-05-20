#pragma once
#include <stdint.h>
#include <stdbool.h>

// Slave devices
typedef enum
{
    thermal_mcu,
    pressure_mcu

} SlaveDevice;

// Packet types
typedef enum
{
    THERMAL_PACKET_DATA     = 0x01,
    THERMAL_PACKET_COMMAND  = 0x02,
    THERMAL_PACKET_SETTING  = 0x03

} PacketType;

// Telemetry/Data IDs
typedef enum
{
    // Vacuum pump temperatures
    Tp1                 = 0x01,
    Tp2                 = 0x02,

    // Compressor temperature
    Tp3                 = 0x03,

    // Pipe pump/pump
    Tp6                 = 0x04,
    Pp3                 = 0x05,

    // Pipe pump/compressor
    Tp4                 = 0x06,
    Pp1                 = 0x07,

    // Ambient
    Pa1                 = 0x08,
    Ta1                 = 0x09,
    Ta2                 = 0x0A,
    Ha1                 = 0x0B,

    // Measurement chamber
    Tp5                 = 0x0C,
    Pp2                 = 0x0D,

    // Outlet + inlet
    Tt1                 = 0x0E,
    Tt2           = 0x0F,
    Tt3                = 0x10,
} SlaveData;

// Commands
typedef enum
{
    Pumps_off       = 0x01,
    Pumps_on       = 0x02,
    Open_shutters   = 0x03,
    Close_shutters  = 0x04,
    Standby         = 0x05,
    Meassurments    = 0x06,
    Heater_on        = 0x07,
    Heater_off       = 0x08

} SlaveCommand;

// Settings
typedef enum
{
    Set_chamber_temp      = 0x01,
    Set_chamber_preassure  = 0x02

} SlaveSetting;

// Slave MCU status
typedef struct
{
    bool online;
    uint8_t state;
    uint8_t error;
} SlaveStatus;

// API
bool slave_send_data(
    SlaveDevice slave,
    uint8_t data_id,
    float value
);

bool slave_send_command(
    SlaveDevice slave,
    uint8_t command
);

bool slave_update_setting(
    SlaveDevice slave,
    uint8_t setting,
    float value
);

bool slave_read_status(
    SlaveDevice slave,
    SlaveStatus* status
);

void slave_reset(
    SlaveDevice slave
);
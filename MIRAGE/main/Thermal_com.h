#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "read_sensors.h"

// Packet types
typedef enum
{
    THERMAL_PACKET_DATA     = 0x01,
    THERMAL_PACKET_COMMAND  = 0x02,
    THERMAL_PACKET_SETTING  = 0x03

} ThermalPacketType;

// Commands
typedef enum
{
    THERMAL_CMD_Meassurment      = 0x01
    THERMAL_CMD_Stndby           = 0x02

} ThermalCommand;

// Settings
typedef enum
{
    THERMAL_SET_TARGET_TEMP     = 0x01,
    THERMAL_SET_TARGET_PRESSURE = 0x02

} ThermalSetting;

// Thermal MCU status
typedef struct
{
    bool online;

    uint8_t state;

    uint8_t error;

    float internal_temperature;

} ThermalStatus;

// API
bool thermal_send_sensor_data(
    const SensorData* data
);

bool thermal_send_command(
    ThermalCommand cmd
);

bool thermal_update_setting(
    ThermalSetting setting,
    float value
);

bool thermal_read_status(
    ThermalStatus* status
);

void thermal_reset();
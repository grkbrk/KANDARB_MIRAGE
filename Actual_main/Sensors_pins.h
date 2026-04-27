#pragma once
#include "driver/gpio.h"
#include <stdint.h>

// PINS definition
#define PDA_relay_2 GPIO_NUM_1
#define PDA_relay_3 GPIO_NUM_4
#define PDA_relay_4 GPIO_NUM_5
#define PDA_relay_1 GPIO_NUM_28
#define I2C_SDA GPIO_NUM_10
#define I2C_SCL GPIO_NUM_11
#define PDB_current_1 GPIO_NUM_6
#define PDB_current_2 GPIO_NUM_7
#define PDB_current_3 GPIO_NUM_8
#define PDB_current_4 GPIO_NUM_9
#define Shutters GPIO_NUM_24
#define Compressor GPIO_NUM_25
#define PWM_pump_1 GPIO_NUM_29
#define PWM_pump_2 GPIO_NUM_30

//I2c adresses
#define Multiplexer_address 0x70 
#define SHT3x_1_adress 0x44
#define SHT3x_2_adress 0x44


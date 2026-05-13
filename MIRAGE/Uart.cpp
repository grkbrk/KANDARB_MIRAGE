#include "Uart.h"
#include "Settings.h"

void K96_ON()
{
    gpio_set_level(K96_EN_PIN, 1);
}

void K96_OFF()
{
    gpio_set_level(K96_EN_PIN, 0);
}
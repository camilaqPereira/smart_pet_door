#ifndef SERVOMOTOR_H
#define SERVOMOTOR_H

#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define SERVOMOTOR_PIN 18   // GPIO pin for the servomotor



/* Function prototypes*/
uint servomotor_setup();
void servomotor_set_position(uint slice, uint16_t active_time);

#endif
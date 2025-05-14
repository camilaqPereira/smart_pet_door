#ifndef HC_SR04_H
#define HC_SR04_H


#include "pico/stdlib.h"


#define TRIG_PIN 18  // GPIO pin for trigger
#define ECHO_PIN 19  // GPIO pin for echo

void hcsr04_init();
void hcsr04_send_trig_pulse();











#endif //HC_SR04_H
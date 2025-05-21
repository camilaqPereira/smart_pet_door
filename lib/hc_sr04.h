#ifndef HC_SR04_H
#define HC_SR04_H


#include "pico/stdlib.h"


#define TRIG_PIN 19  // GPIO pin for trigger
#define ECHO_PIN 16  // GPIO pin for echo
#define HCSR04_TIMEOUT_US 30000 // Timeout for echo pulse in microseconds


void hcsr04_init();
void hcsr04_send_trig_pulse();
int64_t hcsr04_get_echo_duration();
float hcsr04_calculate_distance(int64_t echo_duration);











#endif //HC_SR04_H
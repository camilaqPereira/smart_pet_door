#include "lib/servomotor.h"

const float PWM_clock_div = 50.0;   // PWM clock divisor
const uint16_t PWM_wrap = 50000;    // PWM wrap
const uint16_t increment_top_limit = 5990;  // Largest active time in sequence

uint servomotor_setup(){
    uint slice;
    gpio_set_function(SERVOMOTOR_PIN, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(SERVOMOTOR_PIN);

    pwm_set_clkdiv(slice, PWM_clock_div);
    pwm_set_wrap(slice, PWM_wrap);
    pwm_set_gpio_level(SERVOMOTOR_PIN, 0);
    
    pwm_set_enabled(slice, true);
    
    return slice;
}

void servomotor_set_position(uint slice, uint16_t active_time){
    pwm_set_gpio_level(SERVOMOTOR_PIN, active_time);
};


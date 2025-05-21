#include "lib/hc_sr04.h"



void hcsr04_init() {
    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_put(TRIG_PIN, 0);

    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
}

void hcsr04_send_trig_pulse() {
    gpio_put(TRIG_PIN, 1);
    sleep_us(10); // 10 us pulse
    gpio_put(TRIG_PIN, 0);
}


int64_t hcsr04_get_echo_duration() {

    hcsr04_send_trig_pulse(); // Send trigger pulse
    absolute_time_t timeout_time = make_timeout_time_us(HCSR04_TIMEOUT_US);

    // Wait for echo pulse to begin, with timeout
    while (gpio_get(ECHO_PIN) == 0) {
        if (absolute_time_diff_us(get_absolute_time(), timeout_time) < 0) {
            // Timed out waiting for echo start
            return -1;
        }
    }

    absolute_time_t start_time = get_absolute_time();
    timeout_time = make_timeout_time_us(HCSR04_TIMEOUT_US);

    // Wait for echo pulse to end, with timeout
    while (gpio_get(ECHO_PIN) == 1) {
        if (absolute_time_diff_us(get_absolute_time(), timeout_time) < 0) {
            // Timed out waiting for echo end
            return -1;
        }
    }
    
    absolute_time_t end_time = get_absolute_time();

    // Calculate the duration of the echo pulse in us
    int64_t echo_duration = absolute_time_diff_us(start_time, end_time);

    return echo_duration;
}

float hcsr04_calculate_distance(int64_t echo_duration) {
    float distance_cm = (echo_duration * 0.0343) / 2.0;
    return distance_cm;

}



#ifndef BUTTONS_H
#define BUTTONS_H

#include "pico/stdlib.h"

void setup_buttons();
int64_t debounce_timer_callback(alarm_id_t id, void *user_data);
void connection_state_change(uint gpio, uint32_t events);

#endif
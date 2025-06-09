#include "inc/buttons.h"
#include "inc/wifi.h"
#include "inc/display.h"

static volatile bool button_pressed = false; // Flag to prevent multiple button presses
alarm_id_t debounce_alarm_id; // Alarm ID for the debounce timer


void setup_buttons()
{
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &connection_state_change);
}

int64_t debounce_timer_callback(alarm_id_t id, void *user_data)
{

    if (gpio_get(BUTTON_PIN) == 0)
    {
        printf("Botão pressionado!\n");
    }
    button_pressed = false;
    return 0;
}

void connection_state_change(uint gpio, uint32_t events)
{

    if (!button_pressed)
    {
        button_pressed = true;
        toggle_wifi = true;
        debounce_alarm_id = add_alarm_in_ms(DEBOUNCE_DELAY_MS, debounce_timer_callback, NULL, false);
    }
}
#include "inc/buttons.h"
#include "inc/wifi.h"
#include "inc/display.h"

static volatile bool button_pressed = false; // Flag to prevent multiple button presses
alarm_id_t debounce_alarm_id; // Alarm ID for the debounce timer

/**
 * @brief Initializes the button GPIO pin and sets up an interrupt for button presses.
 * 
 * This function configures the GPIO pin connected to the button as an input,
 * enables a pull-up resistor, and sets up an interrupt to handle button presses.
 * The interrupt will trigger on the falling edge (button press) and call the
 * `connection_state_change` function to handle the button press event.
 */

void setup_buttons()
{
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &connection_state_change);
}

/**
 * @brief Callback function for the debounce timer.
 * 
 * This function is called when the debounce timer expires. It checks if the button
 * is still pressed and prints a message if it is. It also resets the button_pressed flag.
 * 
 * @param id The ID of the alarm that triggered this callback.
 * @param user_data User data passed to the callback (not used here).
 * 
 * @return Returns 0 to indicate successful completion.
 */

int64_t debounce_timer_callback(alarm_id_t id, void *user_data)
{

    if (gpio_get(BUTTON_PIN) == 0)
    {
        printf("Botão pressionado!\n");
    }
    button_pressed = false;
    return 0;
}

/**
 * @brief Handles the button press event and sets the toggle_wifi flag.
 * 
 * This function is called when the button is pressed. It checks if the button has
 * already been pressed to prevent multiple triggers. If not, it sets the toggle_wifi
 * flag to true and starts a debounce timer to avoid multiple rapid presses.
 * 
 * @param gpio The GPIO pin number that triggered the interrupt (not used here).
 * @param events The events that triggered the interrupt (not used here).
 */

void connection_state_change(uint gpio, uint32_t events)
{

    if (!button_pressed)
    {
        button_pressed = true;
        toggle_wifi = true;
        debounce_alarm_id = add_alarm_in_ms(DEBOUNCE_DELAY_MS, debounce_timer_callback, NULL, false);
    }
}
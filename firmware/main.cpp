#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/bootrom.h"
#include "tusb.h"

#define BTN_LAYER 0
#define BTN_1     1
#define BTN_2     2
#define BTN_3     3
#define NUM_BUTTONS 4
#define DEBOUNCE_MS 20
#define RESET_HOLD_MS 2000

static const uint8_t button_pins[NUM_BUTTONS] = {BTN_LAYER, BTN_1, BTN_2, BTN_3};

// HID keycodes per layer for buttons 1-3
// Layer 0: Up, Left, F12
// Layer 1: Down, Right, Enter
static const uint8_t key_codes[2][3] = {
    {HID_KEY_ARROW_UP,   HID_KEY_ARROW_LEFT,  HID_KEY_F12},
    {HID_KEY_ARROW_DOWN, HID_KEY_ARROW_RIGHT, HID_KEY_ENTER},
};

static const char* key_names[2][3] = {
    {"Up",    "Left",  "F12"},
    {"Down",  "Right", "Enter"},
};

static bool button_state[NUM_BUTTONS] = {};
static uint32_t last_change[NUM_BUTTONS] = {};

static int layer = 0;
static uint32_t layer_press_time = 0;
static bool reset_triggered = false;

void init_buttons() {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        gpio_init(button_pins[i]);
        gpio_set_dir(button_pins[i], GPIO_IN);
        gpio_pull_up(button_pins[i]);
    }
}

void send_keyboard_report() {
    if (!tud_hid_ready()) return;

    uint8_t keycodes[6] = {0};
    int idx = 0;

    for (int i = 1; i < NUM_BUTTONS && idx < 6; i++) {
        if (button_state[i]) {
            keycodes[idx++] = key_codes[layer][i - 1];
        }
    }

    tud_hid_keyboard_report(0, 0, keycodes);
}

// Required TinyUSB callbacks
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                                hid_report_type_t report_type,
                                uint8_t* buffer, uint16_t reqlen) {
    (void)instance; (void)report_id; (void)report_type;
    (void)buffer; (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                            hid_report_type_t report_type,
                            uint8_t const* buffer, uint16_t bufsize) {
    (void)instance; (void)report_id; (void)report_type;
    (void)buffer; (void)bufsize;
}

int main() {
    stdio_init_all();
    tusb_init();

    if (cyw43_arch_init()) {
        return 1;
    }

    init_buttons();
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);

    printf("\n--- Pico HID Keyboard ---\n");
    printf("Layer: %d\n\n", layer);

    while (true) {
        tud_task();

        uint32_t now = to_ms_since_boot(get_absolute_time());
        bool state_changed = false;

        for (int i = 0; i < NUM_BUTTONS; i++) {
            bool raw = !gpio_get(button_pins[i]);

            if (raw != button_state[i] && (now - last_change[i]) > DEBOUNCE_MS) {
                button_state[i] = raw;
                last_change[i] = now;
                state_changed = true;

                if (i == 0) {
                    if (raw) {
                        layer_press_time = now;
                        reset_triggered = false;
                    } else if (!reset_triggered) {
                        layer = 1 - layer;
                        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, layer);
                        printf("[LAYER]   Switched to Layer %d\n", layer);
                    }
                } else {
                    int key_idx = i - 1;
                    if (raw) {
                        printf("[PRESS]   %s (Layer %d)\n", key_names[layer][key_idx], layer);
                    } else {
                        printf("[RELEASE] %s (Layer %d)\n", key_names[layer][key_idx], layer);
                    }
                }
            }
        }

        // Long hold layer button → reset
        if (button_state[0] && !reset_triggered) {
            if ((now - layer_press_time) >= RESET_HOLD_MS) {
                reset_triggered = true;
                printf("[FLASH]   Entering bootloader...\n");
                sleep_ms(100);
                reset_usb_boot(0, 0);
            }
        }

        // Send HID report on state change
        if (state_changed) {
            send_keyboard_report();
        }

        sleep_ms(1);
    }

    return 0;
}

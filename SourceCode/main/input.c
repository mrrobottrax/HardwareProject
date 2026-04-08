#include "input.h"

#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>

// https://www.sunrom.com/p/3x4-soft-touch-matrix-keypad-module

// 13 14 27 26 25 33 32

#define NUM_ROWS (sizeof(ROW_PINS) / sizeof(ROW_PINS[0]))
#define NUM_COLS (sizeof(COL_PINS) / sizeof(COL_PINS[0]))

const int ROW_PINS[] = {14, 32, 33, 26};
const int COL_PINS[] = {27, 13, 25};

esp_err_t input_init()
{
    for (int i = 0; i < NUM_ROWS; i++)
    {
        gpio_reset_pin(ROW_PINS[i]);
        gpio_set_direction(ROW_PINS[i], GPIO_MODE_OUTPUT_OD);
        gpio_set_level(ROW_PINS[i], 1);
    }

    for (int i = 0; i < NUM_COLS; i++)
    {
        gpio_reset_pin(COL_PINS[i]);
        gpio_set_direction(COL_PINS[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(COL_PINS[i], GPIO_PULLUP_ONLY);
    }

    return ESP_OK;
}

input_data_t input_read()
{
    input_data_t input = {0};

    for (int row = 0; row < NUM_ROWS; row++)
    {
        gpio_set_level(ROW_PINS[row], 0);

        esp_rom_delay_us(50);

        for (int col = 0; col < NUM_COLS; col++)
        {
            if (gpio_get_level(COL_PINS[col]) == 0)
            {
                input.keypad[row * NUM_COLS + col] = true;
            }
        }

        gpio_set_level(ROW_PINS[row], 1);
    }

    return input;
}
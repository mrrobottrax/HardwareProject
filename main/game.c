#include "game.h"

#include <freertos/FreeRTOS.h>

#include "display.h"
#include "input.h"

void game_logic_task(void *pvParams)
{
    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t tick_rate = pdMS_TO_TICKS(33); // 30 FPS

    while (1)
    {
        input_data_t input = input_read();

        // 2. Game Logic (Lightning fast, no hardware delays!)
        // if (current_inputs.button_up)
        // {
        //     player_y -= 1;
        // }

        ESP_ERROR_CHECK(display_clear());
        for (int i = 0; i < sizeof(input.keypad) / sizeof(input.keypad[0]); ++i)
            if (input.keypad[i])
                ESP_ERROR_CHECK(display_write_data(49 + i));

        vTaskDelayUntil(&last_wake_time, tick_rate);
    }
}
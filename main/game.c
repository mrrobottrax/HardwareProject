#include "game.h"

#include "display.h"
#include <freertos/FreeRTOS.h>

void game_logic_task(void *pvParams)
{
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t tick_rate = pdMS_TO_TICKS(33); // 30 FPS

    while (1)
    {
        // GameInputState_t current_inputs;

        // 1. Grab a snapshot of the latest inputs
        // if (xSemaphoreTake(input_mutex, portMAX_DELAY) == pdTRUE)
        // {
        //     // Struct assignment copies all values at once
        //     current_inputs = global_input_state;
        //     xSemaphoreGive(input_mutex);
        // }

        // 2. Game Logic (Lightning fast, no hardware delays!)
        // if (current_inputs.button_up)
        // {
        //     player_y -= 1;
        // }

        // 3. Send render commands to the display queue (from previous steps)
        // ... xQueueSend(display_cmd_queue, &cmd, 0); ...

        ESP_ERROR_CHECK(display_clear());

        ESP_ERROR_CHECK(display_set_dd_address(0));
        ESP_ERROR_CHECK(display_write_data('A'));
        ESP_ERROR_CHECK(display_write_data(0));
        ESP_ERROR_CHECK(display_write_data(0));
        ESP_ERROR_CHECK(display_write_data(0));

        for (int i = 0; i < 100; ++i)
            for (int a = 0; a < 6; ++a)
            {
                int h = a;
                if (a > 3)
                {
                    h = 6 - a;
                }

                ESP_ERROR_CHECK(display_set_cg_address(0));
                for (int z = 0; z < 3 - h; ++z)
                    ESP_ERROR_CHECK(display_write_data(0));
                ESP_ERROR_CHECK(display_write_data(0b11111));
                ESP_ERROR_CHECK(display_write_data(0b10001));
                ESP_ERROR_CHECK(display_write_data(0b10001));
                ESP_ERROR_CHECK(display_write_data(0b10001));
                ESP_ERROR_CHECK(display_write_data(0b11111));
                for (int z = 0; z < h; ++z)
                    ESP_ERROR_CHECK(display_write_data(0));

                vTaskDelay(10);
            }

        // 4. Wait for the next frame
        vTaskDelayUntil(&last_wake_time, tick_rate);
    }
}
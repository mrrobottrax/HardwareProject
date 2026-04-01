#include "game.h"

#include <string.h>
#include <freertos/FreeRTOS.h>

#include "display.h"
#include "input.h"

void game_logic_task(void *pvParams)
{
}

void space_game_task(void *pvParams)
{
    display_clear();

    enum
    {
        SPRITE_PLAYER,
        SPRITE_ASTEROID,
        SPRITE_EXPLOSION0,
        SPRITE_EXPLOSION1,
    };

    // load player
    display_set_cg_address(SPRITE_PLAYER);
    display_write_data(0);
    display_write_data(0);
    display_write_data(0);
    display_write_data(0b11100);
    display_write_data(0b01110);
    display_write_data(0b11100);
    display_write_data(0);
    display_write_data(0);

    // load asteroid
    display_set_cg_address(SPRITE_ASTEROID * 8);
    display_write_data(0);
    display_write_data(0);
    display_write_data(0b01110);
    display_write_data(0b10111);
    display_write_data(0b11111);
    display_write_data(0b11011);
    display_write_data(0b01110);
    display_write_data(0);

    // load explosion
    display_set_cg_address(SPRITE_EXPLOSION0 * 8);
    display_write_data(0);
    display_write_data(0b01100);
    display_write_data(0b00110);
    display_write_data(0b01111);
    display_write_data(0b11110);
    display_write_data(0b00110);
    display_write_data(0b00100);
    display_write_data(0);

    display_set_cg_address(SPRITE_EXPLOSION1 * 8);
    display_write_data(0);
    display_write_data(0b00100);
    display_write_data(0b10001);
    display_write_data(0b01010);
    display_write_data(0b01100);
    display_write_data(0b10001);
    display_write_data(0b01000);
    display_write_data(0);

    bool player_up = false;

    uint8_t row0[16];
    uint8_t row1[16];

    memset(row0, ' ', 16);
    memset(row1, ' ', 16);

    const TickType_t tick_rate = pdMS_TO_TICKS(300);
    TickType_t last_wake_time = xTaskGetTickCount();

    while (1)
    {
        bool was_player_up = player_up;
        input_data_t input = input_read();

        // copy screen
        uint8_t prev_row0[16];
        uint8_t prev_row1[16];

        memcpy(prev_row0, row0, 16);
        memcpy(prev_row1, row1, 16);

        // scroll rows
        for (int i = 0; i < 14; ++i)
            row0[i] = row0[i + 1];

        for (int i = 0; i < 14; ++i)
            row1[i] = row1[i + 1];

        row0[15] = '2';
        row1[15] = '8';

        // add new asteroids
        bool spawned = false;
        if (((float)rand() / RAND_MAX) < 0.5f)
        {
            bool up = ((float)rand() / RAND_MAX) < 0.5f;
            if (up)
            {
                // check if clear
                if (row1[13] == ' ')
                {
                    row0[14] = SPRITE_ASTEROID;
                    row1[14] = ' ';
                    spawned = true;
                }
            }
            else
            {
                // check if clear
                if (row0[13] == ' ')
                {
                    row0[14] = ' ';
                    row1[14] = SPRITE_ASTEROID;
                    spawned = true;
                }
            }
        }

        if (!spawned)
        {
            row0[14] = ' ';
            row1[14] = ' ';
        }

        // player move
        if (input.keypad[1])
        {
            player_up = true;
        }

        if (input.keypad[7])
        {
            player_up = false;
        }

        // draw dirty chars
        {
            uint8_t addr = 255;
            for (int i = 0; i < 16; ++i)
            {
                if (prev_row0[i] != row0[i])
                {
                    if (addr != i)
                    {
                        display_set_dd_address(i);
                        addr = i;
                    }
                    display_write_data(row0[i]);
                }
            }

            for (int i = 0; i < 16; ++i)
            {
                if (prev_row1[i] != row1[i])
                {
                    if (addr != 64 + i)
                    {
                        display_set_dd_address(64 + i);
                        addr = 64 + i;
                    }
                    display_write_data(row1[i]);
                }
            }
        }

        // draw player
        if (player_up)
        {
            display_set_dd_address(1);
            display_write_data(SPRITE_PLAYER);
            if (!was_player_up)
            {
                display_set_dd_address(65);
                display_write_data(row1[1]);
            }
        }
        else
        {
            display_set_dd_address(65);
            display_write_data(SPRITE_PLAYER);
            if (was_player_up)
            {
                display_set_dd_address(1);
                display_write_data(row0[1]);
            }
        }

        // check lose
        if ((player_up && row0[1] == SPRITE_ASTEROID) ||
            (!player_up && row1[1] == SPRITE_ASTEROID))
        {
            if (player_up)
            {
                display_set_dd_address(1);
            }
            else
            {
                display_set_dd_address(65);
            }
            display_write_data(SPRITE_EXPLOSION0);

            vTaskDelay(100 / portTICK_PERIOD_MS);

            display_shift_cursor_l();
            display_write_data(SPRITE_EXPLOSION1);

            vTaskDelay(200 / portTICK_PERIOD_MS);

            display_shift_cursor_l();
            display_write_data(' ');

            vTaskDelay(400 / portTICK_PERIOD_MS);

            display_clear();
            display_set_dd_address(4);

            const char string[] = "You Lose";

            for (int i = 0; i < sizeof(string) - 1; ++i)
                display_write_data(string[i]);

            vTaskDelay(2000 / portTICK_PERIOD_MS);

            TaskHandle_t game_logic_task_handle;
            if (xTaskCreatePinnedToCore(space_game_task, "Game Logic", 4096, NULL, 2, &game_logic_task_handle, 1) != pdPASS)
                ESP_ERROR_CHECK(ESP_FAIL);

            vTaskDelete(NULL);
        }

        vTaskDelayUntil(&last_wake_time, tick_rate);
    }
}

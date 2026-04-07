#include "space_game.h"

#include <string.h>
#include <freertos/FreeRTOS.h>

#include "display.h"
#include "input.h"
#include "accelerometer.h"
#include "audio.h"
#include "sounds.h"
#include "meta_logic.h"

void space_game_task(void *pvParams)
{
    audio_playfile(SOUND_FOLDER_SPACE, SOUND_SPACE_MENU);

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

    // intro instructions
    display_set_dd_address(0);
    display_write_string("2 = MOVE UP");
    display_set_dd_address(64);
    display_write_string("0 = MOVE DOWN");
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    display_clear();
    display_set_dd_address(0);
    display_write_string("AVOID THE METEORS");
    display_set_dd_address(64);
    display_write_string("LAST 30 SECONDS");
    vTaskDelay(4000 / portTICK_PERIOD_MS);

    display_clear();

    const TickType_t tick_rate = pdMS_TO_TICKS(10);
    const TickType_t tick_ratio = 30;
    TickType_t last_wake_time = xTaskGetTickCount();
    TickType_t win_time = xTaskGetTickCount() + pdMS_TO_TICKS(30 * 1000);

    TickType_t current_tick = 0;
    input_data_t input = {0};
    while (1)
    {
        input_data_t input_currentframe = input_read();

        for (int i = 0; i < 12; ++i)
            input.keypad[i] |= input_currentframe.keypad[i];

        if (current_tick++ != tick_ratio)
        {
            vTaskDelayUntil(&last_wake_time, tick_rate);
            continue;
        }

        // check win
        if (xTaskGetTickCount() > win_time)
        {
            vTaskDelay(500 / portTICK_PERIOD_MS);
            win_game();
        }

        current_tick = 0;

        bool was_player_up = player_up;

        // copy screen
        uint8_t prev_row0[16];
        uint8_t prev_row1[16];

        memcpy(prev_row0, row0, 16);
        memcpy(prev_row1, row1, 16);

        // scroll rows
        for (int i = 1; i < 15; ++i)
            row0[i] = row0[i + 1];

        for (int i = 1; i < 15; ++i)
            row1[i] = row1[i + 1];

        row0[0] = '2';
        row1[0] = '0';

        // add new asteroids
        bool spawned = false;
        if (((float)rand() / RAND_MAX) < 0.5f)
        {
            bool up = ((float)rand() / RAND_MAX) < 0.5f;
            if (up)
            {
                // check if clear
                if (row1[14] == ' ')
                {
                    row0[15] = SPRITE_ASTEROID;
                    row1[15] = ' ';
                    spawned = true;
                }
            }
            else
            {
                // check if clear
                if (row0[14] == ' ')
                {
                    row0[15] = ' ';
                    row1[15] = SPRITE_ASTEROID;
                    spawned = true;
                }
            }
        }

        if (!spawned)
        {
            row0[15] = ' ';
            row1[15] = ' ';
        }

        // player move
        if (input.keypad[1])
        {
            audio_playfile(SOUND_FOLDER_SPACE, SOUND_SPACE_UP);
            player_up = true;
        }

        if (input.keypad[7] || input.keypad[10])
        {
            audio_playfile(SOUND_FOLDER_SPACE, SOUND_SPACE_DOWN);
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
            display_set_dd_address(2);
            display_write_data(SPRITE_PLAYER);
            if (!was_player_up)
            {
                display_set_dd_address(66);
                display_write_data(row1[2]);
            }
        }
        else
        {
            display_set_dd_address(66);
            display_write_data(SPRITE_PLAYER);
            if (was_player_up)
            {
                display_set_dd_address(2);
                display_write_data(row0[2]);
            }
        }

        // check lose
        if ((player_up && row0[2] == SPRITE_ASTEROID) ||
            (!player_up && row1[2] == SPRITE_ASTEROID))
        {
            audio_playfile(SOUND_FOLDER_SPACE, SOUND_SPACE_CRASH);

            if (player_up)
            {
                display_set_dd_address(2);
            }
            else
            {
                display_set_dd_address(66);
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

            lose_game();
        }

        for (int i = 0; i < 12; ++i)
            input.keypad[i] = 0;

        vTaskDelayUntil(&last_wake_time, tick_rate);
    }
}

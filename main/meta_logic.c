#include "meta_logic.h"

#include <freertos/FreeRTOS.h>

#include "display.h"
#include "input.h"
#include "audio.h"
#include "sounds.h"

#include "space_game.h"
#include "shake_game.h"
#include "simon_game.h"

static int lives = 3;
static int current_game = 0;
static void (*games_list[])(void *) = {space_game_task, shake_game_task, simon_game_task};

static bool did_intro = false;

void meta_logic_task(void *pvParams)
{
    // intro sequence
    if (!did_intro)
    {
        did_intro = true;

        display_clear();

        display_set_dd_address(0);
        display_write_string("PRESS 5 TO START");

        while (true)
        {
            input_data_t input = input_read();
            if (input.keypad[4])
                break;
            vTaskDelay(1);
        }

        audio_playfile(SOUND_FOLDER_META, SOUND_META_ARMED);

        display_set_dd_address(0);
        display_write_string("DEVICE HAS BEEN ");
        display_set_dd_address(64 + 5);
        display_write_string("ARMED");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

    // check for death
    if (lives <= 0)
    {
        audio_playfile(SOUND_FOLDER_META, SOUND_META_FAIL);

        display_clear();
        display_set_dd_address(3);
        display_write_string("YOU FAILED");
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        display_set_dd_address(0);
        display_write_string("DETONATION IN ");

        for (int i = 5; i >= 0; --i)
        {
            audio_playfile(SOUND_FOLDER_META, SOUND_META_BEEP0);
            display_set_dd_address(14);
            display_write_data('0' + i);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        display_clear();
        audio_playfile(SOUND_FOLDER_META, SOUND_META_EXPLODE);

        vTaskDelay(5000 / portTICK_PERIOD_MS);
        esp_restart();
    }

    // check for win
    if (current_game >= sizeof(games_list) / sizeof(games_list[0]))
    {
        audio_playfile(SOUND_FOLDER_META, SOUND_META_DEFUSE);

        display_clear();
        display_set_dd_address(1);
        display_write_string("DEVICE DEFUSED");
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        audio_playfile(SOUND_FOLDER_META, SOUND_META_WIN);

        display_clear();
        display_set_dd_address(4);
        display_write_string("YOU WIN!");

        vTaskDelay(5000 / portTICK_PERIOD_MS);
        esp_restart();
    }

    // lives update
    for (int i = 0; i < 3; ++i)
    {
        audio_playfile(SOUND_FOLDER_META, SOUND_META_BEEP0);
        display_clear();
        if (lives > 1)
        {
            display_set_dd_address(3);
            display_write_data('0' + lives);
            display_write_string(" ATTEMPTS");
            display_set_dd_address(64 + 5);
            display_write_string("REMAIN");
        }
        else
        {
            display_set_dd_address(4);
            display_write_string("LAST TRY");
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
        display_clear();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    // final game
    if (current_game >= sizeof(games_list) / sizeof(games_list[0]) - 1)
    {
        for (int i = 0; i < 3; ++i)
        {
            audio_playfile(SOUND_FOLDER_META, SOUND_META_BEEP1);
            display_clear();
            display_set_dd_address(3);
            display_write_string("FINAL GAME");
            vTaskDelay(500 / portTICK_PERIOD_MS);
            display_clear();
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }

    // fake loading
    display_clear();
    display_set_dd_address(0);

    display_write_string("Loading...");

    display_set_dd_address(64);

    for (int i = 0; i < 16; ++i)
    {
        display_write_data(255);

        int delay = ((float)rand() / RAND_MAX) * 500;

        vTaskDelay(delay / portTICK_PERIOD_MS);
    }

    vTaskDelay(500 / portTICK_PERIOD_MS);

    // start game
    TaskHandle_t game_logic_task_handle;
    if (xTaskCreatePinnedToCore(games_list[current_game], "Game Logic", 4096, NULL, 2, &game_logic_task_handle, 1) != pdPASS)
        ESP_ERROR_CHECK(ESP_FAIL);
    vTaskDelete(NULL);
}

void win_game()
{
    audio_playfile(SOUND_FOLDER_META, SOUND_META_GAME_WIN);

    display_clear();
    display_set_dd_address(3);
    display_write_string("GAME WON!");
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    ++current_game;
    TaskHandle_t game_logic_task_handle;
    if (xTaskCreatePinnedToCore(meta_logic_task, "Meta Logic", 4096, NULL, 2, &game_logic_task_handle, 1) != pdPASS)
        ESP_ERROR_CHECK(ESP_FAIL);
    vTaskDelete(NULL);
}

void lose_game()
{
    audio_playfile(SOUND_FOLDER_META, SOUND_META_GAME_LOSE);

    display_clear();
    display_set_dd_address(3);
    display_write_string("GAME LOST");
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    --lives;
    TaskHandle_t game_logic_task_handle;
    if (xTaskCreatePinnedToCore(meta_logic_task, "Meta Logic", 4096, NULL, 2, &game_logic_task_handle, 1) != pdPASS)
        ESP_ERROR_CHECK(ESP_FAIL);
    vTaskDelete(NULL);
}

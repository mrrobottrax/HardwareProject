#include "shake_game.h"

#include <freertos/FreeRTOS.h>

#include "display.h"
#include "accelerometer.h"
#include "meta_logic.h"
#include "audio.h"
#include "sounds.h"

void shake_game_task(void *pvParams)
{
    display_clear();
    display_set_dd_address(0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    display_write_string("Shake me!");

    display_set_dd_address(64);

    TickType_t last_increase_time = xTaskGetTickCount();
    int progress = 0;
    while (true)
    {
        // read accel
        int16_t x, y, z;
        if (accel_read(&x, &y, &z) != ESP_OK)
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }
        float movement = accel_get_movement(x, y, z);

        // decay
        if (progress > 0 && (xTaskGetTickCount() - last_increase_time) > 500 / portTICK_PERIOD_MS)
        {
            --progress;
            display_set_dd_address(progress + 64);
            display_write_data(' ');

            audio_playfile(SOUND_FOLDER_SHAKE, SOUND_SHAKE_LOSS_TONES_START + progress);

            vTaskDelay(300 / portTICK_PERIOD_MS);
        }

        float threshold = 0.6f;

        if (progress > 10)
        {
            threshold = 1.5f;
        }
        else if (progress > 5)
        {
            threshold = 1.0f;
        }

        // add
        if (movement > threshold)
        {
            audio_playfile(SOUND_FOLDER_SHAKE, SOUND_SHAKE_WIN_TONES_START + progress);

            ++progress;
            last_increase_time = xTaskGetTickCount();
            display_set_dd_address(progress + 63);
            display_write_data(255);
            vTaskDelay(300 / portTICK_PERIOD_MS);

            if (progress >= 16)
            {
                win_game();
            }
        }

        vTaskDelay(1);
    }
}
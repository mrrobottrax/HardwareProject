#include <stdio.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <driver/i2c_master.h>

#include "display.h"
#include "input.h"
#include "audio.h"
#include "accelerometer.h"
#include "meta_logic.h"

#include "space_game.h"
#include "shake_game.h"
#include "simon_game.h"

#define I2C_PORT 0x00

void app_main(void)
{
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    printf("Starting I2C on port %i\n", I2C_PORT);

    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .scl_io_num = 22,
        .sda_io_num = 21,
        .glitch_ignore_cnt = 3,
        .flags.enable_internal_pullup = false,
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    for (int i = 0; i <= 0x7F; ++i)
    {
        if (i2c_master_probe(bus_handle, i, -1) == ESP_OK)
        {
            printf("Found I2C %i\n", i);
        }
    }

    printf("Starting Game...\n");

    ESP_ERROR_CHECK(input_init());
    ESP_ERROR_CHECK(display_init(I2C_PORT));
    ESP_ERROR_CHECK(audio_init());
    ESP_ERROR_CHECK(accel_init(I2C_PORT));

    TaskHandle_t display_task_handle;
    if (xTaskCreatePinnedToCore(display_task, "Display Task", 4096, NULL, 1, &display_task_handle, 0) != pdPASS)
        ESP_ERROR_CHECK(ESP_FAIL);

    // TaskHandle_t game_logic_task_handle;
    // if (xTaskCreatePinnedToCore(meta_logic_task, "Meta Logic", 4096, NULL, 2, &game_logic_task_handle, 1) != pdPASS)
    //     ESP_ERROR_CHECK(ESP_FAIL);

    TaskHandle_t game_logic_task_handle;
    if (xTaskCreatePinnedToCore(shake_game_task, "Game Test", 4096, NULL, 2, &game_logic_task_handle, 1) != pdPASS)
        ESP_ERROR_CHECK(ESP_FAIL);
}

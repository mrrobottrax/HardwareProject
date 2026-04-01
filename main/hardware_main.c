#include <stdio.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <driver/i2c_master.h>
#include <driver/uart.h>
#include <driver/gpio.h>

#include "display.h"
#include "game.h"
#include "input.h"

#define I2C_PORT 0x00

void app_main(void)
{
    printf("Starting I2C on port %i\n", I2C_PORT);

    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .scl_io_num = 22,
        .sda_io_num = 21,
        .glitch_ignore_cnt = 7,
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

    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, GPIO_NUM_17, GPIO_NUM_16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, 1024 * 2, 0, 0, NULL, 0));

    // Example hex command array to "Play Next Track"
    uint8_t cmd_next[10] = {0x7E, 0xFF, 0x06, 0x01, 0x00, 0x00, 0x00, 0xFE, 0xFA, 0xEF};

    uart_write_bytes(UART_NUM_2, (const char *)cmd_next, sizeof(cmd_next));

    input_init();
    display_init();

    static display_task_params_t display_params = {
        .i2c_port = I2C_PORT,
    };
    TaskHandle_t display_task_handle;
    if (xTaskCreatePinnedToCore(display_task, "Display Task", 4096, &display_params, 1, &display_task_handle, 0) != pdPASS)
        ESP_ERROR_CHECK(ESP_FAIL);

    TaskHandle_t game_logic_task_handle;
    if (xTaskCreatePinnedToCore(space_game_task, "Game Logic", 4096, NULL, 2, &game_logic_task_handle, 1) != pdPASS)
        ESP_ERROR_CHECK(ESP_FAIL);
}

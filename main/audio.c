#include "audio.h"

#include <driver/uart.h>
#include <driver/gpio.h>

// https : // picaxe.com/docs/spe033.pdf

#define UART_PORT UART_NUM_2
#define BUFFER_SIZE (1024)

esp_err_t audio_init()
{
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    esp_err_t err = uart_param_config(UART_PORT, &uart_config);
    if (err != ESP_OK)
        return err;

    err = uart_set_pin(UART_PORT, GPIO_NUM_17, GPIO_NUM_16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK)
        return err;

    err = uart_driver_install(UART_PORT, BUFFER_SIZE * 2, 0, 0, NULL, 0);
    if (err != ESP_OK)
        return err;

    return ESP_OK;
}

esp_err_t audio_playfile(uint8_t folder, uint8_t file)
{
    if (folder < 1 || folder > 99 || file < 1)
    {
        return ESP_FAIL;
    }
    uint8_t cmd_play[10] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, folder, file, 0x00, 0x00, 0xEF};
    uint16_t checksum = 0 - (0xFF + 0x06 + 0x0F + 0x00 + folder + file);

    cmd_play[7] = (uint8_t)(checksum >> 8);
    cmd_play[8] = (uint8_t)(checksum);

    uart_write_bytes(UART_NUM_2, (const char *)cmd_play, sizeof(cmd_play));

    int bytes_written = uart_write_bytes(UART_NUM_2, (const char *)cmd_play, sizeof(cmd_play));

    if (bytes_written != sizeof(cmd_play))
    {
        return ESP_FAIL;
    }

    return ESP_OK;
}

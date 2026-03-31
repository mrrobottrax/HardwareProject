#include "display.h"

#include <freertos/FreeRTOS.h>

#include "lcd.h"

#define LCD_I2C_ID 0x27

#define QUEUE_SIZE 256

static QueueHandle_t lcd_cmd_queue;

esp_err_t display_init()
{
    lcd_cmd_queue = xQueueCreate(QUEUE_SIZE, sizeof(lcd_cmd_t));
    if (lcd_cmd_queue == NULL)
    {
        printf("Failed to create display queue\n");
        return ESP_FAIL;
    }

    return ESP_OK;
}

void display_task(void *pvParams)
{
    display_task_params_t params = *(display_task_params_t *)pvParams;

    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_master_get_bus_handle(params.i2c_port, &bus_handle));

    lcd_handle_t lcd_handle;
    lcd_config_t lcd_config = {
        .bus_handle = bus_handle,
        .address = LCD_I2C_ID,
    };
    ESP_ERROR_CHECK(lcd_create(&lcd_config, &lcd_handle));

    lcd_cmd_t cmd;
    while (1)
    {
        if (xQueueReceive(lcd_cmd_queue, &cmd, portMAX_DELAY))
        {
            switch (cmd.type)
            {
            case LCD_CMD_CLEAR:
                ESP_ERROR_CHECK(lcd_clear(lcd_handle));
                break;
            case LCD_CMD_WRITE:
                ESP_ERROR_CHECK(lcd_write_data(lcd_handle, cmd.write.data));
                break;
            case LCD_CMD_CURSOR_R:
                ESP_ERROR_CHECK(lcd_shift_cursor_r(lcd_handle));
                break;
            case LCD_CMD_CURSOR_L:
                ESP_ERROR_CHECK(lcd_shift_cursor_l(lcd_handle));
                break;
            case LCD_CMD_DISPLAY_R:
                ESP_ERROR_CHECK(lcd_shift_display_r(lcd_handle));
                break;
            case LCD_CMD_DISPLAY_L:
                ESP_ERROR_CHECK(lcd_shift_display_l(lcd_handle));
                break;

            case LCD_CMD_SET_CG_ADDRESS:
                ESP_ERROR_CHECK(lcd_set_cg_address(lcd_handle, cmd.set_cg_address.address));
                break;
            case LCD_CMD_SET_DD_ADDRESS:
                ESP_ERROR_CHECK(lcd_set_dd_address(lcd_handle, cmd.set_dd_address.address));
                break;

            default:
                printf("Unknown LCD command\n");
                break;
            }
        }
    }
}

esp_err_t display_cmd(const lcd_cmd_t *cmd)
{
    switch (cmd->type)
    {
    case LCD_CMD_CLEAR:
    case LCD_CMD_WRITE:
    case LCD_CMD_CURSOR_R:
    case LCD_CMD_CURSOR_L:
    case LCD_CMD_DISPLAY_R:
    case LCD_CMD_DISPLAY_L:
        break;

    case LCD_CMD_SET_CG_ADDRESS:
        if (cmd->set_cg_address.address >= 64)
            return ESP_ERR_INVALID_ARG;
        break;
    case LCD_CMD_SET_DD_ADDRESS:
        if (cmd->set_dd_address.address >= 80)
            return ESP_ERR_INVALID_ARG;
        break;

    default:
        printf("Unknown LCD command\n");
        return ESP_ERR_INVALID_ARG;
    }

    if (xQueueSendToBack(lcd_cmd_queue, cmd, 10000 / portTICK_PERIOD_MS) != pdTRUE)
    {
        printf("Display queue is full\n");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t display_clear()
{
    lcd_cmd_t cmd = {
        .type = LCD_CMD_CLEAR,
    };
    return display_cmd(&cmd);
}

esp_err_t display_set_dd_address(uint8_t address)
{
    lcd_cmd_t cmd = {
        .type = LCD_CMD_SET_DD_ADDRESS,
        .set_dd_address = {
            .address = address,
        }};
    return display_cmd(&cmd);
}

esp_err_t display_set_cg_address(uint8_t address)
{
    lcd_cmd_t cmd = {
        .type = LCD_CMD_SET_CG_ADDRESS,
        .set_cg_address = {
            .address = address,
        }};
    return display_cmd(&cmd);
}

esp_err_t display_write_data(uint8_t data)
{
    lcd_cmd_t cmd = {
        .type = LCD_CMD_WRITE,
        .write = {
            .data = data,
        }};
    return display_cmd(&cmd);
}

esp_err_t display_shift_cursor_r()
{
    lcd_cmd_t cmd = {
        .type = LCD_CMD_CURSOR_R,
    };
    return display_cmd(&cmd);
}

esp_err_t display_shift_cursor_l()
{
    lcd_cmd_t cmd = {
        .type = LCD_CMD_CURSOR_L,
    };
    return display_cmd(&cmd);
}

esp_err_t display_shift_display_r()
{
    lcd_cmd_t cmd = {
        .type = LCD_CMD_DISPLAY_R,
    };
    return display_cmd(&cmd);
}

esp_err_t display_shift_display_l()
{
    lcd_cmd_t cmd = {
        .type = LCD_CMD_DISPLAY_L,
    };
    return display_cmd(&cmd);
}
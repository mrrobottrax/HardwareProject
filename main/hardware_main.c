#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "driver/i2c_master.h"

#include "lcd.h"

#define I2C_PORT 0x00
#define LCD_I2C_ID 0x27

void app_main(void)
{
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

    i2c_device_config_t lcd_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = LCD_I2C_ID,
        .scl_speed_hz = 40 * 1000,
    };

    i2c_master_dev_handle_t lcd_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &lcd_cfg, &lcd_handle));

    printf("Running Backlight Test...\n");
    for (int i = 0; i < 10; i++)
    {
        uint8_t bl_on = 0b00001000; // Backlight ON bit
        i2c_master_transmit(lcd_handle, &bl_on, 1, 10);
        vTaskDelay(pdMS_TO_TICKS(200));

        uint8_t bl_off = 0b00000000; // Backlight OFF bit
        i2c_master_transmit(lcd_handle, &bl_off, 1, 10);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    printf("Backlight Test Done.\n");

    printf("Initializing display...\n");

    lcd_setup(lcd_handle);

    printf("Display done\n");

    lcd_clear(lcd_handle);

    lcd_set_cg_address(lcd_handle, 0);
    for (int i = 0; i < 8 * 8; ++i)
        lcd_write_data(lcd_handle, 0);

    printf("CGRAM Clear\n");

    lcd_set_dd_address(lcd_handle, 0);
    lcd_write_data(lcd_handle, 0);
    lcd_write_data(lcd_handle, 0);
    lcd_write_data(lcd_handle, 0);

    printf("Draw box\n");

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    for (int i = 0; i < 10; ++i)
    {
        printf("Loop %i\n", i);
        for (int j = 0; j < 6; ++j)
        {
            int h = j;
            if (h >= 4)
            {
                h = 6 - h;
            }

            lcd_set_cg_address(lcd_handle, 0);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            for (int x = 0; x < (3 - h); ++x)
                lcd_write_data(lcd_handle, 0b00000000);
            lcd_write_data(lcd_handle, 0b00011111);
            lcd_write_data(lcd_handle, 0b00010001);
            lcd_write_data(lcd_handle, 0b00010001);
            lcd_write_data(lcd_handle, 0b00010001);
            lcd_write_data(lcd_handle, 0b00011111);
            for (int x = 0; x < h; ++x)
                lcd_write_data(lcd_handle, 0b00000000);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_ERROR_CHECK(i2c_master_bus_rm_device(lcd_handle));

    ESP_ERROR_CHECK(i2c_del_master_bus(bus_handle));

    for (int i = 10; i >= 0; i--)
    {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}

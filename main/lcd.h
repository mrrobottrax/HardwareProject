#include "driver/i2c_master.h"

typedef struct lcd_t *lcd_handle_t;

typedef struct
{
    i2c_master_bus_handle_t bus_handle;
    uint16_t address;
} lcd_config_t;

esp_err_t lcd_create(const lcd_config_t *config, lcd_handle_t *lcd_handle);

esp_err_t lcd_delete(lcd_handle_t lcd_handle);

esp_err_t lcd_clear(lcd_handle_t lcd_handle);

esp_err_t lcd_set_dd_address(lcd_handle_t lcd_handle, uint8_t address);

esp_err_t lcd_set_cg_address(lcd_handle_t lcd_handle, uint8_t address);

esp_err_t lcd_write_data(lcd_handle_t lcd_handle, uint8_t byte);

esp_err_t lcd_shift_cursor_r(lcd_handle_t lcd_handle);
esp_err_t lcd_shift_cursor_l(lcd_handle_t lcd_handle);

esp_err_t lcd_shift_display_r(lcd_handle_t lcd_handle);
esp_err_t lcd_shift_display_l(lcd_handle_t lcd_handle);

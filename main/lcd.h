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

// esp_err_t lcd_send_8bit_control(lcd_handle_t lcd_handle, uint8_t data);

// void lcd_send_8bit_data(i2c_master_dev_handle_t lcd_handle, uint8_t data);

// void lcd_cursor_home(i2c_master_dev_handle_t lcd_handle);

// void lcd_entry_mode(i2c_master_dev_handle_t lcd_handle, bool data_increment, bool shift_display);

// void lcd_display_enable(i2c_master_dev_handle_t lcd_handle, bool display_on, bool display_cursor, bool character_blink);

// void lcd_shift_cursor_r(i2c_master_dev_handle_t lcd_handle);
// void lcd_shift_cursor_l(i2c_master_dev_handle_t lcd_handle);

// void lcd_shift_display_r(i2c_master_dev_handle_t lcd_handle);
// void lcd_shift_display_l(i2c_master_dev_handle_t lcd_handle);

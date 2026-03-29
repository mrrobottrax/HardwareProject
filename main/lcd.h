#include "driver/i2c_master.h"

void lcd_setup(i2c_master_dev_handle_t lcd_handle);

void lcd_send_8bit_control(i2c_master_dev_handle_t lcd_handle, uint8_t data);

void lcd_send_8bit_data(i2c_master_dev_handle_t lcd_handle, uint8_t data);

void lcd_clear(i2c_master_dev_handle_t lcd_handle);

void lcd_cursor_home(i2c_master_dev_handle_t lcd_handle);

void lcd_entry_mode(i2c_master_dev_handle_t lcd_handle, bool data_increment, bool shift_display);

void lcd_display_enable(i2c_master_dev_handle_t lcd_handle, bool display_on, bool display_cursor, bool character_blink);

void lcd_shift_cursor_r(i2c_master_dev_handle_t lcd_handle);
void lcd_shift_cursor_l(i2c_master_dev_handle_t lcd_handle);

void lcd_shift_display_r(i2c_master_dev_handle_t lcd_handle);
void lcd_shift_display_l(i2c_master_dev_handle_t lcd_handle);

void lcd_set_cg_address(i2c_master_dev_handle_t lcd_handle, uint8_t address);
void lcd_set_dd_address(i2c_master_dev_handle_t lcd_handle, uint8_t address);

void lcd_write_data(i2c_master_dev_handle_t lcd_handle, uint8_t byte);

void lcd_write_5x7_char(i2c_master_dev_handle_t lcd_handle, uint8_t* array);

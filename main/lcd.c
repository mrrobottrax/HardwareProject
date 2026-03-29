#include "lcd.h"

#include "freertos/FreeRTOS.h"

/*

LCD Bits:
PCF8574 Bit 	LCD Function	Description
P0	            RS	            Register Select (0 = Command, 1 = Data)
P1	            R/W	            Read/Write (usually kept 0 for Write)
P2	            EN	            Enable (Strobe to clock data)
P3	            BL	            Backlight Control (1 = On, 0 = Off)
P4	            D4	            Data Bit 4 (High nibble mode)
P5	            D5	            Data Bit 5
P6	            D6	            Data Bit 6
P7	            D7	            Data Bit 7

*/

// https://www.scribd.com/document/564283347/LCD-HD44780-instruction-set

#define DELAY 50
#define TIMEOUT 50

static void lcd_send_4bit_control(i2c_master_dev_handle_t lcd_handle, uint8_t data)
{
    if (data > 0xF)
    {
        return;
    }

    uint8_t b1 = 0b00001100;
    b1 |= data << 4;

    uint8_t b2 = b1 & 0b11111011;

    uint8_t d[] = {b2, b1, b2};
    ESP_ERROR_CHECK(i2c_master_transmit(lcd_handle, d, 3, TIMEOUT));
    vTaskDelay(1);
}

void lcd_send_8bit_control(i2c_master_dev_handle_t lcd_handle, uint8_t data)
{
    uint8_t b1 = 0b00001100;
    b1 |= data & 0xF0;

    uint8_t b2 = b1 & 0b11111011;

    uint8_t b3 = 0b00001100;
    b3 |= (data & 0xF) << 4;

    uint8_t b4 = b3 & 0b11111011;

    uint8_t d[] = {b2, b1, b2, b4, b3, b4};
    while(i2c_master_transmit(lcd_handle, d, 6, TIMEOUT) == ESP_ERR_INVALID_RESPONSE);
    vTaskDelay(1);
}

void lcd_send_8bit_data(i2c_master_dev_handle_t lcd_handle, uint8_t data)
{
    uint8_t b1 = 0b00001101;
    b1 |= data & 0xF0;

    uint8_t b2 = b1 & 0b11111011;

    uint8_t b3 = 0b00001101;
    b3 |= (data & 0xF) << 4;

    uint8_t b4 = b3 & 0b11111011;

    uint8_t d[] = {b2, b1, b2, b4, b3, b4};
    while(i2c_master_transmit(lcd_handle, d, 6, TIMEOUT) == ESP_ERR_INVALID_RESPONSE);
    vTaskDelay(1);
}

void lcd_setup(i2c_master_dev_handle_t lcd_handle)
{
    vTaskDelay(pdMS_TO_TICKS(50));

    lcd_send_4bit_control(lcd_handle, 0x03);
    vTaskDelay(1);

    lcd_send_4bit_control(lcd_handle, 0x03);
    vTaskDelay(1);

    lcd_send_4bit_control(lcd_handle, 0x03);
    vTaskDelay(1);

    lcd_send_4bit_control(lcd_handle, 0x02);
    vTaskDelay(1);

    // font
    lcd_send_8bit_control(lcd_handle, 0x28);
    vTaskDelay(1);

    lcd_display_enable(lcd_handle, false, false, false);
    vTaskDelay(1);

    lcd_clear(lcd_handle);
    vTaskDelay(1);

    lcd_entry_mode(lcd_handle, true, false);
    vTaskDelay(1);

    lcd_display_enable(lcd_handle, true, false, false);
    vTaskDelay(1);
}

void lcd_clear(i2c_master_dev_handle_t lcd_handle)
{
    lcd_send_8bit_control(lcd_handle, 0b00000001);
    vTaskDelay(1);
}

void lcd_cursor_home(i2c_master_dev_handle_t lcd_handle)
{
    lcd_send_8bit_control(lcd_handle, 0b00000010);
    vTaskDelay(1);
}

void lcd_entry_mode(i2c_master_dev_handle_t lcd_handle, bool data_increment, bool shift_display)
{
    uint8_t b = 0b00000100;
    if (data_increment)
        b |= 0b00000010;

    if (shift_display)
        b |= 0b00000001;

    lcd_send_8bit_control(lcd_handle, b);
    vTaskDelay(1);
    // esp_rom_delay_us(DELAY);
}

void lcd_display_enable(i2c_master_dev_handle_t lcd_handle, bool display_on, bool display_cursor, bool character_blink)
{
    uint8_t b = 0b00001000;
    if (display_on)
        b |= 0b00000100;

    if (display_cursor)
        b |= 0b00000010;

    if (character_blink)
        b |= 0b00000001;

    lcd_send_8bit_control(lcd_handle, b);
    vTaskDelay(1);
    // esp_rom_delay_us(DELAY);
}

void lcd_shift_cursor_r(i2c_master_dev_handle_t lcd_handle)
{
    lcd_send_8bit_control(lcd_handle, 0b00010100);
    vTaskDelay(1);
    // esp_rom_delay_us(DELAY);
}

void lcd_shift_cursor_l(i2c_master_dev_handle_t lcd_handle)
{
    lcd_send_8bit_control(lcd_handle, 0b00010000);
    vTaskDelay(1);
    // esp_rom_delay_us(DELAY);
}

void lcd_shift_display_r(i2c_master_dev_handle_t lcd_handle)
{
    lcd_send_8bit_control(lcd_handle, 0b00011100);
    vTaskDelay(1);
    // esp_rom_delay_us(DELAY);
}

void lcd_shift_display_l(i2c_master_dev_handle_t lcd_handle)
{
    lcd_send_8bit_control(lcd_handle, 0b00011000);
    vTaskDelay(1);
    // esp_rom_delay_us(DELAY);
}

void lcd_set_cg_address(i2c_master_dev_handle_t lcd_handle, uint8_t address)
{
    if (address > 0b00111111)
    {
        return;
    }

    lcd_send_8bit_control(lcd_handle, 0b01000000 | address);
    vTaskDelay(1);
    // esp_rom_delay_us(DELAY);
}

void lcd_set_dd_address(i2c_master_dev_handle_t lcd_handle, uint8_t address)
{
    if (address > 0b01111111)
    {
        return;
    }

    lcd_send_8bit_control(lcd_handle, 0b10000000 | address);
    vTaskDelay(1);
    // esp_rom_delay_us(DELAY);
}

void lcd_write_data(i2c_master_dev_handle_t lcd_handle, uint8_t byte)
{
    lcd_send_8bit_data(lcd_handle, byte);
    esp_rom_delay_us(200);
}

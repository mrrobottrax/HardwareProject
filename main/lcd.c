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

#define DELAY 60
#define TIMEOUT 200

typedef struct
{
    bool is_address_cg;
    uint8_t mem_address;
    uint8_t shift;

    uint8_t ddram[80];
    uint8_t cgram[64];
} lcd_state_t;

typedef struct lcd_t
{
    uint16_t i2c_address;
    i2c_master_bus_handle_t i2c_bus_handle;
    i2c_master_dev_handle_t i2c_dev_handle;

    bool is_device_correct_state;

    lcd_state_t state;
} lcd_t;

static esp_err_t lcd_send_4bit_control(lcd_handle_t lcd_handle, uint8_t data)
{
    const uint8_t BL = 0b1000;
    const uint8_t EN = 0b100;

    if (data > 0xF)
        return ESP_ERR_INVALID_ARG;

    uint8_t l = data << 4;
    l |= BL;

    uint8_t h = l | EN;

    uint8_t d[] = {l, h, l};

    return i2c_master_transmit(lcd_handle->i2c_dev_handle, d, 3, TIMEOUT);
}

static esp_err_t lcd_send_8bit_control(lcd_handle_t lcd_handle, uint8_t data)
{
    const uint8_t BL = 0b1000;
    const uint8_t EN = 0b100;

    uint8_t a_l = data & 0xF0;
    a_l |= BL;

    uint8_t a_h = a_l | EN;

    uint8_t b_l = (data & 0xF) << 4;
    b_l |= BL;

    uint8_t b_h = b_l | EN;

    uint8_t d[] = {a_l, a_h, a_l, b_l, b_h, b_l};

    return i2c_master_transmit(lcd_handle->i2c_dev_handle, d, 6, TIMEOUT);
}

static esp_err_t lcd_send_8bit_data(lcd_handle_t lcd_handle, uint8_t data)
{
    const uint8_t BL = 0b1000;
    const uint8_t EN = 0b100;
    const uint8_t RS = 0b1;

    uint8_t a_l = data & 0xF0;
    a_l |= BL | RS;

    uint8_t a_h = a_l | EN;

    uint8_t b_l = (data & 0xF) << 4;
    b_l |= BL | RS;

    uint8_t b_h = b_l | EN;

    uint8_t d[] = {a_l, a_h, a_l, b_l, b_h, b_l};

    return i2c_master_transmit(lcd_handle->i2c_dev_handle, d, 6, TIMEOUT);
}

static esp_err_t lcd_initialize(lcd_handle_t lcd_handle)
{
    esp_err_t err = ESP_OK;

    vTaskDelay(20 / portTICK_PERIOD_MS);

    err = lcd_send_4bit_control(lcd_handle, 0b0011);
    if (err != ESP_OK)
        return err;
    esp_rom_delay_us(4500);

    err = lcd_send_4bit_control(lcd_handle, 0b0011);
    if (err != ESP_OK)
        return err;
    esp_rom_delay_us(150);

    err = lcd_send_4bit_control(lcd_handle, 0b0011);
    if (err != ESP_OK)
        return err;
    esp_rom_delay_us(DELAY);

    err = lcd_send_4bit_control(lcd_handle, 0b0010);
    if (err != ESP_OK)
        return err;
    esp_rom_delay_us(DELAY);

    err = lcd_send_8bit_control(lcd_handle, 0x28);
    if (err != ESP_OK)
        return err;
    esp_rom_delay_us(DELAY);

    err = lcd_send_8bit_control(lcd_handle, 0x08);
    if (err != ESP_OK)
        return err;
    esp_rom_delay_us(DELAY);

    err = lcd_send_8bit_control(lcd_handle, 0x01);
    if (err != ESP_OK)
        return err;
    esp_rom_delay_us(2000);

    err = lcd_send_8bit_control(lcd_handle, 0x06);
    if (err != ESP_OK)
        return err;
    esp_rom_delay_us(DELAY);

    // display enable
    enum
    {
        display_enable = 0b100,
        cursor = 0b10,
        blink = 0b1,
    };
    uint8_t cmd = 0b1000;

    err = lcd_send_8bit_control(lcd_handle, cmd | display_enable);
    if (err != ESP_OK)
        return err;
    esp_rom_delay_us(DELAY);

    return ESP_OK;
}

static esp_err_t lcd_get_to_correct_state(lcd_handle_t lcd_handle)
{
    lcd_handle->is_device_correct_state = false;

    // ESP_ERROR_CHECK(i2c_master_bus_reset(lcd_handle->i2c_bus_handle));

    printf("Getting LCD into state\n");

    esp_err_t err;
    err = lcd_initialize(lcd_handle);
    if (err != ESP_OK)
        return err;

    // copy ddram
    err = lcd_send_8bit_control(lcd_handle, 0b10000000);
    if (err != ESP_OK)
        return err;
    esp_rom_delay_us(DELAY);

    for (int i = 0; i < sizeof(lcd_handle->state.ddram); ++i)
    {
        err = lcd_send_8bit_data(lcd_handle, lcd_handle->state.ddram[i]);
        if (err != ESP_OK)
            return err;
        esp_rom_delay_us(DELAY);
    }

    // copy cgram
    err = lcd_send_8bit_control(lcd_handle, 0b1000000);
    if (err != ESP_OK)
        return err;
    esp_rom_delay_us(DELAY);

    for (int i = 0; i < sizeof(lcd_handle->state.cgram); ++i)
    {
        err = lcd_send_8bit_data(lcd_handle, lcd_handle->state.cgram[i]);
        if (err != ESP_OK)
            return err;
        esp_rom_delay_us(DELAY);
    }

    // set display shift
    const uint8_t shift = lcd_handle->state.shift;
    if (shift <= 40 - shift)
    {
        for (int i = 0; i < shift; ++i)
        {
            err = lcd_send_8bit_control(lcd_handle, 0b11100);
            if (err != ESP_OK)
                return err;
            esp_rom_delay_us(DELAY);
        }
    }
    else
    {
        for (int i = 0; i < 40 - shift; ++i)
        {
            err = lcd_send_8bit_control(lcd_handle, 0b11000);
            if (err != ESP_OK)
                return err;
            esp_rom_delay_us(DELAY);
        }
    }

    // set address
    if (lcd_handle->state.is_address_cg)
    {
        err = lcd_send_8bit_control(lcd_handle, 0b01000000 | (lcd_handle->state.mem_address & 0b111111));
        if (err != ESP_OK)
            return err;
        esp_rom_delay_us(DELAY);
    }
    else
    {
        err = lcd_send_8bit_control(lcd_handle, 0b10000000 | (lcd_handle->state.mem_address & 0b1111111));
        if (err != ESP_OK)
            return err;
        esp_rom_delay_us(DELAY);
    }

    lcd_handle->is_device_correct_state = true;

    printf("LCD in correct state\n");

    return ESP_OK;
}

static esp_err_t lcd_send_8bit_control_reliable(lcd_handle_t lcd_handle, uint8_t data)
{
    if (!lcd_handle)
        return ESP_ERR_INVALID_ARG;

    while (!lcd_handle->is_device_correct_state)
    {
        if (lcd_get_to_correct_state(lcd_handle) != ESP_OK)
            printf("Error getting into correct state\n");
    }

    while (true)
    {
        esp_err_t err = lcd_send_8bit_control(lcd_handle, data);
        if (err == ESP_OK)
            break;

        printf("Error sending... resetting\n");

        do
        {
            if (lcd_get_to_correct_state(lcd_handle) != ESP_OK)
                printf("Error getting into correct state\n");
        } while (!lcd_handle->is_device_correct_state);
    }

    return ESP_OK;
}

static esp_err_t lcd_send_8bit_data_reliable(lcd_handle_t lcd_handle, uint8_t data)
{
    if (!lcd_handle)
        return ESP_ERR_INVALID_ARG;

    while (!lcd_handle->is_device_correct_state)
    {
        if (lcd_get_to_correct_state(lcd_handle) != ESP_OK)
            printf("Error getting into correct state\n");
    }

    while (true)
    {
        esp_err_t err = lcd_send_8bit_data(lcd_handle, data);
        if (err == ESP_OK)
            break;

        printf("Error sending... resetting\n");

        do
        {
            if (lcd_get_to_correct_state(lcd_handle) != ESP_OK)
                printf("Error getting into correct state\n");
        } while (!lcd_handle->is_device_correct_state);
    }

    return ESP_OK;
}

esp_err_t lcd_create(const lcd_config_t *config, lcd_handle_t *lcd_handle)
{
    if (config->address > 0b1111111)
        return ESP_ERR_INVALID_ARG;

    if (!config->bus_handle)
        return ESP_ERR_INVALID_ARG;

    *lcd_handle = calloc(1, sizeof(lcd_t));
    if (lcd_handle == 0)
        return ESP_ERR_NO_MEM;

    (*lcd_handle)->i2c_address = config->address;
    (*lcd_handle)->i2c_bus_handle = config->bus_handle;
    (*lcd_handle)->i2c_dev_handle = 0;

    for (int i = 0; i < sizeof((*lcd_handle)->state.ddram); ++i)
        (*lcd_handle)->state.ddram[i] = ' ';

    i2c_device_config_t i2c_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = config->address,
        .scl_speed_hz = 100 * 1000,
        .scl_wait_us = 0,
        .flags = {
            .disable_ack_check = false,
        },
    };
    esp_err_t err = i2c_master_bus_add_device(config->bus_handle, &i2c_config, &(*lcd_handle)->i2c_dev_handle);
    if (err != ESP_OK)
    {
        free(*lcd_handle);
        return err;
    }

    return ESP_OK;
}

esp_err_t lcd_delete(lcd_handle_t lcd_handle)
{
    if (lcd_handle == 0)
        return ESP_ERR_INVALID_ARG;

    esp_err_t error = i2c_master_bus_rm_device(lcd_handle->i2c_dev_handle);
    if (error != ESP_OK)
        return error;

    free(lcd_handle);

    return ESP_OK;
}

esp_err_t lcd_clear(lcd_handle_t lcd_handle)
{
    esp_err_t err = lcd_send_8bit_control_reliable(lcd_handle, 0b1);
    if (err != ESP_OK)
        return err;

    lcd_handle->state.is_address_cg = false;
    lcd_handle->state.mem_address = 0;
    lcd_handle->state.shift = 0;

    for (int i = 0; i < sizeof(lcd_handle->state.ddram); ++i)
        lcd_handle->state.ddram[i] = ' ';

    vTaskDelay(1);

    return ESP_OK;
}

esp_err_t lcd_set_dd_address(lcd_handle_t lcd_handle, uint8_t address)
{
    if (address > 0b01111111)
        return ESP_ERR_INVALID_ARG;

    esp_err_t err = lcd_send_8bit_control_reliable(lcd_handle, 0b10000000 | address);
    if (err != ESP_OK)
        return err;

    lcd_handle->state.is_address_cg = false;
    lcd_handle->state.mem_address = address;
    esp_rom_delay_us(DELAY);

    return ESP_OK;
}

esp_err_t lcd_set_cg_address(lcd_handle_t lcd_handle, uint8_t address)
{
    if (address > 0b00111111)
        return ESP_ERR_INVALID_ARG;

    esp_err_t err = lcd_send_8bit_control_reliable(lcd_handle, 0b01000000 | address);
    if (err != ESP_OK)
        return err;

    lcd_handle->state.is_address_cg = true;
    lcd_handle->state.mem_address = address;
    esp_rom_delay_us(DELAY);

    return ESP_OK;
}

esp_err_t lcd_write_data(lcd_handle_t lcd_handle, uint8_t byte)
{
    esp_err_t err = lcd_send_8bit_data_reliable(lcd_handle, byte);
    if (err != ESP_OK)
        return err;

    if (lcd_handle->state.is_address_cg)
    {
        lcd_handle->state.cgram[lcd_handle->state.mem_address] = byte;
        ++lcd_handle->state.mem_address;
        lcd_handle->state.mem_address %= sizeof(lcd_handle->state.cgram);
    }
    else
    {
        lcd_handle->state.ddram[lcd_handle->state.mem_address] = byte;
        ++lcd_handle->state.mem_address;
        lcd_handle->state.mem_address %= sizeof(lcd_handle->state.ddram);
    }

    esp_rom_delay_us(DELAY);

    return ESP_OK;
}

esp_err_t lcd_shift_cursor_r(lcd_handle_t lcd_handle)
{
    esp_err_t err = lcd_send_8bit_data_reliable(lcd_handle, 0b00010100);
    if (err != ESP_OK)
        return err;

    ++lcd_handle->state.mem_address;
    if (lcd_handle->state.is_address_cg)
    {
        lcd_handle->state.mem_address %= sizeof(lcd_handle->state.cgram);
    }
    else
    {
        lcd_handle->state.mem_address %= sizeof(lcd_handle->state.ddram);
    }

    esp_rom_delay_us(DELAY);

    return ESP_OK;
}

esp_err_t lcd_shift_cursor_l(lcd_handle_t lcd_handle)
{
    esp_err_t err = lcd_send_8bit_data_reliable(lcd_handle, 0b00010000);
    if (err != ESP_OK)
        return err;

    --lcd_handle->state.mem_address;
    if (lcd_handle->state.is_address_cg)
    {
        if (lcd_handle->state.mem_address >= sizeof(lcd_handle->state.cgram))
            lcd_handle->state.mem_address = 0;
    }
    else
    {
        if (lcd_handle->state.mem_address >= sizeof(lcd_handle->state.ddram))
            lcd_handle->state.mem_address = 0;
    }

    esp_rom_delay_us(DELAY);

    return ESP_OK;
}

esp_err_t lcd_shift_display_r(lcd_handle_t lcd_handle)
{
    esp_err_t err = lcd_send_8bit_data_reliable(lcd_handle, 0b00011100);
    if (err != ESP_OK)
        return err;

    ++lcd_handle->state.shift;
    lcd_handle->state.shift %= 40;

    esp_rom_delay_us(DELAY);

    return ESP_OK;
}

esp_err_t lcd_shift_display_l(lcd_handle_t lcd_handle)
{
    esp_err_t err = lcd_send_8bit_data_reliable(lcd_handle, 0b00011000);
    if (err != ESP_OK)
        return err;

    --lcd_handle->state.shift;
    if (lcd_handle->state.shift >= 40)
        lcd_handle->state.shift = 0;

    esp_rom_delay_us(DELAY);

    return ESP_OK;
}
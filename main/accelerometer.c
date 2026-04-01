#include "accelerometer.h"

#include <math.h>
#include <freertos/FreeRTOS.h>
#include <driver/i2c_master.h>

#define ACCEL_I2C_ADDRESS 0x68
#define PWR_MGMT_1_REG 0x6B
#define ACCEL_XOUT_H_REG 0x3B
#define TIMEOUT 1000

static i2c_master_dev_handle_t accel_handle;

esp_err_t accel_init(i2c_port_num_t i2c_port)
{
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_master_get_bus_handle(i2c_port, &bus_handle));

    i2c_device_config_t i2c_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = ACCEL_I2C_ADDRESS,
        .scl_speed_hz = 100 * 1000,
        .scl_wait_us = 0,
        .flags = {
            .disable_ack_check = false,
        },
    };
    esp_err_t err = i2c_master_bus_add_device(bus_handle, &i2c_config, &accel_handle);
    if (err != ESP_OK)
        return err;

    uint8_t write_buf[2] = {PWR_MGMT_1_REG, 0x00};
    err = i2c_master_transmit(
        accel_handle,
        write_buf,
        sizeof(write_buf),
        pdMS_TO_TICKS(TIMEOUT));
    if (err != ESP_OK)
        return err;

    return ESP_OK;
}

esp_err_t accel_read(int16_t *accel_x, int16_t *accel_y, int16_t *accel_z)
{
    uint8_t reg = ACCEL_XOUT_H_REG;
    uint8_t data[6];

    esp_err_t err = i2c_master_transmit_receive(
        accel_handle,
        &reg,
        1,
        data,
        6,
        pdMS_TO_TICKS(TIMEOUT));

    if (err == ESP_OK)
    {
        *accel_x = (int16_t)((data[0] << 8) | data[1]);
        *accel_y = (int16_t)((data[2] << 8) | data[3]);
        *accel_z = (int16_t)((data[4] << 8) | data[5]);
    }

    return err;
}

float accel_get_movement(int16_t x, int16_t y, int16_t z)
{
    float fx = (float)x / (1 << 14);
    float fy = (float)y / (1 << 14);
    float fz = (float)z / (1 << 14) - 0.2f; // calibration

    float mag = sqrtf(fx * fx + fy * fy + fz * fz);

    float movement = fabsf(mag - 1);

    return movement;
}
#include <esp_err.h>
#include <driver/i2c_master.h>

esp_err_t accel_init(i2c_port_num_t i2c_port);
esp_err_t accel_read(int16_t *accel_x, int16_t *accel_y, int16_t *accel_z);

float accel_get_movement(int16_t x, int16_t y, int16_t z);
#include <esp_err.h>

typedef struct
{
    bool keypad[12];
} input_data_t;

esp_err_t input_init();

input_data_t input_read();
typedef struct
{
    bool keypad[12];
} input_data_t;

void input_init();

input_data_t input_read();
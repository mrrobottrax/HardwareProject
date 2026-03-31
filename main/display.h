#include <inttypes.h>
#include <esp_err.h>

esp_err_t display_init();

typedef struct
{
    uint8_t i2c_port;
} display_task_params_t;

void display_task(void *pvParams);

typedef enum
{
    LCD_CMD_CLEAR,
    LCD_CMD_SET_DD_ADDRESS,
    LCD_CMD_SET_CG_ADDRESS,
    LCD_CMD_WRITE,
    LCD_CMD_CURSOR_R,
    LCD_CMD_CURSOR_L,
    LCD_CMD_DISPLAY_R,
    LCD_CMD_DISPLAY_L,
} lcd_cmd_type_t;

typedef struct
{
    lcd_cmd_type_t type;

    union
    {
        struct
        {
            uint8_t address;
        } set_dd_address;

        struct
        {
            uint8_t address;
        } set_cg_address;

        struct
        {
            uint8_t data;
        } write;
    };

} lcd_cmd_t;

esp_err_t display_cmd(const lcd_cmd_t *cmd);

esp_err_t display_clear();
esp_err_t display_set_dd_address(uint8_t address);
esp_err_t display_set_cg_address(uint8_t address);
esp_err_t display_write_data(uint8_t byte);
esp_err_t display_shift_cursor_r();
esp_err_t display_shift_cursor_l();
esp_err_t display_shift_display_r();
esp_err_t display_shift_display_l();
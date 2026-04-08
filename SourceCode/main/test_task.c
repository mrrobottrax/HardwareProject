#include "test_task.h"

#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>

#include "input.h"

void test_task(void *pvParams)
{
    while (true)
    {
        gpio_set_level(14, 0);

        esp_rom_delay_us(50);

        // 27, 13, 25
        if (gpio_get_level(27) == 0)
        {
            printf("TEST\n");
        }

        gpio_set_level(14, 1);
        vTaskDelay(1);
    }
}

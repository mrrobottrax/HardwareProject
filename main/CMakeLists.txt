file(GLOB_RECURSE SOURCES *.c *.cpp)

idf_component_register(SRCS "hardware_main.c" "lcd.c" "game.c" "${SOURCES}"
                       PRIV_REQUIRES esp_driver_i2c esp_driver_gpio
                       INCLUDE_DIRS ".")

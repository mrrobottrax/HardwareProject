if(NOT CMAKE_BUILD_EARLY_EXPANSION)
    file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.c *.cpp)
endif()

idf_component_register(SRCS "lcd.c" "hardware_main.c" "${SOURCES}"
                       PRIV_REQUIRES spi_flash esp_driver_i2c esp_driver_gpio
                       INCLUDE_DIRS ".")

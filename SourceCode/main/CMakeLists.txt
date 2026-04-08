file(GLOB_RECURSE SOURCES *.c *.cpp)

idf_component_register(SRCS ${SOURCES}
                       PRIV_REQUIRES esp_driver_i2c esp_driver_gpio esp_driver_uart
                       INCLUDE_DIRS ".")

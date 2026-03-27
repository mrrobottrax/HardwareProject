file(GLOB SOURCES *.c *.cpp)

idf_component_register(SRCS "${SOURCES}"
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS ".")

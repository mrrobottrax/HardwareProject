if(NOT CMAKE_BUILD_EARLY_EXPANSION)
    file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.c *.cpp)
endif()

idf_component_register(SRCS "${SOURCES}"
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS ".")

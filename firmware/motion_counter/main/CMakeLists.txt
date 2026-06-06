idf_component_register(
    SRCS
        "main.c"
        "i2c_bus.c"
        "imu.c"
        "oled.c"
    INCLUDE_DIRS "."
    REQUIRES driver
)
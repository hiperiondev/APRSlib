file(
    GLOB_RECURSE
        SOURCES
            esp32_port/*.c
)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS
        esp32_port/
        .
    REQUIRES    
        esp_timer
        driver
        esp-dsp
        esp_adc

)
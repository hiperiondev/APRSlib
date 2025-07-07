file(
    GLOB_RECURSE
        SOURCES
            ax25_fx25/lwfec/*.c
            ax25_fx25/*.c
            modem/*.c
            kiss/*.c
            aprs/*.c
)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS
        ax25_fx25/lwfec/
        ax25_fx25/
        modem/
        kiss/
        aprs/
    REQUIRES    
        APRSlib_port
)
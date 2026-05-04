idf_component_register(
    SRCS "Multiplexer_sensor_test.cpp" 
        "main.cpp"
        "SDCard.cpp"
        "Ethernet_com.cpp"
    INCLUDE_DIRS "."
    REQUIRES
        driver esp_timer 
        fatfs 
        spi_flash 
        esp_vfs_fat 
        sdmmc 
        ioLibrary_Driver
)

idf_component_register(
    SRCS
       
        "ConnectionLoss.cpp"
        "EthernetCom.cpp"
        "SDCard.cpp"

    INCLUDE_DIRS
        "."

    REQUIRES
        # Logging
        log

        # Timers
        esp_timer

        # GPIO + SPI master driver
        driver

        # SD card + FAT filesystem
        fatfs
        sdmmc

        # FreeRTOS (vTaskDelay etc.)
        freertos

        # WIZnet ioLibrary_Driver — clone into components/ and name the folder ioLibrary_Driver.
        # Add the folder name here exactly as it appears on disk.
        ioLibrary_Driver
)

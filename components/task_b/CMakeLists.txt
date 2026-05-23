idf_component_register(SRCS "task_b.c"
                    INCLUDE_DIRS "include" ".."
                    REQUIRES freertos
                    PRIV_REQUIRES esp_driver_uart)

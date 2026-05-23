idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "." "../components"
    REQUIRES task_a task_b rgb_led
)

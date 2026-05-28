#include "task_b.h"

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "driver/uart.h"
#include "esp_log.h"

#include "shared_types.h"

#define UART_PORT UART_NUM_0
#define BUF_SIZE 128

static const char *TAG = "TASK_B";

static int parse_color(const char *color, rgb_color_t *rgb)
{
    if (strcmp(color, "ROJO") == 0) {
        rgb->r = 255; rgb->g = 0; rgb->b = 0;
        return 1;
    }

    if (strcmp(color, "VERDE") == 0) {
        rgb->r = 0; rgb->g = 255; rgb->b = 0;
        return 1;
    }

    if (strcmp(color, "AZUL") == 0) {
        rgb->r = 0; rgb->g = 0; rgb->b = 255;
        return 1;
    }

    if (strcmp(color, "BLANCO") == 0) {
        rgb->r = 255; rgb->g = 255; rgb->b = 255;
        return 1;
    }

    return 0;
}

void task_b(void *pvParameters)
{
    QueueHandle_t queue = (QueueHandle_t) pvParameters;

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_LOGI(TAG, "Task B iniciada. Escribir comandos tipo: ROJO 5");

    char line[BUF_SIZE];
    int idx = 0;

    while (1) {
        uint8_t c;

        int len = uart_read_bytes(UART_PORT, &c, 1, pdMS_TO_TICKS(20));

        if (len > 0) {
            uart_write_bytes(UART_PORT, (const char *)&c, 1);

            if (c == '\n' || c == '\r') {
                line[idx] = '\0';
                idx = 0;

                if (strlen(line) == 0) {
                    continue;
                }

                ESP_LOGI(TAG, "Comando recibido: %s", line);

                char color_str[20];
                unsigned int delay_s;

                if (sscanf(line, "%19s %u", color_str, &delay_s) == 2) {
                    led_command_t cmd;
                    cmd.delay_s = delay_s;

                    if (!parse_color(color_str, &cmd.color)) {
                        ESP_LOGW(TAG, "Color no reconocido: %s", color_str);
                        uart_write_bytes(UART_PORT, "\nERROR: color invalido\n", 23);
                        continue;
                    }

                    if (xQueueSend(queue, &cmd, portMAX_DELAY) == pdPASS) {
                        char response[64];
                        
                        // Monitoreo el STACK de la tarea antes de enviar la respuesta
                        ESP_LOGI(TAG, "Stack libre: %u bytes", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));
                        
                        snprintf(response, sizeof(response), "\nOK: %s en %us\n", color_str, delay_s);
                        uart_write_bytes(UART_PORT, response, strlen(response));
                        ESP_LOGI(TAG, "Comando enviado a la cola: %s en %us", color_str, delay_s);
                    }
                } else {
                    ESP_LOGW(TAG, "Formato incorrecto: %s", line);
                    uart_write_bytes(UART_PORT, "\nERROR: usar COLOR SEGUNDOS. Ej: ROJO 5\n", 42);
                }
            } else {
                if (idx < BUF_SIZE - 1) {
                    line[idx++] = (char)c;
                } else {
                    idx = 0;
                    uart_write_bytes(UART_PORT, "\nERROR: comando demasiado largo\n", 31);
                }
            }
        }
    }
}

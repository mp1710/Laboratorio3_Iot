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

static const char *TAG = "TASK_B"; //Etiqueta para logs
// Función auxiliar para convertir un string de color a una estructura rgb_color_t
static int parse_color(const char *color, rgb_color_t *rgb)
{ 
    if (strcmp(color, "ROJO") == 0) {
        rgb->r = 255;
        rgb->g = 0;
        rgb->b = 0;
        return 1;
    } else if (strcmp(color, "VERDE") == 0) {
        rgb->r = 0;
        rgb->g = 255;
        rgb->b = 0;
        return 1;
    } else if (strcmp(color, "AZUL") == 0) {
        rgb->r = 0;
        rgb->g = 0;
        rgb->b = 255;
        return 1;
    } else if (strcmp(color, "BLANCO") == 0) {
        rgb->r = 255;
        rgb->g = 255;
        rgb->b = 255;
        return 1;
    } else {
        return -1; // Color no reconocido
    }
}


void task_b(void *pvParameters)
{
    // Obtener la cola de comandos desde los parámetros
    QueueHandle_t queue = (QueueHandle_t)pvParameters;

    // Configurar UART
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    // Inicializar UART
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));

    ESP_LOGI(TAG, "Task B iniciada, esperando comandos por UART...");

    uint8_t data[BUF_SIZE];

    while (1) {
        // Leer datos de UART
        int len = uart_read_bytes(UART_PORT, data, BUF_SIZE - 1, portMAX_DELAY);

        if (len > 0) {
            data[len] = '\0'; // Asegurar que el string esté terminado en null
            
            ESP_LOGI(TAG, "Comando recibido: %s", (char *)data);

            // Parsear el comando
            char color_str[20];
            unsigned int delay_s; 
            // Se espera el formato: <COLOR> <DELAY_S>
            if (sscanf((char *)data, "%19s %u", color_str, &delay_s) == 2) {
                // Crear comando para enviar a la cola
                led_command_t cmd;
                cmd.delay_s = delay_s;
                // Convertir el string de color a rgb_color_t
                if (!parse_color(color_str, &cmd.color)) {
                    ESP_LOGW(TAG, "Color no reconocido: %s", color_str);
                    uart_write_bytes(UART_PORT, "Color no reconocido. Use ROJO, VERDE, AZUL o BLANCO.\n", 57);
                    continue;
                }
                // Enviar comando a la cola
                if (xQueueSend(queue, &cmd, portMAX_DELAY) == pdPASS) {
                    char response[64];
                    snprintf(response, sizeof(response), "OK: %s en %us\n", color_str, delay_s);
                    uart_write_bytes(UART_PORT, response, strlen(response));
                    ESP_LOGI(TAG, "Comando enviado a la cola: Color=%s, Delay=%u", color_str, delay_s);
                }
                }   
            } else {
                // El comando no tiene el formato correcto, se avisa al usuario
                ESP_LOGE(TAG, "Formato de comando incorrecto.");
                uart_write_bytes(UART_PORT, "Formato incorrecto. Use: <COLOR> <DELAY_S>\n", 38);
            }
        }
}    
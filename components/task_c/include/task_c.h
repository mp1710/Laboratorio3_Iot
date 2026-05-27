#ifndef TASK_C_H
#define TASK_C_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "shared_types.h"

typedef struct {
    QueueHandle_t command_queue; // Cola para recibir comandos de TASK B
    SemaphoreHandle_t color_mutex; // Mutex para proteger el acceso al color actual
    rgb_color_t *current_color; // Puntero al color actual del LED
} task_c_params_t;

void task_c(void *pvParameters);

#endif // TASK_C_H
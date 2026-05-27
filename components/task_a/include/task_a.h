#ifndef TASK_A_H
#define TASK_A_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "shared_types.h"

typedef struct {
    rgb_color_t *current_color;
    SemaphoreHandle_t color_mutex;
} task_a_params_t;

void task_a(void *pvParameters);

#endif 

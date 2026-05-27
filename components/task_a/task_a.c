#include "task_a.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "rgb_led.h"

static const char *TAG = "TASK_A";

void task_a(void *pvParameters)
{
    task_a_params_t *params = (task_a_params_t *) pvParameters;

    if (params == NULL || params->current_color == NULL || params->color_mutex == NULL) {
        ESP_LOGE(TAG, "Parametros invalidos");
        vTaskDelete(NULL);
        return;
    }

    rgb_color_t color_local;

    ESP_LOGI(TAG, "TASK A iniciada");

    while (1) {
        if (xSemaphoreTake(params->color_mutex, portMAX_DELAY) == pdTRUE) {
            color_local = *(params->current_color);
            xSemaphoreGive(params->color_mutex);
        } else {
            ESP_LOGW(TAG, "No se pudo tomar el mutex");
            continue;
        }

        rgb_led_set_color(color_local.r, color_local.g, color_local.b);
        vTaskDelay(pdMS_TO_TICKS(100));

        rgb_led_off();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

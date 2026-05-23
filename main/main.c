#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "esp_err.h"

#include "rgb_led.h"
#include "task_a.h"
#include "shared_types.h"

static const char *TAG = "MAIN";

static rgb_color_t g_current_color = {
    .r = 255,
    .g = 255,
    .b = 0
};

static SemaphoreHandle_t g_color_mutex = NULL;

void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando app");

    esp_err_t err = rgb_led_init();

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error inicializando RGB LED");
        return;
    }

    g_color_mutex = xSemaphoreCreateMutex();

    if (g_color_mutex == NULL) {
        ESP_LOGE(TAG, "Error creando mutex");
        return;
    }

    static task_a_params_t task_a_params;

    task_a_params.current_color = &g_current_color;
    task_a_params.color_mutex = g_color_mutex;

    BaseType_t result = xTaskCreate(
        task_a,
        "task_a",
        4096,
        &task_a_params,
        tskIDLE_PRIORITY + 1,
        NULL
    );

    if (result != pdPASS) {
        ESP_LOGE(TAG, "Error creando TASK A");
        return;
    }

    ESP_LOGI(TAG, "TASK A creada correctamente");
}

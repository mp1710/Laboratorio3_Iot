#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_err.h"

#include "rgb_led.h"
#include "task_a.h"
#include "task_b.h"
#include "shared_types.h"

static const char *TAG = "MAIN"; //Etiqueta para logs
static QueueHandle_t g_command_queue = NULL; // Cola para comandos recibidos por UART
// Color actual del LED, protegido por un mutex
static rgb_color_t g_current_color = {
    .r = 255,
    .g = 0,
    .b = 0
};
// Mutex para proteger el acceso a g_current_color
static SemaphoreHandle_t g_color_mutex = NULL;

void app_main(void)
{
    // Establecer el nivel de log para ws2812 (viene de led_strip) a WARN para reducir la verbosidad
    esp_log_level_set("ws2812", ESP_LOG_WARN);
    
    ESP_LOGI(TAG, "Iniciando app");

    esp_err_t err = rgb_led_init();

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error inicializando RGB LED");
        return;
    }
    // Crear mutex para proteger el acceso a g_current_color
    g_color_mutex = xSemaphoreCreateMutex();

    if (g_color_mutex == NULL) {
        ESP_LOGE(TAG, "Error creando mutex");
        return;
    }
    // Crear cola para comandos recibidos por UART
    g_command_queue = xQueueCreate(10, sizeof(led_command_t));

    if (g_command_queue == NULL) {
        ESP_LOGE(TAG, "Error creando cola de comandos");
        return;
    }

    // Crear TASK A
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

    // Crear TASK B
    result = xTaskCreate(
        task_b,
        "task_b",
        4096,
        g_command_queue,
        tskIDLE_PRIORITY + 3,
        NULL
    );

    if (result != pdPASS) {
        ESP_LOGE(TAG, "Error creando TASK B");
        return;
    }
    ESP_LOGI(TAG, "TASK B creada correctamente");
}


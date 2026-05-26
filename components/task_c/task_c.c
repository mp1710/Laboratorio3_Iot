#include "task_c.h"

#include "freertos/timers.h"
#include "esp_log.h"

// recibe comandos de la cola y crea un timer one-shot 
// para cambiar el color despues de delay_s
static const char *TAG = "TASK_C"; //Etiqueta para logs

static rgb_color_t *g_current_color = NULL; // Puntero al color actual del LED
static SemaphoreHandle_t g_color_mutex = NULL; // Mutex para proteger el acceso al color actual

// Función de callback del timer, se ejecuta cuando el timer expira
static void timer_callback(TimerHandle_t xTimer)
{
    // Obtener el comando del timer
    rgb_color_t *new_color = (rgb_color_t *)pvTimerGetTimerID(xTimer);

    // Aplicar el nuevo color al LED, protegiendo el acceso con el mutex
    if (new_color == NULL) {
        ESP_LOGE(TAG, "Timer sin color asociado");
        return;
    }

    if (xSemaphoreTake(g_color_mutex, portMAX_DELAY) == pdTRUE) {
        g_current_color->r = new_color->r; // Actualizar el color actual
        g_current_color->g = new_color->g;
        g_current_color->b = new_color->b;

        xSemaphoreGive(g_color_mutex);

        ESP_LOGI(TAG, "Color actualizado: R=%d, G=%d, B=%d", new_color->r, new_color->g, new_color->b);
    }
    vPortFree(new_color); // Liberar la memoria del color
    xTimerDelete(xTimer, 0); // Eliminar el timer
}
        

void task_c(void *pvParameters)
{
    task_c_params_t *params = (task_c_params_t *)pvParameters;

    // Obtener la cola de comandos y el puntero al color actual desde los parámetros
    QueueHandle_t queue = params->command_queue;
    g_current_color = params->current_color;
    g_color_mutex = params->color_mutex;

    // Variable para almacenar el comando recibido
    led_command_t cmd;

    ESP_LOGI(TAG, "Task C iniciada");

    while (1) {

        // Esperar por un comando en la cola
        if (xQueueReceive(queue, &cmd, portMAX_DELAY) == pdTRUE) {

            rgb_color_t *color_copy = pvPortMalloc(sizeof(rgb_color_t)); // Crear una copia del color para el timer

            if (color_copy == NULL) {
                ESP_LOGE(TAG, "Error al asignar memoria para el color");
                continue;
            }
            
            *color_copy = cmd.color; // Copiar el color al nuevo espacio de memoria

            TimerHandle_t timer = xTimerCreate(
                "ColorTimer",
                pdMS_TO_TICKS(cmd.delay_s * 1000), // Convertir segundos a ticks
                pdFALSE, // Timer one-shot
                color_copy, // Pasar el color como ID del timer
                timer_callback // Función de callback del timer
            );

            if (timer == NULL) {
                ESP_LOGE(TAG, "Error al crear el timer");
                vPortFree(color_copy); // Liberar la memoria si no se pudo crear el timer
            }

            if (xTimerStart(timer, 0) != pdPASS) {
                ESP_LOGE(TAG, "Error al iniciar el timer");
                xTimerDelete(timer, 0); // Eliminar el timer si no se pudo iniciar
                vPortFree(color_copy); // Liberar la memoria del color
            }
            ESP_LOGI(TAG, "Timer creado para cambiar en %u segundos", cmd.delay_s);
        }
    }
}

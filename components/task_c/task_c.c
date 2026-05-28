#include "task_c.h"

#include "freertos/timers.h"
#include "esp_log.h"

#define NUM_COLORS 4
// Array para almacenar los timers activos, indexados por color
static TimerHandle_t color_timers[NUM_COLORS] = { NULL };

// Función para mapear un color a un índice de timer
typedef enum {
    COLOR_ROJO = 0,
    COLOR_VERDE,
    COLOR_AZUL,
    COLOR_BLANCO,
    COLOR_INVALID
} color_id_t;

// Función para obtener el ID del color basado en su valor RGB
static color_id_t get_color_id(const rgb_color_t *color) {
    if (color->r == 255 && color->g == 0 && color->b == 0) {
        return COLOR_ROJO;
    }
    if (color->r == 0 && color->g == 255 && color->b == 0) {
        return COLOR_VERDE;
    }
    if (color->r == 0 && color->g == 0 && color->b == 255) {
        return COLOR_AZUL;
    }
    if (color->r == 255 && color->g == 255 && color->b == 255) {
        return COLOR_BLANCO;
    }
    return COLOR_INVALID;
}

// Función para convertir un ID de color a su representación en string (para logs)
static const char *color_to_string(color_id_t id)
{
    switch (id) {
        case COLOR_ROJO:   return "ROJO";
        case COLOR_VERDE:  return "VERDE";
        case COLOR_AZUL:   return "AZUL";
        case COLOR_BLANCO: return "BLANCO";
        default:           return "INVALID";
    }
}

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
    color_id_t id = get_color_id(new_color);

    if (id != COLOR_INVALID) {
        color_timers[id] = NULL;
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
            color_id_t id = get_color_id(&cmd.color);
            
            if (id != COLOR_INVALID &&  // Si ya hay un timer activo para este color, eliminarlo antes de crear uno nuevo
                color_timers[id] != NULL)
            {
                rgb_color_t *old_color =
                    (rgb_color_t *) pvTimerGetTimerID(color_timers[id]);

                if (old_color != NULL) {
                    vPortFree(old_color);
                }

                xTimerDelete(color_timers[id], 0);

                color_timers[id] = NULL;

                ESP_LOGI(TAG,
                    "Timer %s reemplazado por nuevo delay de %u s",
                    color_to_string(id),
                    cmd.delay_s);
            }
            rgb_color_t *color_copy = pvPortMalloc(sizeof(rgb_color_t)); // Crear una copia del color para el timer

            if (color_copy == NULL) {
                ESP_LOGE(TAG, "Error al asignar memoria para el color");
                continue;
            }
            
            *color_copy = cmd.color; // Copiar el color al nuevo espacio de memoria

            color_timers[id] = xTimerCreate(
                "ColorTimer",
                pdMS_TO_TICKS(cmd.delay_s * 1000), // Convertir segundos a ticks
                pdFALSE, // Timer one-shot
                color_copy, // Pasar el color como ID del timer
                timer_callback // Función de callback del timer
            );

            if (color_timers[id] == NULL) {
                ESP_LOGE(TAG, "Error al crear el timer");
                vPortFree(color_copy); // Liberar la memoria si no se pudo crear el timer
            }

            if (xTimerStart(color_timers[id], 0) != pdPASS) {
                ESP_LOGE(TAG, "Error al iniciar el timer");
                xTimerDelete(color_timers[id], 0); // Eliminar el timer si no se pudo iniciar
                vPortFree(color_copy); // Liberar la memoria del color
            }
            ESP_LOGI(TAG, "Timer creado para cambiar en %u segundos", cmd.delay_s);

            // Monitoreo el STACK de la tarea cada vez que recibo un comando para definir el tamaño del stack
            ESP_LOGI(TAG, "Stack libre: %u bytes", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));
        }
    }
}

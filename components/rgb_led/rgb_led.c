#include "rgb_led.h"
#include "led_strip.h"
#include <string.h>

static bool led_on = false;
static char color_actual[16] = "off";

// Función del driver original que se encuentra en components/led_strip/led_strip.c,
// se declara aca para poder utilizarla en este archivo.
extern esp_err_t led_rgb_init(led_strip_t **strip);

// Se declara un puntero a una estructura de tipo led_strip_t, que es la estructura
// que se utiliza para controlar el LED RGB, se inicializa en NULL.
static led_strip_t *strip = NULL;

// Esta función se encarga de establecer el color del LED RGB, recibe como parámetros
// los valores de rojo, verde y azul, y retorna un valor de tipo esp_err_t que indica si se realizo o no.
esp_err_t rgb_led_init(void)
{
    return led_rgb_init(&strip);
}

// Esta función es una función auxiliar que se encarga de establecer el color del LED RGB,
// recibe como parámetros los valores de rojo, verde y azul, y retorna un valor de tipo esp_err_t que indica si se realizo o no.
static esp_err_t set_color(uint8_t r, uint8_t g, uint8_t b)
{
    if (!strip) return ESP_FAIL;
    strip->set_pixel(strip, 0, r, g, b);
    led_on = true;
    return strip->refresh(strip, 100);
}

// Estas funciones utilizan la funcion anterior para setear el color a rojo/verde/azul
esp_err_t rgb_led_rojo(void)
{
    strcpy(color_actual, "rojo");
    return set_color(255, 0, 0);
}

esp_err_t rgb_led_verde(void)
{
    strcpy(color_actual, "verde");
    return set_color(0, 255, 0);
}

esp_err_t rgb_led_azul(void)
{
    strcpy(color_actual, "azul");
    return set_color(0, 0, 255);
}

esp_err_t rgb_led_amarillo(void)
{
    strcpy(color_actual, "amarillo");
    return set_color(255,255,0);
}

esp_err_t rgb_led_cyan(void)
{
    strcpy(color_actual, "cyan");
    return set_color(0,255,255);
}

esp_err_t rgb_led_magenta(void)
{
    strcpy(color_actual, "magenta");
    return set_color(255,0,255);
}

esp_err_t rgb_led_blanco(void)
{
    strcpy(color_actual, "blanco");
    return set_color(255,255,255);
}

// Esta función se encarga de apagar el LED RGB, para esto utiliza
// la función clear() de la estructura strip, que se encarga de apagar todos los LEDs,
// retorna un valor de tipo esp_err_t que indica si se realizo o no.
esp_err_t rgb_led_off(void)
{
    if (!strip) return ESP_FAIL;
    led_on = false;
    strcpy(color_actual, "off");
    return strip->clear(strip, 100);
}

esp_err_t rgb_led_set_color(uint8_t r, uint8_t g, uint8_t b)
{
    return set_color(r, g, b);
}

bool rgb_led_is_on(void)
{
    return led_on;
}

const char *rgb_led_get_color(void)
{
    return color_actual;
}

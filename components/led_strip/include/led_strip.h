// Copyright 2019 Espressif Systems (Shanghai) PTE LTD
// Licensed under the Apache License, Version 2.0

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include <stdint.h>

/**
 * @brief LED Strip Type
 */
typedef struct led_strip_s led_strip_t;

/**
 * @brief LED Strip Device Type
 */
typedef void *led_strip_dev_t;

/**
 * @brief Declare of LED Strip Type
 */
struct led_strip_s {
    esp_err_t (*set_pixel)(led_strip_t *strip, uint32_t index, uint32_t red, uint32_t green, uint32_t blue);
    esp_err_t (*refresh)(led_strip_t *strip, uint32_t timeout_ms);
    esp_err_t (*clear)(led_strip_t *strip, uint32_t timeout_ms);
    esp_err_t (*del)(led_strip_t *strip);
};

/**
 * @brief LED Strip Configuration Type
 */
typedef struct {
    uint32_t max_leds;      /*!< Maximum LEDs in a single strip */
    led_strip_dev_t dev;   /*!< LED strip device (e.g. RMT channel, PWM channel, etc) */
} led_strip_config_t;

/**
 * @brief Default configuration for LED strip
 */
#define LED_STRIP_DEFAULT_CONFIG(number, dev_hdl) \
    {                                           \
        .max_leds = number,                     \
        .dev = dev_hdl,                         \
    }

led_strip_t *led_strip_new_rmt_ws2812(const led_strip_config_t *config);
esp_err_t led_rgb_init(led_strip_t **strip);

#ifdef __cplusplus
}
#endif

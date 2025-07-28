#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "esp_log.h"


typedef struct keyboard {
    int32_t row_gpio[4];
    int32_t col_gpio[4];
} mkbd_keyboard;

void keyboard_init(mkbd_keyboard *keyboard);
uint64_t pin_mask(gpio_num_t pins[4]);
void scan_key();

#endif
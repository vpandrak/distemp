
#include "keyboard.h"

static const char *TAG = "mkbd";

uint64_t pin_mask(gpio_num_t pins[4]){
    uint64_t result = 0;
    for (int i = 0; i < len(pins); i++)
    {
        result |= (1ULL << pins[i]);
    }
    return result;
}

void keyboard_init(mkbd_keyboard *keyboard){
    
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = pin_mask(keyboard->row_gpio);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&io_conf);

    io_conf.pin_bit_mask = pin_mask(keyboard->col_gpio);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&io_conf);    

    xTaskCreate(scan_key, "Scan Key Task", 2048, &keyboard, 2, NULL);
 
}


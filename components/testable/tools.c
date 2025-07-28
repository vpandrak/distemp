#include "tools.h"

uint64_t pin_mask(gpio_num_t pins[4]){
    uint64_t result = 0;
    for (int i = 0; i < len(pins); i++)
    {
        result |= (1ULL << pins[i]);
    }
    return result;
}
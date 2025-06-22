#ifndef SENSORS_H
#define SENSORS_H

#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include <rom/ets_sys.h>

#define DHT11_OK                0
#define DHT11_TIMEOUT           -1
#define DHT11_ERROR_CHECKSUM    -2

#define DHT11_PIN GPIO_NUM_23

esp_err_t dht11_set_pin(uint8_t pin);
int8_t dht11_read();
uint8_t dht11_get_humidity();
uint8_t dht11_get_temp();

#endif
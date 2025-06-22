#include "sensors.h"


#define NUM_OF_BITS 40
#define NUM_OF_BYTES 5

static const char *TAG = "SENSORS";

uint8_t humidity = 0;
uint8_t temp = 0;
uint8_t dht11_pin;

int dht11_get_signal_level(int usTimeOut, int state){

    int uSec = 0;

    while (gpio_get_level(dht11_pin) == state)
    {
        if (uSec > usTimeOut)
        {
            ESP_LOGD(TAG, "Timeout error!");
            return -1;
        }
        uSec++;
        ets_delay_us(1);
    }
    return uSec;
    
}

esp_err_t dht11_set_pin(uint8_t pin){

    ESP_LOGD(TAG, "Setting data pin...");

    dht11_pin = pin;
    return ESP_OK;
}

int8_t dht11_read(){

    ESP_LOGD(TAG, "Reading sensor...");

    uint8_t bytes[NUM_OF_BYTES];
    uint8_t bitIdx = 7;
    uint8_t byteIdx = 0;

    for (int i = 0; i < NUM_OF_BYTES; i++)
    {
        bytes[i] = 0;
    }

    // Отдача команды на устройство о начале работы
    gpio_set_direction(dht11_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(dht11_pin, 0);
    ets_delay_us(25000);
    gpio_set_level(dht11_pin, 1);
    ets_delay_us(25);

    gpio_set_direction(dht11_pin, GPIO_MODE_INPUT);
    
    int uSec = 0;

    uSec = dht11_get_signal_level(85, 0);
    if (uSec < 0)
    {
        return DHT11_TIMEOUT;
    }

    uSec = dht11_get_signal_level(85, 1);
    if (uSec < 0)
    {
        return DHT11_TIMEOUT;
    }

    for (int i = 0; i < NUM_OF_BITS; i++){
        uSec = dht11_get_signal_level(56, 0);
        if (uSec < 0)
        {
            return DHT11_TIMEOUT;
        }

        uSec = dht11_get_signal_level(75, 1);
        if (uSec < 0)
        {
            return DHT11_TIMEOUT;
        }

        if(uSec > 40){
            bytes[byteIdx] |= (1<< bitIdx);
        }
        if (bitIdx == 0){
            bitIdx = 7;
            byteIdx++;
        } else{
            bitIdx--;
        }
    }

    uint8_t sum  = 0;

    for (int i=0; i<(NUM_OF_BYTES-1); i++){
        sum += bytes[i];
    }

    if (bytes[4] == (sum & 0xFF)){
        humidity = bytes[0];
        temp = bytes[2];
        return DHT11_OK;
    }

    ESP_LOGD(TAG, "Checksum error!");
    return DHT11_ERROR_CHECKSUM; 
}

uint8_t dht11_get_humidity(){
    return humidity;
}

uint8_t dht11_get_temp(){
    return temp;
}

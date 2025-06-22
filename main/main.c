#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "drivers/sensors.h"
#include "drivers/display.h"
#include "drivers/font.h"

//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#define ACTIVATED GPIO_NUM_25
#define DEACTIVATED GPIO_NUM_26
#define LED   GPIO_NUM_4



static QueueHandle_t MSt_evt_queue = NULL;


void MSt_btn_isr_handler(void* arg){
    uint32_t gpio_num = (uint32_t) arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(MSt_evt_queue, &gpio_num, &xHigherPriorityTaskWoken);
    if(xHigherPriorityTaskWoken){
        portYIELD_FROM_ISR();
    }
}


void MSt_get_data(void* pvParameters){
    uint32_t io_num;
    (void) pvParameters;

    while (1)
    {
        if (xQueueReceive(MSt_evt_queue, &io_num, portMAX_DELAY))
        {
            printf("GPIO[%ld] intr, val %d\n", io_num, gpio_get_level(io_num));
        }
        
    }
    
}

void MSt_server(void* pvParameters){

    while (1)
    {
        if (dht11_read() == DHT11_OK){
            ESP_LOGD("MAIN"," Температура воздуха: %d\n", dht11_get_temp());
            ESP_LOGD("MAIN"," Влажность воздуха: %d\n", dht11_get_humidity());
        } else{
            ESP_LOGE("MAIN", "error sensor");
        }
        
        vTaskDelay(4000/portTICK_PERIOD_MS);
    }
    
}

void app_main(void)
{

    DISPLAY_T oled;
    int center, top, bottom;
    char lineChar[20];
    oled.flip = true;
    i2c_master_init(&oled,SDA_I2C_PIN, SCL_I2C_PIN, GPIO_NUM_11);
    display_init(&oled, 128, 64);
    display_clear_screen(&oled, false);
    display_DISPLAY_Text(&oled, 0, "Hello", 5, false);
    display_DISPLAY_Text(&oled, 1, "ABCDEFGHIJKLMNOP", 16, false);
    vTaskDelay(3000/portTICK_PERIOD_MS);
    gpio_config_t io_conf = {};

    // Инициализация портов на выход
    io_conf.pin_bit_mask = (1ULL << LED);
    io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    // Инициализация портов на вход
    io_conf.pin_bit_mask = (1ULL << ACTIVATED | 1ULL << DEACTIVATED);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);

    MSt_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    if (MSt_evt_queue ==NULL){
        printf("queue is not created\n");
    }

    gpio_isr_handler_add(ACTIVATED, MSt_btn_isr_handler, (void*) ACTIVATED);
    gpio_isr_handler_add(DEACTIVATED, MSt_btn_isr_handler, (void*) DEACTIVATED);

    xTaskCreate(MSt_get_data, "get_data Task", 2048, NULL, 2, NULL);
    xTaskCreate(MSt_server, "Server Task", 1024, NULL, 2, NULL);
    dht11_set_pin(DHT11_PIN);
    while (1)
    {
        //gpio_set_level(LED, !gpio_get_level(LED));
        //printf("%d\n", dht11_get_temp());
        //ESP_LOGD("MAIN", "main task");
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
    
}
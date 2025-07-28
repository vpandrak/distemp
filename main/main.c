#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "drivers/keyboard.h"
#include "drivers/sensors.h"
#include "drivers/display.h"
#include "drivers/font.h"

#define NUM_ROW 4 
static gpio_num_t pins_row[] = {GPIO_NUM_35, GPIO_NUM_32, GPIO_NUM_25, GPIO_NUM_33};
#define NUM_COL 4 
static gpio_num_t pins_col[] = {GPIO_NUM_26, GPIO_NUM_27, GPIO_NUM_14, GPIO_NUM_12};

static uint32_t mkbd_layout[NUM_ROW][NUM_COL] = {
            {0x01, 0x02, 0x03, 0x0A},        
            {0x04, 0x05, 0x06, 0x0B},        
            {0x07, 0x08, 0x09, 0x0C},        
            {0x0F, 0x00, 0x0E, 0x0D},
};


// Struct for sending temperature and humidity via queue between sensor task and display task
typedef struct  __attribute__((packed)){
    uint8_t temperature;
    uint8_t humidity;
} climat;



static QueueHandle_t MSt_evt_queue = NULL;
static QueueHandle_t queue_mkbd = NULL;

// Function for sending struct data to display
void MSt_send_to_display(void* pvParameters){
    // Copy struct oled to this function
    DISPLAY_T* oled = (DISPLAY_T*) pvParameters;
    climat climat;
    char temp[8];
    char hum[7];
    while (1)
    {
        if (xQueueReceive(MSt_evt_queue, &climat, portMAX_DELAY))
        {
            ESP_LOGD("MAIN"," Температура воздуха: %d\n", climat.temperature);
            ESP_LOGD("MAIN"," Влажность воздуха: %d\n", climat.humidity);
            // join rext with value of temp and humidity
            sniprintf(temp, sizeof(temp), "Temp %d", climat.temperature);
            sniprintf(hum, sizeof(hum), "Hum %d", climat.humidity);
            display_DISPLAY_Text(oled, 0,temp, 8, false);
            display_DISPLAY_Text(oled, 1, hum, 7, false);
        } else{
            ESP_LOGE("MAIN", "Сообщение не получено из очереди");
        }
        
    }
    
}

void MSt_get_data(void* pvParameters){
    climat clim;
    while (1)
    {
        if (dht11_read() == DHT11_OK){
            clim.temperature = dht11_get_temp();
            clim.humidity =  dht11_get_humidity();
            if(xQueueSend(MSt_evt_queue, &clim,portMAX_DELAY) == pdPASS){
                ESP_LOGD("MAIN","message added to the queue");
            }
        } else{
            ESP_LOGE("MAIN", "error sensor");
        }
        
        vTaskDelay(4000/portTICK_PERIOD_MS);
    }
    
}

static void MSt_get_key(void* pvParameters){
    
    
}

void app_main(void)
{
    DISPLAY_T oled;
    i2c_master_init(&oled,SDA_I2C_PIN, SCL_I2C_PIN, GPIO_NUM_11);
    display_init(&oled, 128, 64);
    display_clear_screen(&oled, false);
    display_DISPLAY_Text(&oled, 3, "ver 0.1", 7, false);
    MSt_evt_queue = xQueueCreate(10, sizeof(climat));
    if (MSt_evt_queue ==NULL){
        printf("queue is not created\n");
    }
    mkbd_keyboard mkbd = {
        .row_gpio = pins_row,
        .col_gpio = pins_col, 
    };
  
    keyboard_init(&mkbd);


    dht11_set_pin(DHT11_PIN);
    xTaskCreate(MSt_send_to_display, "Send_to_display Task", 2048, &oled, 2, NULL);
    xTaskCreate(MSt_get_data, "Get_data Task", 2048, NULL, 2, NULL);
    while (1)
    {
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
    
}
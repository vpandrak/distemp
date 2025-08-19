#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"


#define RX_BUF_SIZE 1024
#define TXD_PIN (GPIO_NUM_1)
#define RXD_PIN (GPIO_NUM_3)

static const char *UART_TAG = "UART";

void rx_task(void* param){
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    ESP_LOGI(UART_TAG, "reseave started");
    while (1){
        const int rxBytes = uart_read_bytes(UART_NUM_0, data, RX_BUF_SIZE, 1000/portTICK_PERIOD_MS);
        if (rxBytes > 0){
        data[rxBytes] = 0;
        ESP_LOGI(UART_TAG, "Mes: %s", (char*)data);
        }
    }
    free(data);
}
void uart_init(){
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, RX_BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_LOGI(UART_TAG, "UART initalizated");

    xTaskCreate(&rx_task, "RX Task", 2048, NULL, 2, NULL);
}


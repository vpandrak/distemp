#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "display.h"

#define TAG "I2C_DRiVER"

#define I2C_TICKS_TO_WAIT 100


void i2c_master_init(DISPLAY_T *oled, int16_t sda, int16_t scl, int16_t reset){
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = sda,
        .scl_io_num = scl,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_device_config_t dev_config ={
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = OLED_I2C_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle));

    oled->address = OLED_I2C_ADDR;
    oled->flip = false;
    oled->i2c_num = I2C_MASTER_NUM;
    oled->i2c_bus_handle = bus_handle;
    oled->i2c_dev_handle = dev_handle;
}


void i2c_device_add(DISPLAY_T * oled, i2c_port_t i2c_num, int16_t reset, uint16_t i2c_address){

    ESP_LOGD(TAG, "New i2c driver is used");
    ESP_LOGD(TAG, "Will not insall i2c master driver");

#if 0
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .i2c_port = i2c_num,
        .scl_io_num = SCL_I2C_PIN,
        .sda_io_num = SDA_I2C_PIN,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t i2c_bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &i2c_bus_handle));
#endif

    i2c_device_config_t dev_cfg ={
        .dev_addr_length = I2C_ADDR_BIT_7,
        .device_address = i2c_address,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    i2c_master_dev_handle_t i2c_dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(oled->i2c_bus_handle, &dev_cfg, &i2c_dev_handle));

    if (reset >= 0){
        gpio_reset_pin(reset);
        gpio_set_direction(reset, GPIO_MODE_OUTPUT);
        gpio_set_level(reset, 0);
        vTaskDelay(50/portTICK_PERIOD_MS);
        gpio_set_level(reset, 1);
    }
    oled->address = i2c_address;
    oled->flip = false;
    oled->i2c_num = i2c_num;
    oled->i2c_dev_handle = i2c_dev_handle;
}    

void i2c_init(DISPLAY_T *oled, int width, int height){
    oled->width = width;
    oled->height = height;
    oled->pages = 8;

    if (oled->height == 32) {
        oled->pages = 4;
    }
    uint8_t buffer[27];
    uint8_t ind_buffer = 0;
    buffer[ind_buffer++] = OLED_CONTROL_BYTE_CMD_STREAM;
    buffer[ind_buffer++] = OLED_CMD_DISPLAY_OFF;
    buffer[ind_buffer++] = OLED_CMD_SET_MUX_RATIO;

    if (oled->height == 64) {
        buffer[ind_buffer++] = 0x3F;
    }
    if (oled->height == 32) {
        buffer[ind_buffer++] = 0x1F;
    }
    buffer[ind_buffer++] = OLED_CMD_SET_DISPLAY_OFFSET;
    buffer[ind_buffer++] = 0x00;
    buffer[ind_buffer++] = OLED_CMD_SET_DISPLAY_START_LINE;

    if (oled->flip){
        buffer[ind_buffer++] = OLED_CMD_SET_SEGMENT_REMAP_0; 
    } else{
        buffer[ind_buffer++] = OLED_CMD_SET_SEGMENT_REMAP_1;
    }

    buffer[ind_buffer++] = OLED_CMD_SET_COM_SCAN_MODE;
    buffer[ind_buffer++] = OLED_CMD_SET_DISPLAY_CLK_DIV;
    buffer[ind_buffer++] = 0x80;
    buffer[ind_buffer++] = OLED_CMD_SET_COM_PIN_MAP;

    if (oled->height == 64) {
        buffer[ind_buffer++] = 0x12;
    }
    if (oled->height == 32) {
        buffer[ind_buffer++] = 0x02;
    }

    buffer[ind_buffer++] = OLED_CMD_SET_CONTRAST;
    buffer[ind_buffer++] = 0xff;
    buffer[ind_buffer++] = OLED_CMD_DISPLAY_RAM;
    buffer[ind_buffer++] = OLED_CMD_SET_VCOMH_DESELCT;
    buffer[ind_buffer++] = 0x40;
    buffer[ind_buffer++] = OLED_CMD_SET_MEMORY_ADDR_MODE;
    buffer[ind_buffer++] = OLED_CMD_SET_PAGE_ADDR_MODE;
    buffer[ind_buffer++] = 0x00;
    buffer[ind_buffer++] = 0x10;
    buffer[ind_buffer++] = OLED_CMD_SET_CHARGE_PUMP;
    buffer[ind_buffer++] = 0x14;
    buffer[ind_buffer++] = OLED_CMD_DEACTIVE_SCROLL;
    buffer[ind_buffer++] = OLED_CMD_DISPLAY_NORMAL;
    buffer[ind_buffer++] = OLED_CMD_DISPLAY_ON;
// какая-то фигня возможно стоит переписать
    esp_err_t res;
    res = i2c_master_transmit(oled->i2c_dev_handle, buffer, ind_buffer, I2C_TICKS_TO_WAIT);
    if (res == ESP_OK) {
        ESP_LOGI(TAG, "OLED configured sucessfully");
    } else {    
        ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)", oled->address, oled->i2c_num, res, esp_err_to_name(res));
    }

}

void i2c_display_image(DISPLAY_T * oled, int page, int seg, const uint8_t * images, int width){
    if (page >= oled->pages) return;
    if (seg >= oled->width) return;

    int _seg = seg;
    uint8_t columnLow = _seg & 0x0F;
    uint8_t columnHigh = (_seg >> 4) &0x0F;

    int _page = page;
    if (oled->flip){
        _page = (oled->pages - page) - 1;
    }

    uint8_t *buffer;
    buffer = malloc(width<4 ?4:width+1);
    if (buffer == NULL)
    {
        ESP_LOGE(TAG, "malloc fail");
        return;
    }

    int buffer_index = 0;
    buffer[buffer_index++] = OLED_CONTROL_BYTE_CMD_STREAM;
    buffer[buffer_index++] = (0x00 + columnLow);
    buffer[buffer_index++] = (0x10 + columnHigh);
    buffer[buffer_index++] = 0xB0 | _page;

    esp_err_t res;
    res = i2c_master_transmit(oled->i2c_dev_handle, buffer, buffer_index, I2C_TICKS_TO_WAIT);
    if (res != ESP_OK){
        ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)", oled->address, oled->i2c_num, res, esp_err_to_name(res));
    }

    buffer[0] = OLED_CONTROL_BYTE_DATA_STREAM;
    memcpy(&buffer[1], images, width);

    res = i2c_master_transmit(oled->i2c_dev_handle, buffer, width+1, I2C_TICKS_TO_WAIT);
    if (res != ESP_OK){
        ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)", oled->address, oled->i2c_num, res, esp_err_to_name(res));
    }
    free(buffer);
    
}

void i2c_contrast(DISPLAY_T * oled, int contrast) {
    uint8_t _contrast  = contrast;
    if (contrast < 0x00) _contrast = 0;
    if (contrast > 0xFF) _contrast = 0xFF;

    uint8_t buffer[3];
    int buffer_index = 0;
    buffer[buffer_index++] = OLED_CONTROL_BYTE_CMD_STREAM;
    buffer[buffer_index++] = OLED_CMD_SET_CONTRAST;
    buffer[buffer_index++] = _contrast;

    esp_err_t res = i2c_master_transmit(oled->i2c_dev_handle, buffer, 3, I2C_TICKS_TO_WAIT);
    if (res != ESP_OK){
        ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)", oled->address, oled->i2c_num, res, esp_err_to_name(res));
    }

}

void i2c_hardware_scroll(DISPLAY_T * oled, display_scroll_type_t scroll){
    uint8_t buffer[11];
    int buffer_index = 0;
    buffer[buffer_index++] = OLED_CONTROL_BYTE_CMD_STREAM;

    if (scroll == SCROLL_RIGHT){
        buffer[buffer_index++] = OLED_CMD_HORIZONTAL_RIGHT;
        buffer[buffer_index++] = 0x00;
        buffer[buffer_index++] = 0x00;
        buffer[buffer_index++] = 0x07;
        buffer[buffer_index++] = 0x07;
        buffer[buffer_index++] = 0x00;
        buffer[buffer_index++] = 0xFF;
        buffer[buffer_index++] = OLED_CMD_ACTIVE_SCROLL;
    }


    if (scroll == SCROLL_LEFT){
        buffer[buffer_index++] = OLED_CMD_HORIZONTAL_LEFT;
        buffer[buffer_index++] = 0x00;
        buffer[buffer_index++] = 0x00;
        buffer[buffer_index++] = 0x07;
        buffer[buffer_index++] = 0x07;
        buffer[buffer_index++] = 0x00;
        buffer[buffer_index++] = 0xFF;
        buffer[buffer_index++] = OLED_CMD_ACTIVE_SCROLL;
    }

    if (scroll == SCROLL_DOWN){
        buffer[buffer_index++] = OLED_CMD_CONTINUOUS_SCROLL;
        buffer[buffer_index++] = 0x00;
        buffer[buffer_index++] = 0x00;
        buffer[buffer_index++] = 0x07;
        buffer[buffer_index++] = 0x00;
        buffer[buffer_index++] = 0x3F;
        buffer[buffer_index++] = OLED_CMD_VERTICAL;
        buffer[buffer_index++] = 0x00;
        if (oled->height == 64){
            buffer[buffer_index++] = 0x40;
        }
        if (oled->height == 32){
            buffer[buffer_index++] = 0x20;
        }
        buffer[buffer_index++] = OLED_CMD_ACTIVE_SCROLL;
    }

    if (scroll == SCROLL_UP){
        buffer[buffer_index++] = OLED_CMD_CONTINUOUS_SCROLL;
        buffer[buffer_index++] = 0x00;
        buffer[buffer_index++] = 0x00;
        buffer[buffer_index++] = 0x07;
        buffer[buffer_index++] = 0x00;
        buffer[buffer_index++] = 0x01;
        buffer[buffer_index++] = OLED_CMD_VERTICAL;
        buffer[buffer_index++] = 0x00;
        if (oled->height == 64){
            buffer[buffer_index++] = 0x40;
        }
        if (oled->height == 32){
            buffer[buffer_index++] = 0x20;
        }
        buffer[buffer_index++] = OLED_CMD_ACTIVE_SCROLL;
    }

    if (scroll == SCROLL_STOP){
        buffer[buffer_index] = OLED_CMD_DEACTIVE_SCROLL;
    }

    esp_err_t res = i2c_master_transmit(oled->i2c_dev_handle, buffer, buffer_index, I2C_TICKS_TO_WAIT);
    if (res != ESP_OK){
        ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)", oled->address, oled->i2c_num, res, esp_err_to_name(res));
    }
}
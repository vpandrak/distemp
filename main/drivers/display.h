#ifndef DISPLAY_H
#define DISPLAY_H

#include "string.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "font.h"


typedef enum {
    SCROLL_RIGHT = 1,
    SCROLL_LEFT  =2,
    SCROLL_DOWN = 3,
    SCROLL_UP = 4,
    PAGE_SCROLL_DOWN =5,
    PAGE_SCROLL_UP = 6,
    SCROLL_STOP = 7
} display_scroll_type_t;

typedef struct {
    bool valid;
    int segLen;
    uint8_t segs[128];
} PAGE_t;


typedef struct {
    int address;
    int width;
    int height;
    int pages;
    int dc;
    bool scEnable;
    int scStart;
    int scEnd;
    int scDirection;
    PAGE_t page[8];
    bool flip;
    i2c_port_t i2c_num;
    i2c_master_bus_handle_t i2c_bus_handle;
    i2c_master_dev_handle_t i2c_dev_handle;
} DISPLAY_T;



// Пины отвечающие за протокол i2c
#define SCL_I2C_PIN GPIO_NUM_22
#define SDA_I2C_PIN GPIO_NUM_21

#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_MASTER_TIMEOUS_MS 1000

#define OLED_I2C_ADDR  0x3C 

// Control byte
#define OLED_CONTROL_BYTE_CMD_SINGLE    0x80
#define OLED_CONTROL_BYTE_CMD_STREAM    0x00
#define OLED_CONTROL_BYTE_DATA_STREAM   0x40

#define OLED_CMD_SET_CONTRAST           0x81    // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM            0xA4
#define OLED_CMD_DISPLAY_ALLON          0xA5
#define OLED_CMD_DISPLAY_NORMAL         0xA6
#define OLED_CMD_DISPLAY_INVERTED       0xA7
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_DISPLAY_ON             0xAF

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE   0x20    // follow with 0x00 = HORZ mode = Behave like a KS108 graphic LCD
#define OLED_CMD_SET_PAGE_ADDR_MODE     0x02    // Page Addressing Mode
#define OLED_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP_0    0xA0   
#define OLED_CMD_SET_SEGMENT_REMAP_1    0xA1
#define OLED_CMD_SET_MUX_RATIO          0xA8    // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE      0xC8    
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3    // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP        0xDA    // follow with 0x12
#define OLED_CMD_NOP                    0xE3    // NOP

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV    0xD5    // follow with 0x80
#define OLED_CMD_SET_PRECHARGE          0xD9    // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT      0xDB    // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP        0x8D    // follow with 0x14

// Scrolling Command
#define OLED_CMD_HORIZONTAL_RIGHT       0x26
#define OLED_CMD_HORIZONTAL_LEFT        0x27
#define OLED_CMD_CONTINUOUS_SCROLL      0x29
#define OLED_CMD_DEACTIVE_SCROLL        0x2E
#define OLED_CMD_ACTIVE_SCROLL          0x2F
#define OLED_CMD_VERTICAL               0xA3

void display_init(DISPLAY_T * oled, int width, int height);
int display_get_width(DISPLAY_T * oled);
int display_get_height(DISPLAY_T * oled);
int display_get_pages(DISPLAY_T * oled);
void display_show_buffer(DISPLAY_T * oled);
void display_set_buffer(DISPLAY_T * oled);
void display_get_buffer(DISPLAY_T * oled);
void display_set_page(DISPLAY_T * oled, int page, const uint8_t * buffer);
void display_get_page(DISPLAY_T * oled, int page, uint8_t * buffer);
void display_display_image(DISPLAY_T * oled, int page, int seg, const uint8_t * images, int width);
void display_DISPLAY_Text(DISPLAY_T * oled, int page, const char * text, int text_len, bool invert);
void display_DISPLAY_Text_box1(DISPLAY_T * oled, int page, int seg, const char * text, int box_width, int text_len, bool invert, int delay);
void display_DISPLAY_Text_box2(DISPLAY_T * oled, int page, int seg, const char * text, int box_width, int text_len, bool invert, int delay);
void display_DISPLAY_Text_x3(DISPLAY_T * oled, int page, const char * text, int text_len, bool invert);
void display_clear_screen(DISPLAY_T * oled, bool invert);
void display_clear_line(DISPLAY_T * oled, int page, bool invert);
void display_contrast(DISPLAY_T * oled, int contrast);
void display_software_scroll(DISPLAY_T * oled, int start, int end);
void display_scroll_text(DISPLAY_T * oled, const char * text, int text_len, bool invert);
void display_scroll_clear(DISPLAY_T * oled);
void display_hardware_scroll(DISPLAY_T * oled, display_scroll_type_t scroll);
void display_wrap_arround(DISPLAY_T * oled, display_scroll_type_t scroll, int start, int end, int8_t delay);
void display_bitmaps(DISPLAY_T * oled, int xpos, int ypos, const uint8_t * bitmap, int width, int height, bool invert);
void display_bitmaps(DISPLAY_T * oled, int xpos, int ypos, const uint8_t * bitmap, int width, int height, bool invert);
void display_pixel(DISPLAY_T * oled, int xpos, int ypos, bool invert);
void display_line(DISPLAY_T * oled, int x1, int y1, int x2, int y2,  bool invert);
void display_circle(DISPLAY_T * oled, int x0, int y0, int r, unsigned int opt, bool invert);
void display_disc(DISPLAY_T * oled, int x0, int y0, int r, unsigned int opt, bool invert);
void display_cursor(DISPLAY_T * oled, int x0, int y0, int r, bool invert);
void display_invert(uint8_t *buf, size_t blen);
void display_flip(uint8_t *buf, size_t blen);
uint8_t display_copy_bit(uint8_t src, int srcBits, uint8_t dst, int dstBits);
uint8_t display_rotate_byte(uint8_t ch1);
void display_fadeout(DISPLAY_T * oled);
void display_rotate_image(uint8_t *image, bool flip);
void display_display_rotate_text(DISPLAY_T * oled, int seg, const char * text, int text_len, bool invert);
void display_dump(DISPLAY_T oled);
void display_dump_page(DISPLAY_T * oled, int page, int seg);

void i2c_master_init(DISPLAY_T * oled, int16_t sda, int16_t scl, int16_t reset);
void i2c_oledice_add( DISPLAY_T * oled, i2c_port_t i2c_num, int16_t reset, uint16_t i2c_address);
void i2c_init(DISPLAY_T * oled, int width, int height);
void i2c_display_image(DISPLAY_T * oled, int page, int seg, const uint8_t * images, int width);
void i2c_contrast(DISPLAY_T * oled, int contrast);
void i2c_hardware_scroll(DISPLAY_T * oled, display_scroll_type_t scroll);



#endif
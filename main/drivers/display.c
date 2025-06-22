#include "display.h"


static const char *TAG = "DISPLAY";

DISPLAY_T oled;

#define PACK8 __attribute__((aligned( __alignof__( uint8_t ) ), packed))

typedef union out_column_t{
    uint32_t u32;
    uint8_t u8[4];
} PACK8 out_column_t;


void display_init(DISPLAY_T *oled, int width, int height){
    i2c_init(oled, width, height);

    for(int i = 0; i < oled->pages; i++){
        memset(oled->page[i].segs, 0, 128);
    }
}

int display_get_width (DISPLAY_T * oled){
    return oled->width;
}

int display_get_height(DISPLAY_T * oled){
    return oled->height;
}

int display_get_pages(DISPLAY_T * oled){
    return oled->pages;
}


void display_show_buffer(DISPLAY_T * oled){
    for (int page = 0; page < oled->pages; page++)
    {
        i2c_display_image(oled, page, 0, oled->page[page].segs, oled->width);
    }
    
}





void display_clear_screen(DISPLAY_T * oled, bool invert){
    char space[16];
    memset(space, 0x00, sizeof(space));
    for (int page = 0; page < oled->pages; page++){
        display_DISPLAY_Text(oled, page, space, sizeof(space), invert);
    }
}

void display_display_image(DISPLAY_T *oled, int page, int seg, const uint8_t * images, int width){
    i2c_display_image(oled, page, seg, images, width);
    memcpy(&oled->page[page].segs[seg], images, width);
}


void display_DISPLAY_Text(DISPLAY_T * oled, int page, const char * text, int text_len, bool invert){
    if (page >= oled-> pages) return;
    int _text_len = text_len;
    if (_text_len >16) _text_len = 16;

    int seg = 0;
    uint8_t image[8];
    for (int i = 0; i<_text_len; i++){
        memcpy(image, font8x8_basic_tr[(uint8_t)text[i]], 8);
        if (invert) display_invert(image, 8);
        display_display_image(oled, page, seg, image, 8);
        seg = seg + 8;
    }
}

void display_invert(uint8_t *buf, size_t blen){
    uint8_t wk;
    for (int i = 0; i < blen; i++)
    {
        wk = buf[i];
        buf[i] = ~wk;
    }
    
}

void display_flip(uint8_t *buf , size_t blen){
    for (int i = 0; i < blen; i++)
    {
        buf[i] = display_rotate_byte(buf[i]);
    }
    
}

uint8_t display_rotate_byte(uint8_t ch1){
    uint8_t ch2 = 0;
    for (int i = 0; i < 8; i++)
    {
        ch2 = (ch2 << 1) +(ch1 &0x01);
        ch1 = ch1 >> 1;
    }
    return ch2;
    
}
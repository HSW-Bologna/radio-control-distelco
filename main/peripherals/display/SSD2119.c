/**
 * @file SSD2119.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "SSD2119.h"
#include "display.h"
#include "lvgl.h"
//#if USE_NT7534

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/*********************
 *      DEFINES
 *********************/

#define SSD2119_CMD_MODE  0
#define SSD2119_DATA_MODE 1

#define SSD2119_HOR_RES 320
#define SSD2119_VER_RES 240

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void ssd2119_command(uint8_t cmd);
static void ssd2119_data(uint16_t data);
static void ssd2119_set_window(int x1, int x2, int y1, int y2);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void ssd2119_init(void) {
    display_set_reset(1);
    vTaskDelay(pdMS_TO_TICKS(30));
    display_set_reset(0);
    vTaskDelay(pdMS_TO_TICKS(30));
    display_set_reset(1);
    vTaskDelay(pdMS_TO_TICKS(30));

    ssd2119_command(0x0028);     // VCOM OTP
    ssd2119_data(0x0006);        // Page 55-56 of SSD2119 datasheet

    ssd2119_command(0x0000);     // start Oscillator
    ssd2119_data(0x0001);        // Page 36 of SSD2119 datasheet

    ssd2119_command(0x0001);     // Driver Output Control
    ssd2119_data(0x72EF);        // Page 36-39 of SSD2119 datasheet

    ssd2119_command(0x0002);     // LCD Driving Waveform Control
    ssd2119_data(0x0600);        // Page 40-42 of SSD2119 datasheet

    ssd2119_command(0x0003);     // Power Control 1
    ssd2119_data(0x6A38);        // Page 43-44 of SSD2119 datasheet

    ssd2119_command(0x0010);     // Sleep mode
    ssd2119_data(0x0000);        // Page 49 of SSD2119 datasheet

    ssd2119_command(0x0011);     // Entry Mode
    ssd2119_data(0x6430);        // Page 50-52 of SSD2119 datasheet

    ssd2119_command(0x0007);     // Display Control
    ssd2119_data(0x0033);        // Page 45 of SSD2119 datasheet

    ssd2119_command(0x0025);     // Frame Frequency Control
    ssd2119_data(0xd000);        // Page 53 of SSD2119 datasheet

    ssd2119_command(0x000B);     // Frame Cycle Control
    ssd2119_data(0x5308);        // Page 45 of SSD2119 datasheet

    // Gamma Control (R30h to R3Bh) -- Page 56 of SSD2119 datasheet
    ssd2119_command(0x0030);
    ssd2119_data(0x0000);

    ssd2119_command(0x0031);
    ssd2119_data(0x0101);

    ssd2119_command(0x0032);
    ssd2119_data(0x0100);

    ssd2119_command(0x0033);
    ssd2119_data(0x0707);

    ssd2119_command(0x0034);
    ssd2119_data(0x0707);

    ssd2119_command(0x0035);
    ssd2119_data(0x0305);

    ssd2119_command(0x0036);
    ssd2119_data(0x0707);

    ssd2119_command(0x0037);
    ssd2119_data(0x0201);

    ssd2119_command(0x003A);
    ssd2119_data(0x1200);

    ssd2119_command(0x003B);
    ssd2119_data(0x0900);

    ssd2119_command(0x000C);     // Power Control 2
    ssd2119_data(0x0004);        // Page 47 of SSD2119 datasheet

    vTaskDelay(pdMS_TO_TICKS(5));

    ssd2119_command(0x000D);     // Power Control 3
    ssd2119_data(0x0009);        // Page 48 of SSD2119 datasheet

    vTaskDelay(pdMS_TO_TICKS(5));

    ssd2119_command(0x001E);     // Power Control 5
    ssd2119_data(0x0068);        // Page 53 of SSD2119 datasheet

    vTaskDelay(pdMS_TO_TICKS(5));

    ssd2119_command(0x000E);     // Power Control 4
    ssd2119_data(0x2700);        // Page 48 of SSD2119 datasheet

    vTaskDelay(pdMS_TO_TICKS(5));

    ssd2119_command(0x0026);     // Analog setting
    ssd2119_data(0x7c00);        // Page 54 of SSD2119 datasheet

    ssd2119_command(0x0027);     // Analog setting
    ssd2119_data(0x006d);        // Page 54 of SSD2119 datasheet


    ssd2119_command(0x000F);     // Gate Scan Position
    ssd2119_data(0x0000);        // Page 49 of SSD2119 datasheet

    ssd2119_command(0x004E);     // Ram Address Set
    ssd2119_data(0x0000);        // Page 58 of SSD2119 datasheet

    ssd2119_command(0x004F);     // Ram Address Set
    ssd2119_data(0x0000);        // Page 58 of SSD2119 datasheet

    // ssd2119_command(0x0012);     // Sleep mode
    // ssd2119_data(0x08D9);        // Page 49 of SSD2119 datasheet

    vTaskDelay(pdMS_TO_TICKS(150));

    ssd2119_command(0x0022);     // RAM data write/read
}


void ssd2119_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    uint8_t *buffer = (uint8_t *)color_p;

    if (area->x2 < 0)
        return;
    if (area->y2 < 0)
        return;
    if (area->x1 > SSD2119_HOR_RES - 1)
        return;
    if (area->y1 > SSD2119_VER_RES - 1)
        return;

    lv_coord_t act_x1 = area->x1 < 0 ? 0 : area->x1;
    lv_coord_t act_y1 = area->y1 < 0 ? 0 : area->y1;
    lv_coord_t act_x2 = area->x2 > SSD2119_HOR_RES - 1 ? SSD2119_HOR_RES - 1 : area->x2;
    lv_coord_t act_y2 = area->y2 > SSD2119_VER_RES - 1 ? SSD2119_VER_RES - 1 : area->y2;

    ssd2119_set_window(act_x1, act_x2, act_y1, act_y2);
    ssd2119_command(0x0022);     // RAM data write/read

    int width    = act_x2 - act_x1 + 1;
    int height   = act_y2 - act_y1 + 1;
    int totbytes = width * 2 * height;
    int index    = 0;

    display_set_data_command(SSD2119_DATA_MODE);
    while (index < totbytes) {
        int chunk = totbytes - index > MAX_TRANSFER_SIZE ? MAX_TRANSFER_SIZE : totbytes - index;
        display_send_data(&buffer[index], chunk);
        index += chunk;
    }
    if (disp_drv)
        lv_disp_flush_ready(disp_drv);

    /*display_set_data_command(SSD2119_DATA_MODE);
    for (int i = 0; i < height; i++)
    {
        display_send_data(&buffer[i * width * 2], width * 2);
    }
    lv_disp_flush_ready(disp_drv);*/
}

/**
 * Write a command to the NT7534
 * @param cmd the command
 */
static void ssd2119_command(uint8_t cmd) {
    display_set_data_command(SSD2119_CMD_MODE);
    display_send_data(&cmd, 1);
}

/**
 * Write data to the NT7534
 * @param data the data
 */
static void ssd2119_data(uint16_t data) {
    uint8_t high, low;
    high = data >> 8;
    low  = data & 0xFF;
    display_set_data_command(SSD2119_DATA_MODE);
    // LV_DRV_DISP_SPI_WR_ARRAY(buffer, 2);
    display_send_data(&high, 1);
    display_send_data(&low, 1);
}

static void ssd2119_set_window(int x1, int x2, int y1, int y2) {
    ssd2119_command(0x004E);     // RAM address set
    ssd2119_data(x1);            // Page 58 of SSD2119 datasheet
    ssd2119_command(0x004F);     // RAM address set
    ssd2119_data(y1);            // Page 58 of SSD2119 datasheet

    ssd2119_command(0x0044);        // Vertical RAM address position
    ssd2119_data(y2 << 8 | y1);     // Page 57 of SSD2119 datasheet
    ssd2119_command(0x0045);        // Horizontal RAM address position
    ssd2119_data(x1);               // Page 57 of SSD2119 datasheet
    ssd2119_command(0x0046);        // Horizontal RAM address position
    ssd2119_data(x2);               // Page 57 of SSD2119 datasheet
}

void ssd2119_test(uint16_t color)
// void ssd2119_test(uint16_t color, int x1, int y1, int x2, int y2)
{
    // uint8_t buffer[2] = {0xF8, 0};
    uint8_t buffer[SSD2119_HOR_RES * 2] = {0};
    for (int i = 0; i < SSD2119_HOR_RES; i++) {
        buffer[i * 2]     = color >> 8;
        buffer[i * 2 + 1] = color & 0xFF;
    }
    ssd2119_set_window(19, 82, 23, 30);
    ssd2119_command(0x0022);     // RAM data write/read

    display_set_data_command(SSD2119_DATA_MODE);
    for (int i = 0; i < SSD2119_VER_RES; i++) {
        display_send_data(&buffer[0], SSD2119_HOR_RES * 2);

        /*for (int j = 0; j < SSD2119_VER_RES; j++)
        {
            display_set_data_command(SSD2119_DATA_MODE);
            display_send_data(&buffer[0], 2);
        }*/
    }
}

//#endif

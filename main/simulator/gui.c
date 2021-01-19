#include <unistd.h>
#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" issue*/
#include <SDL2/SDL.h>
#include <time.h>
#include <sys/time.h>

#include "display/monitor.h"
#include "indev/mouse.h"
#include "indev/mousewheel.h"
#include "lvgl.h"
#include "lv_themes/lv_theme_material.h"

#include "view/view.h"
#include "view/theme/style.h"
#include "view/theme/theme.h"
#include "model/model.h"
#include "simulator_controller.h"

#define DISP_BUF_SIZE (40 * LV_HOR_RES_MAX)

static lv_color_t buf[DISP_BUF_SIZE];

lv_indev_t *gui_init(void) {
    lv_init();

    style_init();
    lv_theme_set_act(theme_init(LV_THEME_DEFAULT_COLOR_PRIMARY, LV_THEME_DEFAULT_COLOR_SECONDARY,
                                LV_THEME_MATERIAL_FLAG_DARK | LV_THEME_MATERIAL_FLAG_NO_FOCUS,
                                LV_THEME_DEFAULT_FONT_SMALL, LV_THEME_DEFAULT_FONT_NORMAL,
                                LV_THEME_DEFAULT_FONT_SUBTITLE, LV_THEME_DEFAULT_FONT_TITLE));

    /* Add a display
     * Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
    monitor_init();

    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE); /*Initialize the display buffer*/

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);       /*Basic initialization*/
    disp_drv.flush_cb = monitor_flush; /*Used when `LV_VDB_SIZE != 0` in lv_conf.h (buffered drawing)*/
    disp_drv.buffer   = &disp_buf;
    // disp_drv.disp_fill = monitor_fill;      /*Used when `LV_VDB_SIZE == 0` in lv_conf.h (unbuffered drawing)*/
    // disp_drv.disp_map = monitor_map;        /*Used when `LV_VDB_SIZE == 0` in lv_conf.h (unbuffered drawing)*/
    lv_disp_drv_register(&disp_drv);

    /* Add the mouse as input device
     * Use the 'mouse' driver which reads the PC's mouse*/
    mouse_init();
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv); /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb =
        mouse_read; /*This function will be called periodically (by the library) to get the mouse position and state*/
    lv_indev_t *indev = lv_indev_drv_register(&indev_drv);

    return indev;
}


void controller_manage_gui(model_t *model) {
    view_message_t umsg;
    view_event_t   event;

    lv_task_handler();

    while (view_get_next_msg(model, &umsg, &event)) {
        controller_process_msg(umsg.cmsg, model);
        view_process_msg(umsg.vmsg, model);
    }
}
#include "lvgl.h"

#include "peripherals/display/display.h"
#include "peripherals/display/SSD2119.h"
#include "peripherals/display/tsc2046.h"
#include "utils/utils.h"
#include "view/view.h"


void controller_gui_init(model_t *model) {
    display_init();
    ssd2119_init();
    tsc2046_init();
    lv_init();

    static lv_color_t    buf[DISP_COLOR_BUF_SIZE];
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, buf, NULL, DISP_COLOR_BUF_SIZE);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.buffer   = &disp_buf;
    disp_drv.hor_res  = 320;
    disp_drv.ver_res  = 240;
    disp_drv.flush_cb = ssd2119_flush;
    lv_disp_drv_register(&disp_drv);

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = tsc2046_touch_read;
    indev_drv.type    = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);

    view_init(model);
}


void controller_manage_gui(model_t *model) {
    view_message_t umsg;
    view_event_t   event;

    lv_task_handler();

    while (view_get_next_msg(model, &umsg, &event)) {
        // controller_process_msg(umsg.cmsg, model, get_millis());
        view_process_msg(umsg.vmsg, model);
    }
}
#include "lvgl.h"

#include "peripherals/display/display.h"
#include "peripherals/display/SSD2119.h"
#include "peripherals/display/tsc2046.h"
#include "controller.h"
#include "utils/utils.h"
#include "config/app_config.h"
#include "view/view.h"
#include "view/theme/theme.h"
#include "view/theme/style.h"

#include "gel/timer/timecheck.h"


void controller_manage_gui(model_t *model) {
    static unsigned long refreshts     = 0;
    static unsigned long screensaverts = 0;
    static int           screensaver   = 0;

    view_message_t umsg;
    view_event_t   event;

    lv_task_handler();

    while (view_get_next_msg(model, &umsg, &event)) {
        if (tsc2046_touch_found()) {
            // As long as someone is using the display do not refresh it (or reset it)
            refreshts     = get_millis();
            screensaverts = get_millis();

#if 1
            if (screensaver == 1) {
                screensaver = 0;
                ssd2119_init();
                display_set_backlight(BASE_BACKLIGHT);
            } else if (screensaver == 2) {
                screensaver = 0;
            }
#endif
        }

        controller_process_msg(umsg.cmsg, model);
        view_process_msg(umsg.vmsg, model);
    }

#if 1
    if (is_expired(refreshts, get_millis(), 1000UL)) {
        lv_obj_invalidate(lv_scr_act());
        refreshts = get_millis();
    }
#endif

#if 1
    switch (screensaver) {
        case 0:
            if (is_expired(screensaverts, get_millis(), IDLE_DISPLAY_TIMEOUT)) {
                screensaver = 1;
                display_set_backlight(0);
                refreshts     = get_millis();
                screensaverts = get_millis();
            } else if (is_expired(refreshts, get_millis(), 1000UL)) {
                lv_obj_invalidate(lv_scr_act());
                refreshts = get_millis();
            }
            break;

        case 1:
            if (is_expired(screensaverts, get_millis(), SCREENSAVER_OFF_TIMEOUT)) {
                screensaver = 2;
                ssd2119_init();
                display_set_backlight(BASE_BACKLIGHT);
                refreshts     = get_millis();
                screensaverts = get_millis();
            }
            break;

        case 2:
            if (is_expired(screensaverts, get_millis(), SCREENSAVER_SHOW_TIMEOUT)) {
                screensaver = 1;
                display_set_backlight(0);
                refreshts     = get_millis();
                screensaverts = get_millis();
            } else if (is_expired(refreshts, get_millis(), 1000UL)) {
                lv_obj_invalidate(lv_scr_act());
                refreshts = get_millis();
            }
            break;
    }
#endif
}
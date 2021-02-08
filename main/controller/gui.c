#include "lvgl.h"

#include "peripherals/display/display.h"
#include "peripherals/display/SSD2119.h"
#include "peripherals/display/tsc2046.h"
#include "controller.h"
#include "utils/utils.h"
#include "view/view.h"
#include "view/theme/theme.h"
#include "view/theme/style.h"

#include "gel/timer/timecheck.h"


void controller_manage_gui(model_t *model) {
    static unsigned long timestamp = 0;
    ;
    view_message_t umsg;
    view_event_t   event;

    lv_task_handler();

    while (view_get_next_msg(model, &umsg, &event)) {
        controller_process_msg(umsg.cmsg, model);
        view_process_msg(umsg.vmsg, model);
    }

    if (is_expired(timestamp, get_millis(), 800)) {
        lv_obj_invalidate(lv_scr_act());
        timestamp = get_millis();
    }
}
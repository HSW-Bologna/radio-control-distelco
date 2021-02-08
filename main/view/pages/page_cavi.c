#include <stdlib.h>
#include <stdio.h>

#include "lvgl.h"
#include "gel/pagemanager/page_manager.h"

#include "view/common.h"
#include "view/view.h"
#include "model/model.h"


enum {
    BACK_BTN_ID,
    CABLE_CB_ID,
};

struct page_data {
    lv_obj_t *cables[4];
};

static void *create_page(model_t *model, void *extra) {
    struct page_data *data = malloc(sizeof(struct page_data));
    return data;
}


static void open_page(model_t *model, void *args) {
    struct page_data *data = args;
    lv_obj_t *        back = view_common_back_button(BACK_BTN_ID);

    lv_obj_t *title = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_text_font(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_title());
    lv_label_set_text(title, "Cavi");
    lv_obj_align(title, back, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

    for (size_t i = 0; i < 4; i++) {
        lv_obj_t *cb         = lv_checkbox_create(lv_scr_act(), NULL);
        char      string[32] = {0};
        snprintf(string, 32, "Cavo radiante %i", i + 1);
        lv_checkbox_set_text(cb, string);
        lv_checkbox_set_checked(cb, model_is_cable_enabled(model, i));
        lv_obj_align(cb, NULL, LV_ALIGN_IN_TOP_MID, 0, 64 + 42 * i);
        view_register_default_callback_number(cb, CABLE_CB_ID, i);
        data->cables[i] = cb;
    }
}


static view_message_t process_page_event(model_t *model, void *arg, view_event_t event) {
    struct page_data *data = arg;
    (void)data;
    view_message_t msg = {0};

    switch (event.code) {
        case VIEW_EVENT_CODE_CUSTOM:
            break;

        case VIEW_EVENT_CODE_LVGL:
            if (event.lvgl.lv_event == LV_EVENT_CLICKED) {
                switch (event.lvgl.data->id) {
                    case BACK_BTN_ID:
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_SAVE_CONFIG;
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        break;

                    default:
                        break;
                }
            } else if (event.lvgl.lv_event == LV_EVENT_VALUE_CHANGED) {
                switch (event.lvgl.data->id) {
                    case CABLE_CB_ID:
                        model_set_cable(model, event.lvgl.data->number,
                                        lv_checkbox_is_checked(data->cables[event.lvgl.data->number]));
                        break;

                    default:
                        break;
                }
            }
            break;

        default:
            break;
    }

    return msg;
}


const pman_page_t page_cavi = {
    .create        = create_page,
    .open          = open_page,
    .close         = view_close_all,
    .destroy       = view_destroy_all,
    .process_event = process_page_event,
};
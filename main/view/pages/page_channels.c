#include <stdlib.h>

#include "lv_misc/lv_color.h"
#include "lv_widgets/lv_dropdown.h"
#include "lvgl.h"
#include "gel/pagemanager/page_manager.h"

#include "view/common.h"
#include "view/view.h"
#include "model/model.h"


enum {
    BACK_BTN_ID,
    CHANNEL_BTN_ID,
};


struct page_data {};


static lv_obj_t *create_channel(lv_obj_t *root, model_t *model, int num) {
    lv_obj_t *button = lv_btn_create(root, NULL);
    lv_btn_set_layout(button, LV_LAYOUT_OFF);
    lv_obj_set_width(button, 200);

    lv_obj_t *lbl = lv_label_create(button, NULL);
    lv_label_set_text_fmt(lbl, "Canale %i", num + 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_LEFT_MID, 10, 0);

    lv_obj_t *led = lv_led_create(button, NULL);
    lv_obj_set_style_local_bg_color(led, LV_LED_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_obj_set_size(led, 24, 24);
    lv_obj_align(led, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);
    model_is_channel_configured(model, num) ? lv_led_on(led) : lv_led_off(led);

    return button;
}


static void *create_page(model_t *model, void *extra) {
    struct page_data *data = malloc(sizeof(struct page_data));
    return data;
}


static void open_page(model_t *model, void *args) {
    // struct page_data *data = args;

    lv_obj_t *back = view_common_back_button(BACK_BTN_ID);

    lv_obj_t *title = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_text_font(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_title());
    lv_label_set_text(title, "Canali");
    lv_obj_align(title, back, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

    lv_obj_t *page = lv_page_create(lv_scr_act(), NULL);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES - 50);
    lv_obj_align(page, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    lv_obj_t *c1 = create_channel(page, model, 0);
    lv_obj_align(c1, NULL, LV_ALIGN_IN_TOP_MID, 0, 2);

    lv_obj_t *c2 = create_channel(page, model, 1);
    lv_obj_align(c2, c1, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);

    lv_obj_t *c3 = create_channel(page, model, 2);
    lv_obj_align(c3, c2, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);

    lv_obj_t *c4 = create_channel(page, model, 3);
    lv_obj_align(c4, c3, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);

    lv_obj_t *c5 = create_channel(page, model, 4);
    lv_obj_align(c5, c4, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);

    view_register_default_callback_number(c1, CHANNEL_BTN_ID, 0);
    view_register_default_callback_number(c2, CHANNEL_BTN_ID, 1);
    view_register_default_callback_number(c5, CHANNEL_BTN_ID, 4);
    view_register_default_callback_number(c4, CHANNEL_BTN_ID, 3);
    view_register_default_callback_number(c3, CHANNEL_BTN_ID, 2);
}


static view_message_t process_page_event(model_t *model, void *arg, view_event_t event) {
    struct page_data *data = arg;
    (void)data;
    view_message_t msg = {0};

    switch (event.code) {
        case VIEW_EVENT_CODE_OPEN:
            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_UPDATE;
            break;

        case VIEW_EVENT_CODE_CUSTOM:
            break;

        case VIEW_EVENT_CODE_LVGL:
            if (event.lvgl.lv_event == LV_EVENT_CLICKED) {
                switch (event.lvgl.data->id) {
                    case BACK_BTN_ID:
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_SAVE_CONFIG;
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        break;

                    case CHANNEL_BTN_ID:
                        msg.vmsg.code  = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE_EXTRA;
                        msg.vmsg.page  = &page_channel_settings;
                        msg.vmsg.extra = (void *)(uintptr_t)event.lvgl.data->number;
                        break;
                }
            }
            break;

        default:
            break;
    }

    return msg;
}


static void update_page(model_t *model, void *arg) {
    // struct page_data *data = arg;
}


const pman_page_t page_channels = {
    .create        = create_page,
    .open          = open_page,
    .update        = update_page,
    .close         = view_close_all,
    .destroy       = view_destroy_all,
    .process_event = process_page_event,
};
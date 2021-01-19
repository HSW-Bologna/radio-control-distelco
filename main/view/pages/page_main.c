#include <stdlib.h>

#include "lvgl.h"
#include "gel/pagemanager/page_manager.h"

#include "view/view.h"
#include "view/theme/style.h"
#include "model/model.h"

enum {
    CONFIG_BTN_ID,
};

struct page_data {
    lv_obj_t *leds;
};


static void update_canali(struct page_data *data, model_t *model) {
    lv_obj_clean(data->leds);
    int count = 0;

    for (size_t i = 0; i < MAX_CHANNELS; i++) {
        if (model_is_channel_configured(model, i)) {
            lv_obj_t *cont = lv_cont_create(data->leds, NULL);
            // lv_cont_set_layout(cont, LV_LAYOUT_OFF);
            lv_obj_set_size(cont, 60, 60);

            lv_obj_t *led = lv_led_create(cont, NULL);
            lv_obj_set_size(led, 32, 32);
            lv_obj_align(led, NULL, LV_ALIGN_IN_TOP_MID, 0, 4);
            lv_obj_set_style_local_bg_color(led, LV_LED_PART_MAIN, LV_STATE_DEFAULT,
                                            model_is_channel_ok(model, i) ? LV_COLOR_GREEN : LV_COLOR_RED);
            lv_obj_set_style_local_shadow_color(led, LV_LED_PART_MAIN, LV_STATE_DEFAULT,
                                                model_is_channel_ok(model, i) ? LV_COLOR_GREEN : LV_COLOR_RED);
            lv_led_on(led);

            lv_obj_t *name = lv_label_create(cont, NULL);
            lv_label_set_long_mode(name, LV_LABEL_LONG_CROP);
            lv_obj_set_width(name, 48);
            lv_label_set_text(name, model_get_channel_name(model, i));
            lv_obj_align(name, led, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

            count++;
        }
    }

    lv_obj_set_size(data->leds, 70 * count, 80);
}


static void *create_page(model_t *model, void *extra) {
    struct page_data *data = malloc(sizeof(struct page_data));
    return data;
}


static void open_page(model_t *model, void *args) {
    struct page_data *data = args;

    lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(cont, LV_HOR_RES, 40);
    lv_obj_align(cont, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

    lv_obj_t *settings = lv_btn_create(cont, NULL);
    lv_obj_set_size(settings, 32, 32);
    lv_obj_t *lbl = lv_label_create(settings, NULL);
    lv_label_set_text(lbl, LV_SYMBOL_SETTINGS);
    lv_obj_align(settings, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);

    cont = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES - 40);
    lv_obj_align(cont, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    lv_obj_t *canali = lv_cont_create(cont, NULL);
    lv_cont_set_layout(canali, LV_LAYOUT_COLUMN_MID);
    lv_obj_set_size(canali, LV_HOR_RES, 120);
    lv_obj_align(canali, NULL, LV_ALIGN_IN_TOP_MID, 0, 4);
    lv_obj_add_style(canali, LV_CONT_PART_MAIN, &style_transparent_cont);

    lv_label_set_text(lv_label_create(canali, NULL), "Canali");

    lv_obj_t *leds = lv_cont_create(canali, NULL);
    lv_cont_set_layout(leds, LV_LAYOUT_PRETTY_MID);
    lv_obj_add_style(leds, LV_CONT_PART_MAIN, &style_transparent_cont);
    data->leds = leds;

    view_register_default_callback(settings, CONFIG_BTN_ID);
}


static view_message_t process_page_event(model_t *model, void *arg, view_event_t event) {
    struct page_data *data = arg;
    (void)data;
    view_message_t msg = {0};

    switch (event.code) {
        case VIEW_EVENT_CODE_MODEL_UPDATE:
            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_UPDATE;
            break;

        case VIEW_EVENT_CODE_CUSTOM:
            break;

        case VIEW_EVENT_CODE_LVGL:
            if (event.lvgl.lv_event == LV_EVENT_CLICKED) {
                switch (event.lvgl.data->id) {
                    case CONFIG_BTN_ID:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                        msg.vmsg.page = &page_menu;
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


static void update_page(model_t *model, void *args) {
    struct page_data *data = args;
    update_canali(data, model);
}


const pman_page_t page_main = {
    .create        = create_page,
    .open          = open_page,
    .update        = update_page,
    .close         = view_close_all,
    .destroy       = view_destroy_all,
    .process_event = process_page_event,
};
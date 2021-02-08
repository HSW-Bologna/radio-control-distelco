#include <stdlib.h>

#include "config/app_config.h"
#include "lvgl.h"
#include "gel/pagemanager/page_manager.h"

#include "view/common.h"
#include "view/view.h"
#include "model/model.h"


enum {
    BACK_BTN_ID,
    CHANNELS_BTN_ID,
    CABLES_BTN_ID,
    SETTINGS_BTN_ID,
    TEST_BTN_ID,
};

struct page_data {};

static void *create_page(model_t *model, void *extra) {
    struct page_data *data = malloc(sizeof(struct page_data));
    return data;
}


static void open_page(model_t *model, void *args) {
    lv_obj_t *canali = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_size(canali, 160, 40);

    lv_obj_t *lbl = lv_label_create(canali, NULL);
    lv_label_set_text(lbl, "Canali");
    lv_obj_align(canali, NULL, LV_ALIGN_IN_TOP_MID, 0, 40);

    lv_obj_t *cavi = lv_btn_create(lv_scr_act(), canali);
    lbl            = lv_label_create(cavi, NULL);
    lv_label_set_text(lbl, "Cavi");
    lv_obj_align(cavi, canali, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);

    lv_obj_t *impostazioni = lv_btn_create(lv_scr_act(), cavi);
    lbl                    = lv_label_create(impostazioni, NULL);
    lv_label_set_text(lbl, "Impostazioni");
    lv_obj_align(impostazioni, cavi, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);

    lv_obj_t *test = lv_btn_create(lv_scr_act(), cavi);
    lbl            = lv_label_create(test, NULL);
    lv_label_set_text(lbl, "Test");
    lv_obj_align(test, impostazioni, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);


    lv_obj_t *fw = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_text_font(fw, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_small());
    lv_obj_set_style_local_text_color(fw, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_label_set_text_fmt(fw, "v%s", APP_CONFIG_FIRMWARE_VERSION);
    lv_obj_align(fw, NULL, LV_ALIGN_IN_TOP_RIGHT, -2, 2);

    view_register_default_callback(canali, CHANNELS_BTN_ID);
    view_register_default_callback(cavi, CABLES_BTN_ID);
    view_register_default_callback(impostazioni, SETTINGS_BTN_ID);
    view_register_default_callback(test, TEST_BTN_ID);
    view_common_back_button(BACK_BTN_ID);
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
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        break;

                    case CHANNELS_BTN_ID:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                        msg.vmsg.page = &page_channels;
                        break;

                    case CABLES_BTN_ID:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                        msg.vmsg.page = &page_cavi;
                        break;

                    case SETTINGS_BTN_ID:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                        msg.vmsg.page = &page_settings;
                        break;

                    case TEST_BTN_ID:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                        msg.vmsg.page = &page_test;
                        break;
                }
            }
            break;

        default:
            break;
    }

    return msg;
}


const pman_page_t page_menu = {
    .create        = create_page,
    .open          = open_page,
    .close         = view_close_all,
    .destroy       = view_destroy_all,
    .process_event = process_page_event,
};
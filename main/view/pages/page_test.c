#include <stdlib.h>
#include <stdio.h>

#include "lv_misc/lv_area.h"
#include "lv_widgets/lv_dropdown.h"
#include "lvgl.h"
#include "gel/pagemanager/page_manager.h"

#include "view/common.h"
#include "view/view.h"
#include "model/model.h"


enum {
    BACK_BTN_ID,
    DATA_DD_ID,
    SEND_BTN_ID,
};


struct page_data {
    uint8_t   registers[10];
    lv_obj_t *label;
    lv_obj_t *dd;
};



static void *create_page(model_t *model, void *extra) {
    struct page_data *data = malloc(sizeof(struct page_data));
    memset(data, 0, sizeof(struct page_data));
    return data;
}


static void open_page(model_t *model, void *args) {
    struct page_data *data = args;

    lv_obj_t *back = view_common_back_button(BACK_BTN_ID);

    lv_obj_t *title = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_text_font(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_title());
    lv_label_set_text(title, "Test");
    lv_obj_align(title, back, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

    char      string[32] = {0};
    lv_obj_t *val        = lv_dropdown_create(lv_scr_act(), NULL);
    lv_dropdown_clear_options(val);
    for (int i = 0; i < 16; i++) {
        snprintf(string, 32, "0x%02X", i);
        lv_dropdown_add_option(val, string, i);
    }
    lv_obj_align(val, NULL, LV_ALIGN_IN_TOP_LEFT, 5, 60);
    data->dd = val;

    lv_obj_t *send = lv_btn_create(lv_scr_act(), NULL);
    lv_label_set_text(lv_label_create(send, NULL), "Invia");
    lv_obj_align(send, val, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

    lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(label, NULL, LV_ALIGN_IN_LEFT_MID, 5, 0);
    data->label = label;

    view_register_default_callback(val, DATA_DD_ID);
    view_register_default_callback(send, SEND_BTN_ID);
}


static view_message_t process_page_event(model_t *model, void *arg, view_event_t event) {
    struct page_data *data = arg;
    (void)data;
    view_message_t msg = {0};

    switch (event.code) {
        case VIEW_EVENT_CODE_TEST_MANAGEMENT_RESPONSE:
            memcpy(data->registers, event.management_registers, 10);
            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_UPDATE;
            break;

        case VIEW_EVENT_CODE_OPEN:
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

                    case SEND_BTN_ID:
                        msg.cmsg.code     = VIEW_CONTROLLER_COMMAND_MANAGEMENT_SEND;
                        msg.cmsg.data_reg = lv_dropdown_get_selected(data->dd);
                }
            } else if (event.lvgl.lv_event == LV_EVENT_VALUE_CHANGED) {
                switch (event.lvgl.data->id) {}
            }
            break;

        default:
            break;
    }

    return msg;
}


static void update_page(model_t *model, void *arg) {
    struct page_data *data = arg;

    lv_label_set_text_fmt(data->label, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", data->registers[0],
                          data->registers[1], data->registers[2], data->registers[3], data->registers[4],
                          data->registers[5], data->registers[6], data->registers[7], data->registers[8],
                          data->registers[9]);
}


const pman_page_t page_test = {
    .create        = create_page,
    .open          = open_page,
    .update        = update_page,
    .close         = view_close_all,
    .destroy       = view_destroy_all,
    .process_event = process_page_event,
};
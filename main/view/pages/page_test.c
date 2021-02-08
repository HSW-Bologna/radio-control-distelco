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
    RELE_BTN_ID,
};


struct page_data {
    uint8_t   registers[14];
    lv_obj_t *btns[4];
    lv_obj_t *label;
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

    lv_obj_t *btn1 = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_t *lbl  = lv_label_create(btn1, NULL);
    lv_btn_set_checkable(btn1, 1);
    lv_label_set_text(lbl, "01");
    lv_obj_align(btn1, NULL, LV_ALIGN_IN_TOP_LEFT, 8, 50);

    lv_obj_t *btn2 = lv_btn_create(lv_scr_act(), NULL);
    lbl            = lv_label_create(btn2, NULL);
    lv_btn_set_checkable(btn2, 1);
    lv_label_set_text(lbl, "02");
    lv_obj_align(btn2, NULL, LV_ALIGN_IN_TOP_RIGHT, -8, 50);

    lv_obj_t *btn3 = lv_btn_create(lv_scr_act(), NULL);
    lbl            = lv_label_create(btn3, NULL);
    lv_btn_set_checkable(btn3, 1);
    lv_label_set_text(lbl, "03");
    lv_obj_align(btn3, NULL, LV_ALIGN_IN_TOP_LEFT, 8, 50 * 2);

    lv_obj_t *btn4 = lv_btn_create(lv_scr_act(), NULL);
    lbl            = lv_label_create(btn4, NULL);
    lv_btn_set_checkable(btn4, 1);
    lv_label_set_text(lbl, "04");
    lv_obj_align(btn4, NULL, LV_ALIGN_IN_TOP_RIGHT, -8, 50 * 2);

    lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(label, 1);
    lv_obj_align(label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -2);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(label, LV_HOR_RES);
    data->label = label;

    data->btns[0] = btn4;
    data->btns[1] = btn3;
    data->btns[2] = btn2;
    data->btns[3] = btn1;

    view_register_default_callback_number(btn1, RELE_BTN_ID, 3);
    view_register_default_callback_number(btn2, RELE_BTN_ID, 2);
    view_register_default_callback_number(btn3, RELE_BTN_ID, 1);
    view_register_default_callback_number(btn4, RELE_BTN_ID, 0);
}


static view_message_t process_page_event(model_t *model, void *arg, view_event_t event) {
    struct page_data *data = arg;
    (void)data;
    view_message_t msg = {0};

    switch (event.code) {
        case VIEW_EVENT_CODE_MODEL_UPDATE:
            memcpy(data->registers, model->spi_received, 14);
            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_UPDATE;
            break;

        case VIEW_EVENT_CODE_OPEN:
            msg.cmsg.code = VIEW_CONTROLLER_COMMAND_TEST;
            msg.cmsg.test = 1;
            break;

        case VIEW_EVENT_CODE_CUSTOM:
            break;

        case VIEW_EVENT_CODE_LVGL:
            if (event.lvgl.lv_event == LV_EVENT_CLICKED) {
                switch (event.lvgl.data->id) {
                    case BACK_BTN_ID:
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_CODE_SAVE_CONFIG;
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_TEST;
                        msg.cmsg.test = 0;
                        break;
                }
            } else if (event.lvgl.lv_event == LV_EVENT_VALUE_CHANGED) {
                switch (event.lvgl.data->id) {
                    case RELE_BTN_ID:
                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_TEST_RELE;
                        msg.cmsg.rele = event.lvgl.data->number;
                        msg.cmsg.level =
                            lv_btn_get_state(data->btns[event.lvgl.data->number]) == LV_BTN_STATE_CHECKED_RELEASED;
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
    struct page_data *data = arg;

    lv_label_set_text_fmt(data->label, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                          data->registers[0], data->registers[1], data->registers[2], data->registers[3],
                          data->registers[4], data->registers[5], data->registers[6], data->registers[7],
                          data->registers[8], data->registers[9], data->registers[10], data->registers[11],
                          data->registers[12], data->registers[13]);
}


const pman_page_t page_test = {
    .create        = create_page,
    .open          = open_page,
    .update        = update_page,
    .close         = view_close_all,
    .destroy       = view_destroy_all,
    .process_event = process_page_event,
};
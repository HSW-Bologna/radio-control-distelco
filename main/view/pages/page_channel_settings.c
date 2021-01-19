#include <stdlib.h>

#include "lv_misc/lv_color.h"
#include "lv_widgets/lv_dropdown.h"
#include "lvgl.h"
#include "gel/pagemanager/page_manager.h"

#include "view/common.h"
#include "view/view.h"
#include "model/model.h"


#define FOCUS_NONE   0
#define FOCUS_MASTER 1
#define FOCUS_MINION 2


enum {
    BACK_BTN_ID,
    MASTER_BTN_ID,
    MINION_BTN_ID,
    ADD_MASTER_BTN_ID,
    ADD_MINION_BTN_ID,
    DEL_MASTER_BTN_ID,
    DEL_MINION_BTN_ID,
};


struct page_data {
    lv_obj_t *page;
    size_t    channel;
    int       focus;
};


#include "esp_log.h"


static void update_device_list(struct page_data *data, model_t *model) {
    lv_page_clean(data->page);
    lv_obj_t *tail = NULL;

    for (size_t i = 0; i < model_get_masters_in_channel(model, data->channel); i++) {
        lv_obj_t *btn = lv_btn_create(data->page, NULL);
        lv_obj_set_width(btn, 200);
        lv_obj_t *lbl = lv_label_create(btn, NULL);
        lv_label_set_text_fmt(lbl, "Master %i", i);
        view_register_default_callback_number(btn, MASTER_BTN_ID, i);

        if (tail)
            lv_obj_align(btn, tail, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
        else
            lv_obj_align(btn, NULL, LV_ALIGN_IN_TOP_MID, -20, 2);
        tail = btn;

        lv_obj_t *del = lv_btn_create(data->page, NULL);
        lv_obj_set_size(del, 42, 42);
        lv_label_set_text(lv_label_create(del, NULL), LV_SYMBOL_CLOSE);
        lv_obj_align(del, btn, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
        view_register_default_callback_number(del, DEL_MASTER_BTN_ID, i);
    }

    if (model_get_masters_in_channel(model, data->channel) < MAX_MASTERS_PER_CHANNEL) {
        lv_obj_t *btn = lv_btn_create(data->page, NULL);
        lv_btn_set_layout(btn, LV_LAYOUT_OFF);
        lv_obj_set_width(btn, 220);

        lv_obj_t *lbl = lv_label_create(btn, NULL);
        lv_label_set_text(lbl, LV_SYMBOL_PLUS);
        lv_obj_align(lbl, NULL, LV_ALIGN_IN_LEFT_MID, 20, 0);

        lbl = lv_label_create(btn, NULL);
        lv_label_set_text(lbl, "Aggiungi master");
        lv_obj_align(lbl, NULL, LV_ALIGN_IN_RIGHT_MID, -20, 0);

        if (tail)
            lv_obj_align(btn, tail, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
        else
            lv_obj_align(btn, NULL, LV_ALIGN_IN_TOP_MID, -20, 2);
        tail = btn;

        if (data->focus == FOCUS_MASTER)
            lv_page_focus(data->page, btn, LV_ANIM_OFF);


        view_register_default_callback(btn, ADD_MASTER_BTN_ID);
    }

    for (size_t i = 0; i < model_get_minions_in_channel(model, data->channel); i++) {
        lv_obj_t *btn = lv_btn_create(data->page, NULL);
        lv_obj_set_width(btn, 200);
        lv_obj_t *lbl = lv_label_create(btn, NULL);
        lv_label_set_text_fmt(lbl, "Minion %i", i);

        if (tail)
            lv_obj_align(btn, tail, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
        else
            lv_obj_align(btn, NULL, LV_ALIGN_IN_TOP_MID, -20, 2);
        tail = btn;

        view_register_default_callback_number(btn, MINION_BTN_ID, i);

        lv_obj_t *del = lv_btn_create(data->page, NULL);
        lv_obj_set_size(del, 42, 42);
        lv_label_set_text(lv_label_create(del, NULL), LV_SYMBOL_CLOSE);
        lv_obj_align(del, btn, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
        view_register_default_callback_number(del, DEL_MINION_BTN_ID, i);
    }

    if (model_get_minions_in_channel(model, data->channel) < MAX_MINIONS_PER_CHANNEL) {
        lv_obj_t *btn = lv_btn_create(data->page, NULL);
        lv_btn_set_layout(btn, LV_LAYOUT_OFF);
        lv_obj_set_width(btn, 220);

        lv_obj_t *lbl = lv_label_create(btn, NULL);
        lv_label_set_text(lbl, LV_SYMBOL_PLUS);
        lv_obj_align(lbl, NULL, LV_ALIGN_IN_LEFT_MID, 20, 0);

        lbl = lv_label_create(btn, NULL);
        lv_label_set_text(lbl, "Aggiungi minion");
        lv_obj_align(lbl, NULL, LV_ALIGN_IN_RIGHT_MID, -20, 0);

        if (tail)
            lv_obj_align(btn, tail, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
        else
            lv_obj_align(btn, NULL, LV_ALIGN_IN_TOP_MID, -20, 2);
        tail = btn;

        if (data->focus == FOCUS_MINION)
            lv_page_focus(data->page, btn, LV_ANIM_OFF);

        view_register_default_callback(btn, ADD_MINION_BTN_ID);
    }
}

static void *create_page(model_t *model, void *extra) {
    struct page_data *data = malloc(sizeof(struct page_data));
    data->channel          = (size_t)(uintptr_t)extra;
    data->focus            = FOCUS_NONE;
    return data;
}


static void open_page(model_t *model, void *args) {
    struct page_data *data = args;
    data->focus            = FOCUS_NONE;

    lv_obj_t *back = view_common_back_button(BACK_BTN_ID);

    lv_obj_t *title = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_text_font(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_title());
    lv_label_set_text_fmt(title, "Canale %i", data->channel + 1);
    lv_obj_align(title, back, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

    lv_obj_t *page = lv_page_create(lv_scr_act(), NULL);
    lv_page_set_scrl_layout(page, LV_LAYOUT_OFF);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES - 50);
    lv_obj_align(page, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    data->page = page;
}


static view_message_t process_page_event(model_t *model, void *arg, view_event_t event) {
    struct page_data *data = arg;
    (void)data;
    view_message_t msg = {0};

    switch (event.code) {
        case VIEW_EVENT_CODE_OPEN:
            break;

        case VIEW_EVENT_CODE_CUSTOM:
            break;

        case VIEW_EVENT_CODE_LVGL:
            if (event.lvgl.lv_event == LV_EVENT_CLICKED) {
                switch (event.lvgl.data->id) {
                    case MASTER_BTN_ID: {
                        msg.vmsg.code  = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE_EXTRA;
                        msg.vmsg.page  = &page_master_settings;
                        size_t *args   = malloc(sizeof(size_t) * 2);
                        args[0]        = data->channel;
                        args[1]        = event.lvgl.data->number;
                        msg.vmsg.extra = args;
                        break;
                    }

                    case MINION_BTN_ID: {
                        msg.vmsg.code  = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE_EXTRA;
                        msg.vmsg.page  = &page_minion_settings;
                        size_t *args   = malloc(sizeof(size_t) * 2);
                        args[0]        = data->channel;
                        args[1]        = event.lvgl.data->number;
                        msg.vmsg.extra = args;
                        break;
                    }

                    case ADD_MASTER_BTN_ID:
                        model_add_master_to_channel(model, data->channel);
                        data->focus   = FOCUS_MASTER;
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_UPDATE;
                        break;

                    case ADD_MINION_BTN_ID:
                        model_add_minion_to_channel(model, data->channel);
                        data->focus   = FOCUS_MINION;
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_UPDATE;
                        break;

                    case BACK_BTN_ID:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        break;
                }
            } else if (event.lvgl.lv_event == LV_EVENT_LONG_PRESSED) {
                switch (event.lvgl.data->id) {
                    case DEL_MASTER_BTN_ID:
                        model_remove_master_from_channel(model, data->channel, event.lvgl.data->number);
                        data->focus   = FOCUS_MASTER;
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_UPDATE;
                        break;

                    case DEL_MINION_BTN_ID:
                        model_remove_minion_from_channel(model, data->channel, event.lvgl.data->number);
                        data->focus   = FOCUS_MINION;
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_UPDATE;
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


static void update_page(model_t *model, void *arg) {
    struct page_data *data = arg;
    update_device_list(data, model);
}


static void destroy_page(void *args, void *extra) {
    free(args);
}


const pman_page_t page_channel_settings = {
    .create        = create_page,
    .open          = open_page,
    .update        = update_page,
    .close         = view_close_all,
    .destroy       = destroy_page,
    .process_event = process_page_event,
};
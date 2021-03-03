#include <stdlib.h>

#include "lv_misc/lv_area.h"
#include "lv_widgets/lv_label.h"
#include "lv_widgets/lv_roller.h"
#include "lvgl.h"
#include "gel/pagemanager/page_manager.h"

#include "view/common.h"
#include "view/view.h"
#include "model/model.h"


enum {
    BACK_BTN_ID,
    IP_BTN_ID,
    PW_ROLLER_ID,
};


struct page_data {
    lv_obj_t *ip1, *ip2, *ip3, *ip4;
    lv_obj_t *pw1, *pw2, *pw3, *pw4;
};


static void update_ip(struct page_data *data, model_t *model) {
    int ip1, ip2, ip3, ip4;
    model_get_my_ip_parts(model, &ip1, &ip2, &ip3, &ip4);
    lv_roller_set_selected(data->ip1, ip1, LV_ANIM_OFF);
    lv_roller_set_selected(data->ip2, ip2, LV_ANIM_OFF);
    lv_roller_set_selected(data->ip3, ip3, LV_ANIM_OFF);
    lv_roller_set_selected(data->ip4, ip4, LV_ANIM_OFF);
}


static void set_ip(model_t *model, struct page_data *data) {
    int ip1 = lv_roller_get_selected(data->ip1);
    int ip2 = lv_roller_get_selected(data->ip2);
    int ip3 = lv_roller_get_selected(data->ip3);
    int ip4 = lv_roller_get_selected(data->ip4);
    model_set_my_ip_parts(model, ip1, ip2, ip3, ip4);
}


static void *create_page(model_t *model, void *extra) {
    struct page_data *data = malloc(sizeof(struct page_data));
    return data;
}


static void open_page(model_t *model, void *args) {
    struct page_data *data = args;

    lv_obj_t *back = view_common_back_button(BACK_BTN_ID);

    lv_obj_t *title = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_text_font(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_title());
    lv_label_set_text(title, "Impostazioni");
    lv_obj_align(title, back, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

    lv_obj_t *page = lv_page_create(lv_scr_act(), NULL);
    lv_page_set_scrl_layout(page, LV_LAYOUT_COLUMN_MID);
    lv_page_set_scrollable_fit2(page, LV_FIT_NONE, LV_FIT_TIGHT);
    lv_page_set_scrl_width(page, LV_HOR_RES);

    lv_obj_set_style_local_pad_all(page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_top(page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 8);
    lv_obj_set_style_local_margin_all(page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_all(page, LV_PAGE_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_margin_all(page, LV_PAGE_PART_BG, LV_STATE_DEFAULT, 0);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES - 50);
    lv_obj_align(page, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    lv_obj_t *cont = view_common_ip_widget(page, &data->ip1, &data->ip2, &data->ip3, &data->ip4);
    lv_page_glue_obj(cont, 1);
    lv_page_glue_obj(data->ip1, 1);
    lv_page_glue_obj(data->ip2, 1);
    lv_page_glue_obj(data->ip3, 1);
    lv_page_glue_obj(data->ip4, 1);

    cont = lv_cont_create(page, NULL);
    lv_cont_set_fit(cont, LV_FIT_TIGHT);
    lv_cont_set_layout(cont, LV_LAYOUT_OFF);
    lv_page_glue_obj(cont, 1);

    lv_obj_t *lbl = lv_label_create(cont, NULL);
    lv_obj_set_style_local_text_font(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_subtitle());
    lv_label_set_text(lbl, "Password");
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

    lv_obj_t *pw1 = lv_roller_create(cont, NULL);
    view_common_roller_range(pw1, 0, 9);
    lv_obj_set_style_local_pad_hor(pw1, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, 12);
    lv_obj_set_style_local_pad_hor(pw1, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, 12);

    lv_roller_set_visible_row_count(pw1, 2);

    lv_obj_t *pw2 = lv_roller_create(cont, pw1);
    lv_obj_t *pw3 = lv_roller_create(cont, pw1);
    lv_obj_t *pw4 = lv_roller_create(cont, pw1);

    lv_obj_align(pw1, lbl, LV_ALIGN_OUT_BOTTOM_MID, -(lv_obj_get_width(pw1) * 2) - 16, 4);
    lv_obj_align(pw2, lbl, LV_ALIGN_OUT_BOTTOM_MID, -lv_obj_get_width(pw1) - 4, 4);
    lv_obj_align(pw3, lbl, LV_ALIGN_OUT_BOTTOM_MID, +4, 4);
    lv_obj_align(pw4, lbl, LV_ALIGN_OUT_BOTTOM_MID, lv_obj_get_width(pw1) + 16, 4);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

    lv_roller_set_selected(pw1, model->password[0], LV_ANIM_OFF);
    lv_roller_set_selected(pw2, model->password[1], LV_ANIM_OFF);
    lv_roller_set_selected(pw3, model->password[2], LV_ANIM_OFF);
    lv_roller_set_selected(pw4, model->password[3], LV_ANIM_OFF);

    data->pw1 = pw1;
    data->pw2 = pw2;
    data->pw3 = pw3;
    data->pw4 = pw4;

    view_register_default_callback(data->pw1, PW_ROLLER_ID);
    view_register_default_callback(data->pw2, PW_ROLLER_ID);
    view_register_default_callback(data->pw3, PW_ROLLER_ID);
    view_register_default_callback(data->pw4, PW_ROLLER_ID);

    view_register_default_callback(data->ip1, IP_BTN_ID);
    view_register_default_callback(data->ip2, IP_BTN_ID);
    view_register_default_callback(data->ip3, IP_BTN_ID);
    view_register_default_callback(data->ip4, IP_BTN_ID);
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
                }
            } else if (event.lvgl.lv_event == LV_EVENT_VALUE_CHANGED) {
                switch (event.lvgl.data->id) {
                    case IP_BTN_ID:
                        set_ip(model, data);
                        break;

                    case PW_ROLLER_ID:
                        model_set_password(model, lv_roller_get_selected(data->pw1), lv_roller_get_selected(data->pw2),
                                           lv_roller_get_selected(data->pw3), lv_roller_get_selected(data->pw4));
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
    update_ip(data, model);
}


const pman_page_t page_settings = {
    .create        = create_page,
    .open          = open_page,
    .update        = update_page,
    .close         = view_close_all,
    .destroy       = view_destroy_all,
    .process_event = process_page_event,
};
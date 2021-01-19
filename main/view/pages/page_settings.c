#include <stdlib.h>

#include "lv_widgets/lv_dropdown.h"
#include "lvgl.h"
#include "gel/pagemanager/page_manager.h"

#include "view/common.h"
#include "view/view.h"
#include "model/model.h"


enum {
    BACK_BTN_ID,
    IP_BTN_ID,
};


struct page_data {
    lv_obj_t *ip1, *ip2, *ip3, *ip4;
};


static void update_ip(struct page_data *data, model_t *model) {
    int ip1, ip2, ip3, ip4;
    model_get_my_ip_parts(model, &ip1, &ip2, &ip3, &ip4);
    lv_dropdown_set_selected(data->ip1, ip1);
    lv_dropdown_set_selected(data->ip2, ip2);
    lv_dropdown_set_selected(data->ip3, ip3);
    lv_dropdown_set_selected(data->ip4, ip4);
}


static void set_ip(model_t *model, struct page_data *data) {
    int ip1 = lv_dropdown_get_selected(data->ip1);
    int ip2 = lv_dropdown_get_selected(data->ip2);
    int ip3 = lv_dropdown_get_selected(data->ip3);
    int ip4 = lv_dropdown_get_selected(data->ip4);
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

    lv_obj_t *cont = view_common_ip_widget(lv_scr_act(), &data->ip1, &data->ip2, &data->ip3, &data->ip4);
    lv_obj_align(cont, NULL, LV_ALIGN_IN_TOP_MID, 0, 60);

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
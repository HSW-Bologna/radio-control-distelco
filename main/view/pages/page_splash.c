#include <stdlib.h>

#include "lvgl.h"
#include "gel/pagemanager/page_manager.h"

#include "view/view.h"
#include "model/model.h"

struct page_data {};

static void *create_page(model_t *model, void *extra) {
    struct page_data *data = malloc(sizeof(struct page_data));
    return data;
}


static void open_page(model_t *model, void *args) {
    lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_COLOR_BLUE);
    lv_obj_set_style_local_bg_opa(btn, LV_BTN_PART_MAIN, LV_STATE_PRESSED, LV_OPA_50);
    lv_obj_t *lbl = lv_label_create(btn, NULL);
    lv_label_set_text(lbl, "Ciao");
    lv_obj_align(btn, NULL, LV_ALIGN_CENTER, 0, 0);
}

static view_message_t process_page_event(model_t *model, void *arg, view_event_t event) {
    struct page_data *data = arg;
    (void)data;
    view_message_t msg = {0};

    switch (event.code) {
        case VIEW_EVENT_CODE_CUSTOM:
            break;

        case VIEW_EVENT_CODE_LVGL:
            break;

        default:
            break;
    }

    return msg;
}


const pman_page_t page_splash = {
    .create        = create_page,
    .open          = open_page,
    .close         = view_close_all,
    .destroy       = view_destroy_all,
    .process_event = process_page_event,
};
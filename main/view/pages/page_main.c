#include <assert.h>
#include <stdlib.h>

#include "lvgl.h"
#include "gel/pagemanager/page_manager.h"

#include "view/view.h"
#include "view/common.h"
#include "view/theme/style.h"
#include "model/model.h"

LV_IMG_DECLARE(img_logo_hsw);
LV_IMG_DECLARE(img_logo_distelco);

enum {
    CONFIG_BTN_ID,
    CHANNEL_LED_ID,
    PASSWORD_POPUP_ID,
    OTHER_SHEET_ID,
};

struct page_data {
    lv_obj_t *channelcont;
    lv_obj_t *cablecont;
    lv_obj_t *leds[MAX_CHANNELS];
    lv_obj_t *names[MAX_CHANNELS];
    lv_obj_t *cable_leds[MAX_CHANNELS];

    lv_obj_t *lrete;
    lv_obj_t *lbat;
    lv_obj_t *lguasto;

    lv_obj_t *popup;

    int guasto_rete_alimentazione;
    int batteria_bassa;
};


static void create_canali(struct page_data *data, model_t *model) {
    lv_obj_clean(data->channelcont);
    int count = 0;

    for (size_t i = 0; i < MAX_CHANNELS; i++) {
        if (model_is_channel_configured(model, i)) {
            lv_obj_t *cont = lv_cont_create(data->channelcont, NULL);
            // lv_cont_set_layout(cont, LV_LAYOUT_OFF);
            lv_obj_add_style(cont, LV_CONT_PART_MAIN, &style_transparent_cont);
            lv_obj_set_size(cont, 60, 60);

            lv_obj_t *led = lv_led_create(cont, NULL);
            lv_obj_set_size(led, 32, 32);
            lv_obj_align(led, NULL, LV_ALIGN_IN_TOP_MID, 0, 6);
            lv_led_on(led);
            view_register_default_callback_number(led, CHANNEL_LED_ID, i);
            data->leds[i] = led;

            lv_obj_t *name = lv_label_create(cont, NULL);
            lv_label_set_long_mode(name, LV_LABEL_LONG_SROLL_CIRC);
            lv_obj_set_width(name, 54);
            lv_obj_set_auto_realign(name, 1);
            lv_obj_align(name, led, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);
            data->names[i] = name;

            count++;
        } else {
            data->leds[i]  = NULL;
            data->names[i] = NULL;
        }
    }

    lv_obj_set_size(data->channelcont, 70 * count, 70);
}


static void create_cavi(struct page_data *data, model_t *model) {
    lv_obj_clean(data->cablecont);
    lv_obj_set_style_local_margin_all(data->cablecont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    int count = 0;

    for (size_t i = 0; i < MAX_CABLES; i++) {
        if (model_is_cable_enabled(model, i)) {
            lv_obj_t *cont = lv_cont_create(data->cablecont, NULL);
            lv_obj_add_style(cont, LV_CONT_PART_MAIN, &style_transparent_cont);
            lv_obj_set_size(cont, 60, 60);

            lv_obj_t *led = lv_led_create(cont, NULL);
            lv_obj_set_size(led, 32, 32);
            lv_obj_align(led, NULL, LV_ALIGN_IN_TOP_MID, 0, 6);
            lv_led_on(led);
            data->cable_leds[i] = led;

            lv_obj_t *name = lv_label_create(cont, NULL);
            lv_label_set_text_fmt(name, "CR%i", i + 1);
            lv_obj_align(name, led, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);
            count++;
        } else {
            data->cable_leds[i] = NULL;
        }
    }

    lv_obj_set_size(data->cablecont, 70 * count, 80);
}


static void update_canali(struct page_data *data, model_t *model) {
    for (size_t i = 0; i < MAX_CHANNELS; i++) {
        if (model_is_channel_configured(model, i)) {
            assert(data->leds[i] && data->names[i]);
            lv_obj_set_style_local_bg_color(data->leds[i], LV_LED_PART_MAIN, LV_STATE_DEFAULT,
                                            model_is_channel_ok(model, i) ? LV_COLOR_GREEN : LV_COLOR_RED);
            lv_obj_set_style_local_shadow_color(data->leds[i], LV_LED_PART_MAIN, LV_STATE_DEFAULT,
                                                model_is_channel_ok(model, i) ? LV_COLOR_GREEN : LV_COLOR_RED);

            if (strcmp(lv_label_get_text(data->names[i]), model_get_channel_name(model, i)))
                lv_label_set_text(data->names[i], model_get_channel_name(model, i));
        }
    }
}


static void update_cavi(struct page_data *data, model_t *model) {
    if (model_get_errore_scheda_gestione(model)) {
        view_common_set_hidden(data->cablecont, 1);
        view_common_set_hidden(data->lbat, 1);
        view_common_set_hidden(data->lrete, 1);
        view_common_set_hidden(data->lguasto, 0);
    } else {
        view_common_set_hidden(data->cablecont, 0);
        view_common_set_hidden(data->lbat, 0);
        view_common_set_hidden(data->lrete, 0);
        view_common_set_hidden(data->lguasto, 1);
    }

    for (size_t i = 0; i < MAX_CABLES; i++) {
        if (model_is_cable_enabled(model, i)) {
            assert(data->cable_leds[i]);
            if (model_is_cable_ok(model, i)) {
                lv_obj_set_style_local_bg_color(data->cable_leds[i], LV_LED_PART_MAIN, LV_STATE_DEFAULT,
                                                LV_COLOR_GREEN);
                lv_obj_set_style_local_shadow_color(data->cable_leds[i], LV_LED_PART_MAIN, LV_STATE_DEFAULT,
                                                    LV_COLOR_GREEN);
            } else {
                lv_obj_set_style_local_bg_color(data->cable_leds[i], LV_LED_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
                lv_obj_set_style_local_shadow_color(data->cable_leds[i], LV_LED_PART_MAIN, LV_STATE_DEFAULT,
                                                    LV_COLOR_RED);
            }
        }
    }
}


static void *create_page(model_t *model, void *extra) {
    struct page_data *data = malloc(sizeof(struct page_data));
    memset(data, 0, sizeof(struct page_data));
    return data;
}


static void open_page(model_t *model, void *args) {
    struct page_data *data = args;

    lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(cont, LV_HOR_RES, 40);
    lv_obj_align(cont, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

    lv_obj_t *img = lv_img_create(cont, NULL);
    lv_img_set_src(img, &img_logo_distelco);
    lv_obj_align(img, NULL, LV_ALIGN_IN_LEFT_MID, 8, 0);

    lv_obj_t *img2 = lv_img_create(cont, NULL);
    lv_img_set_src(img2, &img_logo_hsw);
    lv_obj_align(img2, img, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

    lv_obj_t *settings = lv_btn_create(cont, NULL);
    lv_obj_set_size(settings, 32, 32);
    lv_obj_t *lbl = lv_label_create(settings, NULL);
    lv_label_set_text(lbl, LV_SYMBOL_SETTINGS);
    lv_obj_align(settings, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);

    cont = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES - 40);
    lv_obj_align(cont, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    lv_obj_t *canali = lv_cont_create(cont, NULL);
    lv_obj_add_style(canali, LV_CONT_PART_MAIN, &style_transparent_cont);
    lv_cont_set_layout(canali, LV_LAYOUT_OFF);
    lv_obj_set_size(canali, LV_HOR_RES, 100);
    lv_obj_align(canali, NULL, LV_ALIGN_IN_TOP_MID, 0, 4);

    lbl = lv_label_create(canali, NULL);
    lv_label_set_text(lbl, "Canali");
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_MID, 0, 8);
    lv_obj_set_style_local_margin_all(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);

    lv_obj_t *leds = lv_cont_create(canali, NULL);
    lv_cont_set_layout(leds, LV_LAYOUT_PRETTY_MID);
    lv_obj_add_style(leds, LV_CONT_PART_MAIN, &style_transparent_cont);
    lv_obj_set_auto_realign(leds, 1);
    lv_obj_align(leds, lbl, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    data->channelcont = leds;
    create_canali(data, model);

    lv_obj_align(canali, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

    lv_obj_t *cavi = lv_cont_create(cont, NULL);
    lv_obj_add_style(cavi, LV_CONT_PART_MAIN, &style_transparent_cont);
    lv_cont_set_layout(cavi, LV_LAYOUT_OFF);
    lv_obj_set_size(cavi, LV_HOR_RES, 100);
    lv_obj_align(cavi, NULL, LV_ALIGN_IN_TOP_MID, 0, 4);

    lbl = lv_label_create(cavi, NULL);
    lv_label_set_text(lbl, "Cavi radianti");
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

    lv_obj_t *cables = lv_cont_create(cavi, NULL);
    lv_cont_set_layout(cables, LV_LAYOUT_PRETTY_MID);
    lv_obj_add_style(cables, LV_CONT_PART_MAIN, &style_transparent_cont);
    lv_obj_set_auto_realign(cables, 1);
    lv_obj_align(cables, lbl, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    data->cablecont = cables;
    create_cavi(data, model);
    lv_obj_align(cavi, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    lbl = lv_label_create(cavi, NULL);
    lv_obj_set_style_local_text_font(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_subtitle());
    lv_label_set_text(lbl, "GUASTO");
    lv_obj_align(lbl, NULL, LV_ALIGN_CENTER, 0, 0);
    data->lguasto = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 4, 0);
    data->lbat = lbl;

    lbl = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(lbl, 1);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -4, 0);
    data->lrete = lbl;

    lv_obj_t *sheet = lv_cont_create(lv_scr_act(), NULL);
    lv_obj_set_size(sheet, LV_HOR_RES, LV_VER_RES);
    lv_obj_add_style(sheet, LV_CONT_PART_MAIN, &style_transparent_cont);
    lv_obj_align(sheet, NULL, LV_ALIGN_CENTER, 0, 0);
    view_common_password_popup(model, sheet, PASSWORD_POPUP_ID);

    data->popup = sheet;
    lv_obj_set_hidden(data->popup, 1);

    view_register_default_callback(data->popup, OTHER_SHEET_ID);
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

        case VIEW_EVENT_CODE_POWER_STATE:
            data->batteria_bassa            = (event.power_bits & 0x02) > 0;
            data->guasto_rete_alimentazione = (event.power_bits & 0x01) > 0;
            msg.vmsg.code                   = VIEW_PAGE_COMMAND_CODE_UPDATE;
            break;

        case VIEW_EVENT_CODE_LVGL:
            if (event.lvgl.lv_event == LV_EVENT_CLICKED) {
                switch (event.lvgl.data->id) {
                    case OTHER_SHEET_ID:
                        lv_obj_set_hidden(data->popup, 1);
                        break;

                    case CONFIG_BTN_ID:
                        if (model_password_enabled(model)) {
                            lv_obj_set_hidden(data->popup, 0);
                        } else {
                            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                            msg.vmsg.page = &page_menu;
                        }
                        break;

                    case CHANNEL_LED_ID:
                        msg.vmsg.code  = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE_EXTRA;
                        msg.vmsg.page  = &page_channel_state;
                        msg.vmsg.extra = (void *)(uintptr_t)event.lvgl.data->number;
                        break;

                    default:
                        break;
                }
            } else if (event.lvgl.lv_event == LV_EVENT_APPLY) {
                msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE;
                msg.vmsg.page = &page_menu;
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
    update_cavi(data, model);

    if (data->guasto_rete_alimentazione)
        lv_label_set_text(data->lrete, "mancanza rete");
    else
        lv_label_set_text(data->lrete, "rete ok");

    if (data->batteria_bassa)
        lv_label_set_text(data->lbat, "batteria bassa");
    else
        lv_label_set_text(data->lbat, "batteria ok");
}


const pman_page_t page_main = {
    .create        = create_page,
    .open          = open_page,
    .update        = update_page,
    .close         = view_close_all,
    .destroy       = view_destroy_all,
    .process_event = process_page_event,
};
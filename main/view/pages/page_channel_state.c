#include <stdlib.h>

#include "lv_widgets/lv_page.h"
#include "lvgl.h"
#include "gel/pagemanager/page_manager.h"

#include "view/common.h"
#include "view/view.h"
#include "model/model.h"


enum {
    BACK_BTN_ID,
    REENABLE_MASTER_BTN_ID,
    REENABLE_MINION_BTN_ID,
};


struct page_data {
    size_t    channel;
    lv_obj_t *title;

    struct {
        lv_obj_t *cont;
        lv_obj_t *name;
        lv_obj_t *status;
        lv_obj_t *radio;
        lv_obj_t *antenna;
        lv_obj_t *failed_tx;
        lv_obj_t *tx_disabled;
        lv_obj_t *reenable;
        lv_obj_t *remotes[MAX_MINIONS_PER_CHANNEL];
    } masters[MAX_MASTERS_PER_CHANNEL];

    struct {
        lv_obj_t *name;
        lv_obj_t *status;
        lv_obj_t *radio;
        lv_obj_t *failed_tx;
        lv_obj_t *tx_disabled;
        lv_obj_t *reenable;
        lv_obj_t *remotes[MAX_MINIONS_PER_CHANNEL];
    } minions[MAX_MASTERS_PER_CHANNEL];
};


static void led_color(lv_obj_t *led, lv_color_t color) {
    lv_obj_set_style_local_bg_color(led, LV_LED_PART_MAIN, LV_STATE_DEFAULT, color);
    lv_obj_set_style_local_shadow_color(led, LV_LED_PART_MAIN, LV_STATE_DEFAULT, color);
}


static lv_obj_t *create_master_panel(lv_obj_t *root, model_t *model, struct page_data *data, size_t index) {
    lv_obj_t *cont = lv_cont_create(root, NULL);
    lv_obj_set_style_local_pad_hor(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 8);
    lv_obj_set_style_local_pad_ver(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 4);
    lv_cont_set_layout(cont, LV_LAYOUT_OFF);
    lv_cont_set_fit2(cont, LV_FIT_PARENT, LV_FIT_NONE);

    data->masters[index].cont = cont;

    lv_obj_t *name = lv_label_create(cont, NULL);
    lv_obj_set_style_local_text_font(name, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_subtitle());
    lv_label_set_long_mode(name, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_width(name, lv_obj_get_width(cont) - 32);
    lv_obj_align(name, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 4);
    data->masters[index].name = name;

    lv_obj_t *desc = lv_label_create(cont, NULL);
    uint32_t  ip;
    model_get_channel_master_ip(model, &ip, data->channel, index);
    lv_label_set_text_fmt(desc, "Master - %i.%i.%i.%i", IP_GET_PART_1(ip), IP_GET_PART_2(ip), IP_GET_PART_3(ip),
                          IP_GET_PART_4(ip));
    lv_obj_align(desc, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 32);

    lv_obj_t *status = lv_led_create(cont, NULL);
    lv_obj_set_size(status, 16, 16);
    lv_obj_align(status, NULL, LV_ALIGN_IN_TOP_RIGHT, -12, 12);
    lv_led_on(status);
    data->masters[index].status = status;

    lv_obj_t *lbl = lv_label_create(cont, NULL);
    lv_label_set_text(lbl, "Radio: ");
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 8, 56);
    lv_obj_t *radio = lv_led_create(cont, status);
    lv_obj_align(radio, NULL, LV_ALIGN_IN_TOP_LEFT, 160, 56);
    lv_led_on(radio);
    data->masters[index].radio = radio;

    lbl = lv_label_create(cont, NULL);
    lv_label_set_text(lbl, "Antenna: ");
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 8, 80);
    lv_obj_t *antenna = lv_led_create(cont, status);
    lv_obj_align(antenna, NULL, LV_ALIGN_IN_TOP_LEFT, 160, 80);
    lv_led_on(antenna);
    data->masters[index].antenna = antenna;

    lbl = lv_label_create(cont, NULL);
    lv_label_set_text(lbl, "Tx fail: ");
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 8, 104);
    lv_obj_t *failed_tx = lv_led_create(cont, status);
    lv_obj_align(failed_tx, NULL, LV_ALIGN_IN_TOP_LEFT, 160, 104);
    data->masters[index].failed_tx = failed_tx;

    lbl = lv_label_create(cont, NULL);
    lv_label_set_text(lbl, "Tx disabled:");
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 8, 128);
    lv_obj_t *tx_disabled = lv_led_create(cont, status);
    lv_obj_align(tx_disabled, NULL, LV_ALIGN_IN_TOP_LEFT, 160, 128);
    data->masters[index].tx_disabled = tx_disabled;

    lv_obj_t *reenable = lv_btn_create(cont, NULL);
    lv_obj_set_width(reenable, 80);
    lbl = lv_label_create(reenable, NULL);
    lv_label_set_text(lbl, "enable");
    view_register_default_callback_number(reenable, REENABLE_MASTER_BTN_ID, index);
    lv_obj_align(reenable, tx_disabled, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    data->masters[index].reenable = reenable;

    device_info_t info = model_get_channel_master_info(model, data->channel, index);

    lv_obj_t *fw = lv_label_create(cont, NULL);
    lv_label_set_text(fw, info.fw_version);
    lv_obj_set_auto_realign(fw, 1);
    lv_obj_align(fw, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -4, -4);

    int num_minions = 0;
    for (size_t i = 0; i < MAX_MINIONS_PER_CHANNEL; i++) {
        data->masters[index].remotes[i] = NULL;
        if (info.remotes[i] != RemoteNotPresent)
            num_minions++;
    }

    if (num_minions > 0) {
        lv_obj_t *minions = lv_label_create(cont, NULL);
        lv_obj_set_style_local_text_font(minions, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_subtitle());
        lv_label_set_text(minions, "Remoti:");
        lv_obj_align(minions, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 154);

        int counter = 0;
        for (size_t i = 0; i < MAX_MINIONS_PER_CHANNEL; i++) {
            if (info.remotes[i] == RemoteNotPresent)
                continue;

            uint32_t ip = info.remotes[i];
            lbl         = lv_label_create(cont, NULL);
            lv_label_set_text_fmt(lbl, "%i.%i.%i.%i", IP_GET_PART_1(ip), IP_GET_PART_2(ip), IP_GET_PART_3(ip),
                                  IP_GET_PART_4(ip));
            lv_obj_align(lbl, minions, LV_ALIGN_OUT_BOTTOM_LEFT, 4, 4 + 24 * counter);

            lv_obj_t *remote = lv_led_create(cont, status);
            lv_obj_align(remote, minions, LV_ALIGN_OUT_BOTTOM_LEFT, 160 - 4, 4 + 24 * counter);
            lv_led_on(remote);

            data->masters[index].remotes[i] = remote;
            counter++;
        }
    }

    lv_cont_set_fit2(cont, LV_FIT_PARENT, LV_FIT_TIGHT);
    int height = lv_obj_get_height(cont);
    lv_cont_set_fit2(cont, LV_FIT_PARENT, LV_FIT_NONE);
    lv_obj_set_height(cont, height + 24);

    return cont;
}


static lv_obj_t *create_minion_panel(lv_obj_t *root, model_t *model, struct page_data *data, size_t index) {
    lv_obj_t *cont = lv_cont_create(root, NULL);
    lv_obj_set_style_local_pad_hor(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 8);
    lv_obj_set_style_local_pad_ver(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 4);
    lv_cont_set_layout(cont, LV_LAYOUT_OFF);
    lv_cont_set_fit2(cont, LV_FIT_PARENT, LV_FIT_NONE);

    lv_obj_t *name = lv_label_create(cont, NULL);
    lv_obj_set_style_local_text_font(name, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_subtitle());
    lv_label_set_long_mode(name, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_width(name, lv_obj_get_width(cont) - 32);
    lv_obj_align(name, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 4);
    data->minions[index].name = name;

    lv_obj_t *desc = lv_label_create(cont, NULL);
    uint32_t  ip;
    model_get_channel_minion_ip(model, &ip, data->channel, index);
    lv_label_set_text_fmt(desc, "Minion - %i.%i.%i.%i", IP_GET_PART_1(ip), IP_GET_PART_2(ip), IP_GET_PART_3(ip),
                          IP_GET_PART_4(ip));
    lv_obj_align(desc, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 32);

    lv_obj_t *status = lv_led_create(cont, NULL);
    lv_obj_set_size(status, 16, 16);
    lv_obj_align(status, NULL, LV_ALIGN_IN_TOP_RIGHT, -12, 12);
    lv_led_on(status);
    data->minions[index].status = status;

    lv_obj_t *lbl = lv_label_create(cont, NULL);
    lv_label_set_text(lbl, "Radio: ");
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 8, 56);
    lv_obj_t *radio = lv_led_create(cont, status);
    lv_obj_align(radio, NULL, LV_ALIGN_IN_TOP_LEFT, 160, 56);
    lv_led_on(radio);
    data->minions[index].radio = radio;

    lbl = lv_label_create(cont, NULL);
    lv_label_set_text(lbl, "Tx fail: ");
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 8, 80);
    lv_obj_t *failed_tx = lv_led_create(cont, status);
    lv_obj_align(failed_tx, NULL, LV_ALIGN_IN_TOP_LEFT, 160, 80);
    data->minions[index].failed_tx = failed_tx;

    lbl = lv_label_create(cont, NULL);
    lv_label_set_text(lbl, "Tx disabled:");
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_LEFT, 8, 104);
    lv_obj_t *tx_disabled = lv_led_create(cont, status);
    lv_obj_align(tx_disabled, NULL, LV_ALIGN_IN_TOP_LEFT, 160, 104);
    data->minions[index].tx_disabled = tx_disabled;

    lv_obj_t *reenable = lv_btn_create(cont, NULL);
    lv_obj_set_width(reenable, 80);
    lbl = lv_label_create(reenable, NULL);
    lv_label_set_text(lbl, "enable");
    view_register_default_callback_number(reenable, REENABLE_MINION_BTN_ID, index);
    lv_obj_align(reenable, tx_disabled, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    data->minions[index].reenable = reenable;

    device_info_t info = model_get_channel_minion_info(model, data->channel, index);

    int num_masters = 0;
    for (size_t i = 0; i < MAX_MINIONS_PER_CHANNEL; i++) {
        data->minions[index].remotes[i] = NULL;
        if (info.remotes[i] != RemoteNotPresent)
            num_masters++;
    }

    if (num_masters > 0) {
        lv_obj_t *minions = lv_label_create(cont, NULL);
        lv_obj_set_style_local_text_font(minions, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_subtitle());
        lv_label_set_text(minions, "Remoti:");
        lv_obj_align(minions, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 134);

        int counter = 0;
        for (size_t i = 0; i < MAX_MASTERS_PER_CHANNEL; i++) {
            if (info.remotes[i] == RemoteNotPresent)
                continue;

            uint32_t ip = info.remotes[i];
            lbl         = lv_label_create(cont, NULL);
            lv_label_set_text_fmt(lbl, "%i.%i.%i.%i", IP_GET_PART_1(ip), IP_GET_PART_2(ip), IP_GET_PART_3(ip),
                                  IP_GET_PART_4(ip));
            lv_obj_align(lbl, minions, LV_ALIGN_OUT_BOTTOM_LEFT, 4, 4 + 24 * counter);

            lv_obj_t *remote = lv_led_create(cont, status);
            lv_obj_align(remote, minions, LV_ALIGN_OUT_BOTTOM_LEFT, 160 - 4, 4 + 24 * counter);
            lv_led_on(remote);

            data->minions[index].remotes[i] = remote;
            counter++;
        }
    }

    lv_obj_t *fw = lv_label_create(cont, NULL);
    lv_label_set_text(fw, info.fw_version);
    lv_obj_set_auto_realign(fw, 1);
    lv_obj_align(fw, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -4, -4);

    lv_cont_set_fit2(cont, LV_FIT_PARENT, LV_FIT_TIGHT);
    int height = lv_obj_get_height(cont);
    lv_cont_set_fit2(cont, LV_FIT_PARENT, LV_FIT_NONE);
    lv_obj_set_height(cont, height + 24);

    return cont;
}


static void update_devices(model_t *model, struct page_data *data) {
    for (size_t i = 0; i < model_get_masters_in_channel(model, data->channel); i++) {
        device_info_t info = model_get_channel_master_info(model, data->channel, i);
        lv_label_set_text(data->masters[i].name, model_get_channel_master_name(model, data->channel, i));

        int connected = info.connected && model_get_connected(model);

        if (info.guasto_radio || !connected) {
            led_color(data->masters[i].radio, LV_COLOR_RED);
        } else {
            led_color(data->masters[i].radio, LV_COLOR_GREEN);
        }

        if (info.guasto_antenna || !connected) {
            led_color(data->masters[i].antenna, LV_COLOR_RED);
        } else {
            led_color(data->masters[i].antenna, LV_COLOR_GREEN);
        }

        if (info.failed_tx || !connected) {
            led_color(data->masters[i].failed_tx, LV_COLOR_RED);
        } else {
            led_color(data->masters[i].failed_tx, LV_COLOR_GREEN);
        }

        if (info.tx_disabled || !connected) {
            led_color(data->masters[i].tx_disabled, LV_COLOR_RED);
            lv_obj_set_hidden(data->masters[i].reenable, !connected);
        } else {
            led_color(data->masters[i].tx_disabled, LV_COLOR_GREEN);
            lv_obj_set_hidden(data->masters[i].reenable, 1);
        }

        if (!connected)
            led_color(data->masters[i].status, LV_COLOR_RED);
        else
            led_color(data->masters[i].status, LV_COLOR_GREEN);

        for (size_t j = 0; j < MAX_MINIONS_PER_CHANNEL; j++) {
            if (data->masters[i].remotes[j]) {
                if (info.remoteStatus[j] == RemoteConnected && connected) {
                    led_color(data->masters[i].remotes[j], LV_COLOR_GREEN);
                } else if (info.remoteStatus[j] == RemoteLostConnection) {
                    led_color(data->masters[i].remotes[j], LV_COLOR_RED);
                }
            }
        }
    }

    for (size_t i = 0; i < model_get_minions_in_channel(model, data->channel); i++) {
        device_info_t info = model_get_channel_minion_info(model, data->channel, i);
        lv_label_set_text(data->minions[i].name, model_get_channel_minion_name(model, data->channel, i));

        int connected = info.connected && model_get_connected(model);

        if (info.guasto_radio || !connected) {
            led_color(data->minions[i].radio, LV_COLOR_RED);
        } else {
            led_color(data->minions[i].radio, LV_COLOR_GREEN);
        }

        if (info.failed_tx || !connected) {
            led_color(data->minions[i].failed_tx, LV_COLOR_RED);
        } else {
            led_color(data->minions[i].failed_tx, LV_COLOR_GREEN);
        }

        if (info.tx_disabled || !connected) {
            led_color(data->minions[i].tx_disabled, LV_COLOR_RED);
            lv_obj_set_hidden(data->minions[i].reenable, !connected);
        } else {
            led_color(data->minions[i].tx_disabled, LV_COLOR_GREEN);
            lv_obj_set_hidden(data->minions[i].reenable, 1);
        }

        if (!connected)
            led_color(data->minions[i].status, LV_COLOR_RED);
        else
            led_color(data->minions[i].status, LV_COLOR_GREEN);

        for (size_t j = 0; j < MAX_MASTERS_PER_CHANNEL; j++) {
            if (data->minions[i].remotes[j]) {
                if (info.remoteStatus[j] == RemoteConnected && connected) {
                    led_color(data->minions[i].remotes[j], LV_COLOR_GREEN);
                } else if (info.remoteStatus[j] == RemoteLostConnection) {
                    led_color(data->minions[i].remotes[j], LV_COLOR_RED);
                }
            }
        }
    }
}


static void *create_page(model_t *model, void *extra) {
    struct page_data *data = malloc(sizeof(struct page_data));
    data->channel          = (size_t)(uintptr_t)extra;
    return data;
}


static void open_page(model_t *model, void *args) {
    struct page_data *data = args;

    lv_obj_t *back = view_common_back_button(BACK_BTN_ID);

    lv_obj_t *title = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_text_font(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_title());
    lv_label_set_long_mode(title, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_width(title, LV_HOR_RES - 32);
    lv_obj_align(title, back, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
    data->title = title;

    lv_obj_t *page = lv_page_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_pad_all(page, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, 8);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES - 50);
    lv_obj_align(page, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_page_set_scrl_layout(page, LV_LAYOUT_COLUMN_MID);

    for (size_t i = 0; i < model_get_masters_in_channel(model, data->channel); i++) {
        lv_obj_t *dev = create_master_panel(page, model, data, i);
        lv_page_glue_obj(dev, 1);
    }

    for (size_t i = 0; i < model_get_minions_in_channel(model, data->channel); i++) {
        lv_obj_t *dev = create_minion_panel(page, model, data, i);
        lv_page_glue_obj(dev, 1);
    }
}


static view_message_t process_page_event(model_t *model, void *arg, view_event_t event) {
    struct page_data *data = arg;
    (void)data;
    view_message_t msg = {0};

    switch (event.code) {
        case VIEW_EVENT_CODE_OPEN:
            break;

        case VIEW_EVENT_CODE_MODEL_UPDATE:
            msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_UPDATE;
            break;

        case VIEW_EVENT_CODE_CUSTOM:
            break;

        case VIEW_EVENT_CODE_LVGL:
            if (event.lvgl.lv_event == LV_EVENT_CLICKED) {
                switch (event.lvgl.data->id) {
                    case BACK_BTN_ID:
                        msg.vmsg.code = VIEW_PAGE_COMMAND_CODE_BACK;
                        break;

                    case REENABLE_MASTER_BTN_ID: {
                        uint32_t ip;

                        if (model_get_channel_master_ip(model, &ip, data->channel, event.lvgl.data->number))
                            break;

                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_REENABLE_TX;
                        msg.cmsg.addr = ip;
                        break;
                    }

                    case REENABLE_MINION_BTN_ID: {
                        uint32_t ip;

                        if (model_get_channel_minion_ip(model, &ip, data->channel, event.lvgl.data->number))
                            break;

                        msg.cmsg.code = VIEW_CONTROLLER_COMMAND_REENABLE_TX;
                        msg.cmsg.addr = ip;
                        break;
                    }
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

    update_devices(model, data);
    lv_label_set_text(data->title, model_get_channel_name(model, data->channel));
}


static void destroy_page(void *args, void *extra) {
    free(args);
}


const pman_page_t page_channel_state = {
    .create        = create_page,
    .open          = open_page,
    .update        = update_page,
    .close         = view_close_all,
    .destroy       = destroy_page,
    .process_event = process_page_event,
};
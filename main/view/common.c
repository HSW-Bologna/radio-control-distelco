#include <stdio.h>
#include <assert.h>
#include "lvgl.h"

#include "view_types.h"
#include "view.h"


static void password_event_cb(lv_obj_t *obj, lv_event_t event);


lv_obj_t *view_common_back_button(int id) {
    lv_obj_t *back = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_t *lbl  = lv_label_create(back, NULL);
    lv_label_set_text(lbl, LV_SYMBOL_LEFT);
    lv_obj_set_size(back, 42, 42);
    lv_obj_align(back, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 4);
    view_register_default_callback(back, id);

    return back;
}


void view_common_roller_range(lv_obj_t *roller, int start, int end) {
    assert(start <= end);
    size_t size   = (end + 1 - start) * 8;
    size_t count  = 0;
    char * string = malloc(size);
    memset(string, 0, size);

    for (int i = start; i <= end; i++) {
        if (i == end)
            count += sprintf(&string[count], "%i", i);
        else
            count += sprintf(&string[count], "%i\n", i);
    }

    lv_roller_set_options(roller, string, LV_ROLLER_MODE_NORMAL);
    free(string);
}


void view_common_dropdown_range(lv_obj_t *dd, int start, int end) {
    assert(start <= end);
    char string[32];

    lv_dropdown_clear_options(dd);
    for (int i = start; i <= end; i++) {
        snprintf(string, 32, "%i", i);
        lv_dropdown_add_option(dd, string, LV_DROPDOWN_POS_LAST);
    }
}


lv_obj_t *view_common_ip_widget(lv_obj_t *root, lv_obj_t **ip1, lv_obj_t **ip2, lv_obj_t **ip3, lv_obj_t **ip4) {
    lv_obj_t *cont = lv_cont_create(root, NULL);
    lv_cont_set_fit(cont, LV_FIT_TIGHT);
    // lv_obj_set_height(cont, 140);
    lv_cont_set_layout(cont, LV_LAYOUT_OFF);

    lv_obj_t *lbl = lv_label_create(cont, NULL);
    lv_obj_set_style_local_text_font(lbl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_subtitle());
    lv_label_set_text(lbl, "IP");
    lv_label_set_align(lbl, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

    *ip1 = lv_roller_create(cont, NULL);
    view_common_roller_range(*ip1, 0, 255);
    // lv_roller_set_fix_width(*ip1, 65);

    *ip2 = lv_roller_create(cont, *ip1);
    *ip3 = lv_roller_create(cont, *ip1);
    *ip4 = lv_roller_create(cont, *ip1);

    lv_obj_t *center = lv_label_create(cont, NULL);
    lv_label_set_text(center, ".");
    lv_obj_align(center, lbl, LV_ALIGN_OUT_BOTTOM_MID, 0, 36);

    lv_obj_t *left = lv_label_create(cont, NULL);
    lv_label_set_text(left, ".");
    lv_obj_align(left, center, LV_ALIGN_OUT_LEFT_MID, -68, 0);

    lv_obj_t *right = lv_label_create(cont, NULL);
    lv_label_set_text(right, ".");
    lv_obj_align(right, center, LV_ALIGN_OUT_RIGHT_MID, 68, 0);

    lv_obj_align(*ip1, left, LV_ALIGN_OUT_LEFT_MID, -4, lv_obj_get_height(*ip1) / 6);
    lv_obj_align(*ip2, center, LV_ALIGN_OUT_LEFT_MID, -4, lv_obj_get_height(*ip2) / 6);
    lv_obj_align(*ip3, center, LV_ALIGN_OUT_RIGHT_MID, 4, lv_obj_get_height(*ip3) / 6);
    lv_obj_align(*ip4, right, LV_ALIGN_OUT_RIGHT_MID, 4, lv_obj_get_height(*ip4) / 6);

    lv_roller_set_visible_row_count(*ip1, 2);
    lv_roller_set_visible_row_count(*ip2, 2);
    lv_roller_set_visible_row_count(*ip3, 2);
    lv_roller_set_visible_row_count(*ip4, 2);

    lv_signal_send(*ip1, LV_SIGNAL_FOCUS, NULL);
    lv_signal_send(*ip2, LV_SIGNAL_FOCUS, NULL);
    lv_signal_send(*ip3, LV_SIGNAL_FOCUS, NULL);
    lv_signal_send(*ip4, LV_SIGNAL_FOCUS, NULL);
    lv_obj_align(lbl, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

    return cont;
}


lv_obj_t *view_common_password_popup(model_t *model, lv_obj_t *parent, int id) {
    lv_obj_t *cont = lv_cont_create(parent, NULL);
    lv_obj_set_size(cont, 240, 140);
    lv_obj_set_style_local_bg_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT,
                                    lv_color_lighten(LV_COLOR_GRAY, LV_OPA_60));
    lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 4);
    lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 64);
    lv_obj_set_style_local_border_color(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_color_primary());
    lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 0);


    for (size_t i = 0; i < 4; i++) {
        view_obj_data_t *data = malloc(sizeof(view_obj_data_t));
        data->id              = id;
        data->number          = model->password[i];
        data->extra           = cont;

        lv_obj_t *roller = lv_roller_create(cont, NULL);
        lv_obj_set_style_local_pad_hor(roller, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, 12);
        lv_obj_set_style_local_pad_hor(roller, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT, 12);
        lv_obj_set_style_local_text_font(roller, LV_ROLLER_PART_BG, LV_STATE_DEFAULT, lv_theme_get_font_subtitle());
        lv_obj_set_style_local_text_font(roller, LV_ROLLER_PART_SELECTED, LV_STATE_DEFAULT,
                                         lv_theme_get_font_subtitle());
        lv_roller_set_options(roller, "0\n1\n2\n3\n4\n5\n6\n7\n8\n9", LV_ROLLER_MODE_INIFINITE);
        lv_roller_set_visible_row_count(roller, 2);
        lv_obj_align(roller, NULL, LV_ALIGN_CENTER, -90 + lv_obj_get_width(roller) / 2 + 45 * i, 0);
        lv_obj_set_user_data(roller, data);
        lv_obj_set_event_cb(roller, password_event_cb);
    }

    return cont;
}


int view_common_set_hidden(lv_obj_t *obj, int hidden) {
    if (lv_obj_get_hidden(obj) != hidden) {
        lv_obj_set_hidden(obj, hidden);
        return 1;
    } else {
        return 0;
    }
}

#include "esp_log.h"

static void password_event_cb(lv_obj_t *obj, lv_event_t event) {
    view_obj_data_t *data = lv_obj_get_user_data(obj);

    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *       r1     = lv_obj_get_child(data->extra, NULL);
        int              digit2 = lv_roller_get_selected(r1);
        view_obj_data_t *data2  = lv_obj_get_user_data(r1);

        lv_obj_t *       r2     = lv_obj_get_child(data->extra, r1);
        int              digit1 = lv_roller_get_selected(r2);
        view_obj_data_t *data1  = lv_obj_get_user_data(r2);

        lv_obj_t *       r3     = lv_obj_get_child(data->extra, r2);
        int              digit3 = lv_roller_get_selected(r3);
        view_obj_data_t *data3  = lv_obj_get_user_data(r3);

        lv_obj_t *       r4     = lv_obj_get_child(data->extra, r3);
        int              digit4 = lv_roller_get_selected(r4);
        view_obj_data_t *data4  = lv_obj_get_user_data(r4);

        if (digit1 == data1->number && digit2 == data2->number && digit3 == data3->number && digit4 == data4->number) {
            view_event_t myevent = {.code = VIEW_EVENT_CODE_LVGL, .lvgl = {.data = data, .lv_event = LV_EVENT_APPLY}};
            view_event(myevent);
        }
    }
}

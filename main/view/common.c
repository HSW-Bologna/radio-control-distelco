#include <stdio.h>
#include <assert.h>
#include "lvgl.h"

#include "view.h"


lv_obj_t *view_common_back_button(int id) {
    lv_obj_t *back = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_t *lbl  = lv_label_create(back, NULL);
    lv_label_set_text(lbl, LV_SYMBOL_LEFT);
    lv_obj_set_size(back, 42, 42);
    lv_obj_align(back, NULL, LV_ALIGN_IN_TOP_LEFT, 4, 4);
    view_register_default_callback(back, id);

    return back;
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
    lv_cont_set_layout(cont, LV_LAYOUT_OFF);

    *ip1 = lv_dropdown_create(cont, NULL);
    view_common_dropdown_range(*ip1, 0, 255);
    lv_dropdown_set_draw_arrow(*ip1, 0);
    lv_obj_set_width(*ip1, 55);
    lv_obj_set_style_local_text_font(*ip1, LV_DROPDOWN_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_subtitle());
    lv_obj_set_style_local_text_font(*ip1, LV_DROPDOWN_PART_SELECTED, LV_STATE_DEFAULT, lv_theme_get_font_subtitle());
    lv_obj_set_style_local_text_font(*ip1, LV_DROPDOWN_PART_LIST, LV_STATE_DEFAULT, lv_theme_get_font_subtitle());

    *ip2 = lv_dropdown_create(cont, *ip1);
    *ip3 = lv_dropdown_create(cont, *ip1);
    *ip4 = lv_dropdown_create(cont, *ip1);

    lv_obj_t *center = lv_label_create(cont, NULL);
    lv_label_set_text(center, ".");
    lv_obj_align(center, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);

    lv_obj_t *left = lv_label_create(cont, NULL);
    lv_label_set_text(left, ".");
    lv_obj_align(left, center, LV_ALIGN_OUT_LEFT_MID, -60, 0);

    lv_obj_t *right = lv_label_create(cont, NULL);
    lv_label_set_text(right, ".");
    lv_obj_align(right, center, LV_ALIGN_OUT_RIGHT_MID, 60, 0);

    lv_obj_align(*ip1, left, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_obj_align(*ip2, center, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_obj_align(*ip3, center, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_align(*ip4, right, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    return cont;
}
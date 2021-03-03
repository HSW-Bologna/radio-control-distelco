#include "style.h"


lv_style_t style_transparent_cont;


void style_init(void) {
    lv_style_set_border_width(&style_transparent_cont, LV_STATE_DEFAULT, 0);
    lv_style_set_outline_width(&style_transparent_cont, LV_STATE_DEFAULT, 0);
    lv_style_set_margin_all(&style_transparent_cont, LV_STATE_DEFAULT, 0);
    lv_style_set_pad_all(&style_transparent_cont, LV_STATE_DEFAULT, 0);
    lv_style_set_bg_opa(&style_transparent_cont, LV_STATE_DEFAULT, LV_OPA_TRANSP);
}
#ifndef VIEW_COMMON_H_INCLUDED
#define VIEW_COMMON_H_INCLUDED

#include "lvgl.h"


lv_obj_t *view_common_back_button(int id);
void      view_common_dropdown_range(lv_obj_t *dd, int start, int end);
lv_obj_t *view_common_ip_widget(lv_obj_t *root, lv_obj_t **ip1, lv_obj_t **ip2, lv_obj_t **ip3, lv_obj_t **ip4);

#endif
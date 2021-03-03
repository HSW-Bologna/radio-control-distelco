#ifndef VIEW_COMMON_H_INCLUDED
#define VIEW_COMMON_H_INCLUDED

#include "lvgl.h"
#include "model/model.h"


lv_obj_t *view_common_back_button(int id);
void      view_common_dropdown_range(lv_obj_t *dd, int start, int end);
void      view_common_roller_range(lv_obj_t *roller, int start, int end);
lv_obj_t *view_common_ip_widget(lv_obj_t *root, lv_obj_t **ip1, lv_obj_t **ip2, lv_obj_t **ip3, lv_obj_t **ip4);
lv_obj_t *view_common_password_popup(model_t *model, lv_obj_t *parent, int id);
int       view_common_set_hidden(lv_obj_t *obj, int hidden);

#endif
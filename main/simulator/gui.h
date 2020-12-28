#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

#include "lvgl.h"

#include "model/model.h"

lv_indev_t *gui_init(void);
void        controller_manage_gui(model_t *model);

#endif
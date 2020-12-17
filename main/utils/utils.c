#include <stdlib.h>
#include "utils.h"
#include "lvgl.h"

void free_lv_user_data(lv_obj_t *obj) {
    free(obj->user_data);
}
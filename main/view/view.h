#ifndef VIEW_H_INCLUDED
#define VIEW_H_INCLUDED

#include "lvgl.h"

#include "view_types.h"
#include "gel/pagemanager/page_manager.h"

void view_init(model_t *model, void (*flush_cb)(struct _disp_drv_t *, const lv_area_t *, lv_color_t *),
               bool (*read_cb)(struct _lv_indev_drv_t *, lv_indev_data_t *));
void view_event(view_event_t event);
void view_change_page(model_t *model, const pman_page_t *page);
void view_rebase_page(model_t *model, const pman_page_t *page);
int  view_get_next_msg(model_t *model, view_message_t *msg, view_event_t *eventcopy);
void view_process_msg(view_page_command_t vmsg, model_t *model);
void view_register_default_callback(lv_obj_t *obj, int id);
void view_register_default_callback_number(lv_obj_t *obj, int id, int num);

void view_destroy_all(void *data, void *extra);
void view_close_all(void *data);

extern const pman_page_t page_main;
extern const pman_page_t page_settings;
extern const pman_page_t page_menu;
extern const pman_page_t page_channels;
extern const pman_page_t page_channel_settings;
extern const pman_page_t page_master_settings;
extern const pman_page_t page_minion_settings;
extern const pman_page_t page_test;
extern const pman_page_t page_cavi;
extern const pman_page_t page_channel_state;


#endif
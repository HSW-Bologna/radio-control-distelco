#include <assert.h>
#include <stdlib.h>

#include "lvgl.h"
#include "gel/pagemanager/page_manager.h"
#include "gel/collections/queue.h"

#include "model/model.h"
#include "view/view.h"
#include "theme/style.h"
#include "theme/theme.h"


#define DISP_COLOR_BUF_SIZE (320 * 240 / 8)

QUEUE_DECLARATION(event_queue, view_event_t, 32);
QUEUE_DEFINITION(event_queue, view_event_t);
static struct event_queue q;


static page_manager_t pman;




void view_init(model_t *model, void (*flush_cb)(struct _disp_drv_t *, const lv_area_t *, lv_color_t *),
               bool (*read_cb)(struct _lv_indev_drv_t *, lv_indev_data_t *)) {
    lv_init();
    lv_theme_set_act(theme_init(LV_THEME_DEFAULT_COLOR_PRIMARY, LV_THEME_DEFAULT_COLOR_SECONDARY,
                                LV_THEME_MATERIAL_FLAG_DARK | LV_THEME_MATERIAL_FLAG_NO_FOCUS,
                                LV_THEME_DEFAULT_FONT_SMALL, LV_THEME_DEFAULT_FONT_NORMAL,
                                LV_THEME_DEFAULT_FONT_SUBTITLE, LV_THEME_DEFAULT_FONT_TITLE));
    style_init();

    static lv_color_t    buf[DISP_COLOR_BUF_SIZE];
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, buf, NULL, DISP_COLOR_BUF_SIZE);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.buffer   = &disp_buf;
    disp_drv.hor_res  = 320;
    disp_drv.ver_res  = 240;
    disp_drv.flush_cb = flush_cb;
    lv_disp_drv_register(&disp_drv);

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = read_cb;
    indev_drv.type    = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);

    pman_init(&pman);
    event_queue_init(&q);
    view_change_page(model, &page_main);
}


void view_change_page_extra(model_t *model, const pman_page_t *page, void *extra) {
    assert(page != NULL);
    pman_change_page_extra(&pman, model, *page, extra);
    event_queue_init(&q);     // Butta tutti gli eventi precedenti quando cambi la pagina
    view_event((view_event_t){.code = VIEW_EVENT_CODE_OPEN});
}


void view_change_page(model_t *model, const pman_page_t *page) {
    view_change_page_extra(model, page, NULL);
}


void view_rebase_page(model_t *model, const pman_page_t *page) {
    pman_rebase_page(&pman, model, *(pman_page_t *)page);
    event_queue_init(&q);
    view_event((view_event_t){.code = VIEW_EVENT_CODE_OPEN});
}


int view_get_next_msg(model_t *model, view_message_t *msg, view_event_t *eventcopy) {
    view_event_t event;
    int          found = 0;

    if (!event_queue_is_empty(&q)) {
        event_queue_dequeue(&q, &event);
        found = 1;
    }

    if (found) {
        *msg = pman.current_page.process_event(model, pman.current_page.data, event);
        if (eventcopy)
            *eventcopy = event;
    }

    return found;
}


void view_process_msg(view_page_command_t vmsg, model_t *model) {
    if (vmsg.code == VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE) {
        assert(vmsg.page);
        view_change_page(model, vmsg.page);
    } else if (vmsg.code == VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE_EXTRA) {
        assert(vmsg.page);
        view_change_page_extra(model, vmsg.page, vmsg.extra);
    } else if (vmsg.code == VIEW_PAGE_COMMAND_CODE_BACK) {
        pman_back(&pman, model);
        event_queue_init(&q);
        view_event((view_event_t){.code = VIEW_EVENT_CODE_OPEN});
    } else if (vmsg.code == VIEW_PAGE_COMMAND_CODE_REBASE) {
        assert(vmsg.page);
        view_rebase_page(model, vmsg.page);
    } else if (vmsg.code == VIEW_PAGE_COMMAND_CODE_UPDATE) {
        pman_page_update(&pman, model);
    }
}


void view_event(view_event_t event) {
    event_queue_enqueue(&q, &event);
}


void view_destroy_all(void *data, void *extra) {
    free(data);
    free(extra);
}


void view_close_all(void *data) {
    lv_obj_clean(lv_scr_act());
}


static void page_event_cb(lv_obj_t *obj, lv_event_t event) {
    view_event_t myevent;
    myevent.code          = VIEW_EVENT_CODE_LVGL;
    myevent.lvgl.data     = lv_obj_get_user_data(obj);
    myevent.lvgl.lv_event = event;
    view_event(myevent);
}


void view_register_default_callback(lv_obj_t *obj, int id) {
    view_obj_data_t *data = malloc(sizeof(view_obj_data_t));
    data->id              = id;
    lv_obj_set_user_data(obj, (void *)data);
    lv_obj_set_event_cb(obj, page_event_cb);
}


void view_register_default_callback_number(lv_obj_t *obj, int id, int number) {
    view_obj_data_t *data = malloc(sizeof(view_obj_data_t));
    data->id              = id;
    data->number          = number;
    lv_obj_set_user_data(obj, data);
    lv_obj_set_event_cb(obj, page_event_cb);
}
#include <assert.h>
#include <stdlib.h>

#include "lvgl.h"
#include "gel/pagemanager/page_manager.h"
#include "gel/collections/queue.h"

#include "model/model.h"
#include "view/view.h"

QUEUE_DECLARATION(event_queue, view_event_t, 32);
QUEUE_DEFINITION(event_queue, view_event_t);
static struct event_queue q;


static page_manager_t pman;


void view_init(model_t *model) {
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
        view_change_page(model, vmsg.page);
    } else if (vmsg.code == VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE_EXTRA) {
        view_change_page_extra(model, vmsg.page, vmsg.extra);
    } else if (vmsg.code == VIEW_PAGE_COMMAND_CODE_BACK) {
        pman_back(&pman, model);
        event_queue_init(&q);
        view_event((view_event_t){.code = VIEW_EVENT_CODE_OPEN});
    } else if (vmsg.code == VIEW_PAGE_COMMAND_CODE_REBASE) {
        assert(vmsg.page);
        pman_rebase_page(&pman, model, *(pman_page_t *)vmsg.page);
        event_queue_init(&q);
        view_event((view_event_t){.code = VIEW_EVENT_CODE_OPEN});
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
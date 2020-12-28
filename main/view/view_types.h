#ifndef VIEW_TYPES_H_INCLUDED
#define VIEW_TYPES_H_INCLUDED

#include "model/model.h"

typedef enum {
    VIEW_EVENT_CODE_LVGL,
    VIEW_EVENT_CODE_OPEN,
    VIEW_EVENT_CODE_MODEL_UPDATE,
    VIEW_EVENT_CODE_CUSTOM,
} view_event_code_t;

typedef struct {
    view_event_code_t code;
    union {
        struct {
            int   lv_event;
            void *data;
        } lvgl;
    };
} view_event_t;


typedef enum {
    VIEW_PAGE_COMMAND_CODE_NOTHING = 0,
    VIEW_PAGE_COMMAND_CODE_REBASE,
    VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE,
    VIEW_PAGE_COMMAND_CODE_BACK,
    VIEW_PAGE_COMMAND_CODE_UPDATE,
} view_page_command_code_t;

typedef struct {
    view_page_command_code_t code;

    union {
        const void *page;
    };
} view_page_command_t;


typedef enum {
    VIEW_CONTROLLER_COMMAND_CODE_NOTHING,
} view_controller_command_code_t;


typedef struct {
    view_controller_command_code_t code;
} view_controller_command_t;


typedef struct {
    view_page_command_t       vmsg;
    view_controller_command_t cmsg;
} view_message_t;


typedef struct {
    int id, number;
} view_obj_data_t;




#endif
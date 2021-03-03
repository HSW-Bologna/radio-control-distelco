#ifndef VIEW_TYPES_H_INCLUDED
#define VIEW_TYPES_H_INCLUDED

#include "model/model.h"


typedef struct {
    int   id, number;
    void *extra;
} view_obj_data_t;


typedef enum {
    VIEW_EVENT_CODE_LVGL,
    VIEW_EVENT_CODE_OPEN,
    VIEW_EVENT_CODE_MODEL_UPDATE,
    VIEW_EVENT_CODE_TEST_MANAGEMENT_RESPONSE,
    VIEW_EVENT_CODE_CUSTOM,
    VIEW_EVENT_CODE_POWER_STATE,
} view_event_code_t;

typedef struct {
    view_event_code_t code;
    union {
        struct {
            int              lv_event;
            view_obj_data_t *data;
        } lvgl;
        uint8_t management_registers[14];
        uint8_t power_bits;
    };
} view_event_t;


typedef enum {
    VIEW_PAGE_COMMAND_CODE_NOTHING = 0,
    VIEW_PAGE_COMMAND_CODE_REBASE,
    VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE,
    VIEW_PAGE_COMMAND_CODE_CHANGE_PAGE_EXTRA,
    VIEW_PAGE_COMMAND_CODE_BACK,
    VIEW_PAGE_COMMAND_CODE_UPDATE,
} view_page_command_code_t;

typedef struct {
    view_page_command_code_t code;

    union {
        struct {
            const void *page;
            void *      extra;
        };
    };
} view_page_command_t;


typedef enum {
    VIEW_CONTROLLER_COMMAND_CODE_NOTHING,
    VIEW_CONTROLLER_COMMAND_CODE_SAVE_CONFIG,
    VIEW_CONTROLLER_COMMAND_MANAGEMENT_READ,
    VIEW_CONTROLLER_COMMAND_TEST_RELE,
    VIEW_CONTROLLER_COMMAND_TEST,
    VIEW_CONTROLLER_COMMAND_REENABLE_TX,
} view_controller_command_code_t;


typedef struct {
    view_controller_command_code_t code;

    union {
        uint32_t addr;
        uint8_t  data_reg;
        uint8_t  test;

        struct {
            size_t rele;
            int    level;
        };
    };
} view_controller_command_t;


typedef struct {
    view_page_command_t       vmsg;
    view_controller_command_t cmsg;
} view_message_t;




#endif
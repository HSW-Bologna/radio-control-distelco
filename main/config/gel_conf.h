#ifndef GEL_CONF_H_INCLUDED
#define DEL_CONF_H_INCLUDED

#include "model/model.h"
#include "view/view_types.h"

/*
 * Page manager
 */

#define PMAN_NAVIGATION_DEPTH 4

typedef view_message_t pman_message_t;

typedef view_event_t pman_event_t;

typedef void *pman_page_data_t;

typedef model_t *pman_model_t;

/*
 * PID
 */
typedef double pid_control_type_t;
typedef double pid_output_type_t;

#define GEL_PID_INTEGRAL_HISTORY_SIZE (60 * 3)
#define GEL_PID_DERIVATIVE_DELTA      12

/*
 *  Watcher
 */
#define GEL_MALLOC_AVAILABLE  1
#define GEL_STATIC_BLOCK_SIZE 0


typedef void *parameter_user_data_t;


#endif
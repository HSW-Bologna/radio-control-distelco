#include <unistd.h>
#include <SDL2/SDL.h>

#include "lvgl.h"

#include "simulator_utils.h"
#include "view/view.h"
#include "gui.h"

/**
 * A task to measure the elapsed time for LVGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void *data) {
    (void)data;

    while (1) {
        SDL_Delay(5);   /*Sleep for 5 millisecond*/
        lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
    }

    return 0;
}

int main(int argc, char *argv[]) {
    model_t model = {0};

    model_init(&model);

    gui_init();
    view_init(&model);

    /* Tick init.
     * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about
     * how much time were elapsed Create an SDL thread to do this*/
    SDL_CreateThread(tick_thread, "tick", NULL);


    for (;;) {
        controller_manage_gui(&model);
        usleep(10000UL);
    }

    return 0;
}
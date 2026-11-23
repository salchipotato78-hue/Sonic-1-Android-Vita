#include "utils/init.h"
#include "utils/glutil.h"
#include "sonic1/driver.h"

#include <psp2/kernel/threadmgr.h>

#include <falso_jni/FalsoJNI.h>
#include <so_util/so_util.h>

#ifndef NDK_PORT
#include "reimpl/controls.h"
#endif

int _newlib_heap_size_user = 256 * 1024 * 1024;

#ifdef USE_SCELIBC_IO
int sceLibcHeapSize = 4 * 1024 * 1024;
#endif

so_module so_mod;

int main() {
    /* Carga, relocación, resolución de imports y init del .so (ver utils/init.c). */
    soloader_init_all();

    gl_init();

    if (sonic1_init(&so_mod, DATA_PATH "Data.rsdk.xmf", DATA_PATH "SData.bin") < 0) {
        /* TODO: mostrar error al usuario con utils/dialog.h */
        sceKernelExitDeleteThread(0);
    }

    while (1) {
#ifndef NDK_PORT
        /* Sin esto no se leen botones ni touch: controls_poll() nunca se llamaba. */
        controls_poll();
#endif
        sonic1_frame();
        gl_swap();
    }

    sonic1_shutdown();
    sceKernelExitDeleteThread(0);
}

#ifndef NDK_PORT
void controls_handler_key(int32_t keycode, ControlsAction action) {
    sonic1_key(keycode, (int32_t)action); /* keycode se reinterpreta como índice; ver docs/SYMBOLS.md */
}

void controls_handler_touch(int32_t id, float x, float y, ControlsAction action) {
    sonic1_touch(id, x, y, (int32_t)action);
}

void controls_handler_analog(ControlsStickId which, float x, float y, ControlsAction action) {
    /* Sin implementar: el juego original de Android usa táctil. */
    (void)which; (void)x; (void)y; (void)action;
}
#endif

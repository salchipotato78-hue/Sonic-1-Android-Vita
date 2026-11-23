/*
 * Driver JNI para libsonic1native.so (Sonic 1, Android 2013).
 *
 * BORRADOR. Las firmas y el orden de las llamadas están sin verificar:
 * hay que confirmarlos desmontando las funciones Java_com_sega_sonic1_* del .so.
 * Todos los puntos marcados con TODO deben revisarse antes de probar en hardware.
 */
#include "driver.h"

#include <stdio.h>
#include <falso_jni/FalsoJNI.h>
#include "utils/logger.h"

typedef void *JNIEnvPtr;
typedef void *JObj;
typedef void *JStr;

typedef void (*fn_str)(JNIEnvPtr, JObj, JStr);
typedef void (*fn_void)(JNIEnvPtr, JObj);
typedef void (*fn_int2)(JNIEnvPtr, JObj, int32_t, int32_t);                 /* TODO: confirmar */
typedef void (*fn_key)(JNIEnvPtr, JObj, int32_t, int32_t);                  /* TODO: confirmar */
typedef void (*fn_touch)(JNIEnvPtr, JObj, int32_t, float, float);           /* TODO: confirmar */
typedef void (*fn_touch_id)(JNIEnvPtr, JObj, int32_t);                      /* TODO: confirmar */

static so_module *g_mod;
static JNIEnvPtr  g_env;      /* TODO: obtener de FalsoJNI */
static JObj       g_view;     /* TODO: objeto fake de Sonic1View */
static JObj       g_renderer; /* TODO: objeto fake de Sonic1Renderer */
static JObj       g_activity; /* TODO: objeto fake de Sonic1Activity */

#define SYM(cls, fn) ((void *)so_symbol(g_mod, "Java_com_sega_sonic1_" cls "_" fn))

/* TODO: sustituir por la creación real de jstring con FalsoJNI (NewStringUTF). */
static JStr make_jstring(const char *s) { return (JStr)s; }

static fn_str  p_set_apk_dir, p_set_save;
static fn_int2 p_set_dims;
static fn_void p_startup, p_create_tex, p_create_audio, p_start_audio, p_frame, p_shutdown;
static fn_key  p_key;
static fn_touch p_touch_set, p_touch_add;
static fn_touch_id p_touch_remove, p_touch_clear;

static int resolve_all(void) {
    p_set_apk_dir = SYM("Sonic1View", "SetAPKDirectory");
    p_set_save    = SYM("Sonic1View", "SetSaveFileName");
    p_set_dims    = SYM("Sonic1View", "SetScreenDimensions");
    p_startup     = SYM("Sonic1Renderer", "StartupRetroEngine");
    p_create_tex  = SYM("Sonic1Renderer", "CreateDeviceTextures");
    p_create_audio = SYM("Sonic1Renderer", "CreateAudioBufferQueue");
    p_start_audio = SYM("Sonic1Renderer", "StartAudioBufferQueue");
    p_frame       = SYM("Sonic1Renderer", "ProcessMainLoop");
    p_shutdown    = SYM("Sonic1Activity", "ShutdownRetroEngine");
    p_key         = SYM("Sonic1Activity", "SetKeyData");
    p_touch_set   = SYM("Sonic1Activity", "SetTouch");
    p_touch_add   = SYM("Sonic1Activity", "AddTouch");
    p_touch_remove = SYM("Sonic1Activity", "RemoveTouch");
    p_touch_clear = SYM("Sonic1Activity", "ClearTouchData");

    if (!p_set_apk_dir || !p_set_save || !p_set_dims || !p_startup || !p_frame) {
        l_fatal("Faltan símbolos JNI obligatorios de libsonic1native.so (ver docs/SYMBOLS.md).");
        return -1;
    }
    return 0;
}

int sonic1_init(so_module *mod, const char *data_dir, const char *save_file) {
    g_mod = mod;
    if (resolve_all() < 0)
        return -1;

    /* TODO: crear g_view, g_renderer y g_activity con FalsoJNI. */

    p_set_apk_dir(g_env, g_view, make_jstring(data_dir));
    p_set_save(g_env, g_view, make_jstring(save_file));
    p_set_dims(g_env, g_view, 480, 272); /* TODO: confirmar orden y resolución */

    p_startup(g_env, g_renderer);
    if (p_create_tex)   p_create_tex(g_env, g_renderer);
    if (p_create_audio) p_create_audio(g_env, g_renderer);
    if (p_start_audio)  p_start_audio(g_env, g_renderer);

    l_success("Sonic 1: motor inicializado (borrador).");
    return 0;
}

void sonic1_frame(void) {
    if (p_frame)
        p_frame(g_env, g_renderer);
}

void sonic1_shutdown(void) {
    if (p_shutdown)
        p_shutdown(g_env, g_activity);
}

void sonic1_key(int32_t keycode, int32_t action) {
    if (p_key)
        p_key(g_env, g_activity, keycode, action); /* TODO: confirmar semántica de action */
}

void sonic1_touch(int32_t id, float x, float y, int32_t action) {
    /* TODO: confirmar cuándo usar Set/Add/Remove. Esto es una suposición. */
    if (action == 0 && p_touch_add)
        p_touch_add(g_env, g_activity, id, x, y);
    else if (action == 1 && p_touch_remove)
        p_touch_remove(g_env, g_activity, id);
}

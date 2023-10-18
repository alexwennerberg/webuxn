#include <emscripten.h>
#include <string.h>

#include "uxn.h"
#include "devices/system.h"
/* #include "devices/console.h" */
#include "devices/screen.h"
#include "devices/audio.h"
#include "devices/controller.h"
#include "devices/mouse.h"
/* #include "devices/datetime.h" */

#define HEIGHT 256
#define WIDTH 384

extern void printChar (const char c);
extern void printStr (const char* str);
extern void render (const Uint32* bg, const Uint32* fg);
extern void getDateTime (const Uint8* ptr);

Uxn uxn = {0};
Uint16 screen_vector;

Uint32 toAbgr (Uint32 argb) {
    Uint8 b = (argb & 0x000000ff);
    Uint8 g = (argb & 0x0000ff00) >> 8;
    Uint8 r = (argb & 0x00ff0000) >> 16;
    Uint8 a = (argb & 0xff000000) >> 24;
    return (a << 24) | (b << 16) | (g << 8) | r;
}

Uint8
emu_dei(Uxn *u, Uint8 addr)
{
    Uint8 p = addr & 0x0f, d = addr & 0xf0;
    switch(d) {
    case 0x00: return system_dei(u, addr);
    case 0x20: return screen_dei(u, addr);
    /* case 0x30: return audio_dei(0, &u->dev[d], p); */
    /* case 0x40: return audio_dei(1, &u->dev[d], p); */
    /* case 0x50: return audio_dei(2, &u->dev[d], p); */
    /* case 0x60: return audio_dei(3, &u->dev[d], p); */
    /* case 0xc0: return datetime_dei(u, addr); */
    }
    return u->dev[addr];
}

void
emu_deo(Uxn *u, Uint8 addr)
{
    Uint8 p = addr & 0x0f, d = addr & 0xf0;
    switch(d) {
    case 0x00:
        system_deo(u, &u->dev[d], p);
        if(p > 0x7 && p < 0xe)
            screen_palette(&u->dev[0x8]);
        break;
    /* case 0x10: console_deo(&u->dev[d], p); break; */
    case 0x20: screen_deo(u->ram, &u->dev[d], p); break;
    /* case 0x30: audio_deo(0, &u->dev[d], p, u); break; */
    /* case 0x40: audio_deo(1, &u->dev[d], p, u); break; */
    /* case 0x50: audio_deo(2, &u->dev[d], p, u); break; */
    /* case 0x60: audio_deo(3, &u->dev[d], p, u); break; */
    /* case 0xa0: file_deo(0, u->ram, &u->dev[d], p); break; */
    /* case 0xb0: file_deo(1, u->ram, &u->dev[d], p); break; */
    }
}


Uint8* EMSCRIPTEN_KEEPALIVE getRomPtr () {
    return uxn.ram + PAGE_PROGRAM;
}

void* EMSCRIPTEN_KEEPALIVE getStatePtr () {
    return &uxn;
}

int EMSCRIPTEN_KEEPALIVE getStateSize () {
    return sizeof(uxn);
}

float* EMSCRIPTEN_KEEPALIVE getAudioSamples () {
  // TODO (WONTDO)
}

void EMSCRIPTEN_KEEPALIVE init () {
    // Reset state
    memset(&uxn, 0, sizeof(uxn));

    // Statically allocate pixels to avoid depending on malloc
    /* system_connect(0x0, SYSTEM_VERSION, SYSTEM_DEIMASK, SYSTEM_DEOMASK); */
    /* system_connect(0x1, CONSOLE_VERSION, CONSOLE_DEIMASK, CONSOLE_DEOMASK); */
    system_connect(0x2, SCREEN_VERSION, SCREEN_DEIMASK, SCREEN_DEOMASK);
    system_connect(0x3, AUDIO_VERSION, AUDIO_DEIMASK, AUDIO_DEOMASK);
    system_connect(0x4, AUDIO_VERSION, AUDIO_DEIMASK, AUDIO_DEOMASK);
    system_connect(0x5, AUDIO_VERSION, AUDIO_DEIMASK, AUDIO_DEOMASK);
    system_connect(0x6, AUDIO_VERSION, AUDIO_DEIMASK, AUDIO_DEOMASK);
    system_connect(0x8, CONTROL_VERSION, CONTROL_DEIMASK, CONTROL_DEOMASK);
    system_connect(0x9, MOUSE_VERSION, MOUSE_DEIMASK, MOUSE_DEOMASK);
    /* system_connect(0xa, FILE_VERSION, FILE_DEIMASK, FILE_DEOMASK); */
    /* system_connect(0xb, FILE_VERSION, FILE_DEIMASK, FILE_DEOMASK); */
    /* system_connect(0xc, DATETIME_VERSION, DATETIME_DEIMASK, DATETIME_DEOMASK); */
}

void EMSCRIPTEN_KEEPALIVE runMain () {
    uxn_eval(&uxn, PAGE_PROGRAM);
}

void EMSCRIPTEN_KEEPALIVE onUpdate () {
    for (;;) {
      uxn_eval(&uxn, screen_vector);
    /* if (reqdraw) { */
    /*     reqdraw = 0; */

    /* screen_redraw(&uxn); */
    /* int ll = WIDTH*HEIGHT; */
    /* Uint32 bg_abgr[ll], fg_abgr[ll]; */
    /* Uint32* bg = uxn_screen.pixels; */ 
    /* // TODO ?? */
    /* Uint32* fg = uxn_screen.pixels; */
    /* for (int ii = 0; ii < ll; ++ii) { */
        /* bg_abgr[ii] = toAbgr(bg[ii]); */
        /* fg_abgr[ii] = toAbgr(fg[ii]); */
        /* render(bg_abgr, fg_abgr); */
    /* } */
  }
}

void EMSCRIPTEN_KEEPALIVE onPointerEvent (int x, int y, int buttons) {
  // TODO
}

void EMSCRIPTEN_KEEPALIVE onWheelEvent (int y) {
  // TODO
}

void EMSCRIPTEN_KEEPALIVE onKeyboardEvent (int buttons, int charCode) {
  // TODO
}

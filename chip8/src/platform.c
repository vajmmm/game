/* platform.c — SDL3 平台层：窗口、显示、键盘。
 * 把 CHIP-8 的 64×32 单色显示放大 10 倍画到 640×320 窗口里。
 * 键盘映射：物理 1234/QWER/ASDF/ZXCV → CHIP-8 0x0..0xF */

#include "platform.h"

#include <SDL3/SDL.h>
#include <stdio.h>

#define WINDOW_W  (DISP_W * 10)   /* 640 */
#define WINDOW_H  (DISP_H * 10)   /* 320 */
#define PIXEL_SCALE 10
#define FPS 60
#define FRAME_TIME (1000.0f / FPS)   /* ~16.6 ms */

struct Platform {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    SDL_Texture  *texture;       /* 64×32 纹理，直接用 CHIP-8 display 当像素源 */
};

/* 物理 SDL scancode → CHIP-8 键号 (0x0..0xF)
 * 布局：
 *   1 2 3 4   →  0x1 0x2 0x3 0xC
 *   Q W E R   →  0x4 0x5 0x6 0xD
 *   A S D F   →  0x7 0x8 0x9 0xE
 *   Z X C V   →  0xA 0x0 0xB 0xF
 */
static const SDL_Scancode keymap[16] = {
    SDL_SCANCODE_X,    /* 0x0 */
    SDL_SCANCODE_1,    /* 0x1 */
    SDL_SCANCODE_2,    /* 0x2 */
    SDL_SCANCODE_3,    /* 0x3 */
    SDL_SCANCODE_Q,    /* 0x4 */
    SDL_SCANCODE_W,    /* 0x5 */
    SDL_SCANCODE_E,    /* 0x6 */
    SDL_SCANCODE_A,    /* 0x7 */
    SDL_SCANCODE_S,    /* 0x8 */
    SDL_SCANCODE_D,    /* 0x9 */
    SDL_SCANCODE_Z,    /* 0xA */
    SDL_SCANCODE_C,    /* 0xB */
    SDL_SCANCODE_4,    /* 0xC */
    SDL_SCANCODE_R,    /* 0xD */
    SDL_SCANCODE_F,    /* 0xE */
    SDL_SCANCODE_V,    /* 0xF */
};

Platform *platform_init(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return NULL;
    }

    Platform *p = SDL_calloc(1, sizeof(*p));
    if (!p) return NULL;

    /* SDL3 用 SDL_CreateWindow + SDL_CreateRenderer 分两步
     * SDL2 的 SDL_CreateWindowAndRenderer 在 SDL3 里也还有，但分开更清晰 */
    p->window = SDL_CreateWindow("CHIP-8", WINDOW_W, WINDOW_H, 0);
    if (!p->window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        goto fail;
    }

    p->renderer = SDL_CreateRenderer(p->window, NULL);
    if (!p->renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        goto fail;
    }

    /* 64×32 的 ARGB8888 纹理，逐像素更新。
     * SDL3 用 SDL_TEXTUREACCESS_STREAMING 让能 Lock/Unlock 更新像素。 */
    p->texture = SDL_CreateTexture(p->renderer,
                                   SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   DISP_W, DISP_H);
    if (!p->texture) {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        goto fail;
    }

    return p;

fail:
    if (p->texture) SDL_DestroyTexture(p->texture);
    if (p->renderer) SDL_DestroyRenderer(p->renderer);
    if (p->window) SDL_DestroyWindow(p->window);
    SDL_free(p);
    SDL_Quit();
    return NULL;
}

void platform_destroy(Platform *p) {
    if (!p) return;
    if (p->texture) SDL_DestroyTexture(p->texture);
    if (p->renderer) SDL_DestroyRenderer(p->renderer);
    if (p->window) SDL_DestroyWindow(p->window);
    SDL_free(p);
    SDL_Quit();
}

/* 把 SDL 当前键盘状态（哪些键被按住）同步到 c->keys[]。
 * 这是"按住状态"轮询——FX0A 的"按下事件"在 chip8.c 里单独处理。 */
static void update_keys(Chip8 *c) {
    const bool *state = SDL_GetKeyboardState(NULL);
    for (int i = 0; i < 16; i++) {
        c->keys[i] = state[keymap[i]] ? 1 : 0;
    }
}

bool platform_poll(Platform *p, Chip8 *c) {
    (void)p;  /* 暂时不用 p，但留着接口 */
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_EVENT_QUIT) return false;
    }
    update_keys(c);
    return true;
}

void platform_render(Platform *p, const Chip8 *c) {
    /* Lock 纹理拿到一行一行的像素缓冲，按 display[] 写黑白颜色 */
    uint32_t *pixels = NULL;
    int pitch = 0;
    if (!SDL_LockTexture(p->texture, NULL, (void **)&pixels, &pitch)) {
        fprintf(stderr, "SDL_LockTexture failed: %s\n", SDL_GetError());
        return;
    }
    int cols = pitch / (int)sizeof(uint32_t);   /* 每行有多少像素（可能 > 64，因为对齐） */
    for (int y = 0; y < DISP_H; y++) {
        for (int x = 0; x < DISP_W; x++) {
            uint8_t on = c->display[y * DISP_W + x];
            /* 像素亮 = 白 (0xFFFFFFFF)，灭 = 黑 (0xFF000000) */
            pixels[y * cols + x] = on ? 0xFFFFFFFF : 0xFF000000;
        }
    }
    SDL_UnlockTexture(p->texture);

    /* 把 64×32 纹理拉伸画到 640×320 窗口上 */
    SDL_RenderClear(p->renderer);
    SDL_FRect dst = {0, 0, WINDOW_W, WINDOW_H};
    SDL_RenderTexture(p->renderer, p->texture, NULL, &dst);
    SDL_RenderPresent(p->renderer);
}

void platform_delay(const Platform *p) {
    (void)p;
    /* 简单按 60FPS 等。SDL3 的 SDL_Delay 接受毫秒。 */
    SDL_Delay((Uint32)FRAME_TIME);
}

void platform_beep(const Platform *p, bool on) {
    (void)p; (void)on;
    /* 阶段 5 再加 SDL3 音频。先空实现，模拟器不发声也能跑游戏。 */
}
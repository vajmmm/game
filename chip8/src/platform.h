#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include "chip8.h"

/* 平台层：封装 SDL3 窗口、显示、键盘。
 * chip8.c 不依赖 SDL，只通过这个层跟外界打交道。 */

typedef struct Platform Platform;  /* 不透明指针，main 不用知道内部细节 */

Platform *platform_init(void);
void      platform_destroy(Platform *p);

/* 处理事件、更新键盘状态到 c->keys[]。
 * 返回 false 表示用户关窗口，主循环应退出。 */
bool      platform_poll(Platform *p, Chip8 *c);

/* 把 c->display[] 渲染到窗口。每帧调一次。 */
void      platform_render(Platform *p, const Chip8 *c);

/* 帧率控制：等到下一帧时刻 (~16.6ms)。 */
void      platform_delay(const Platform *p);

/* 蜂鸣：sound_timer > 0 时调用。先空实现，阶段 5 再加音频。 */
void      platform_beep(const Platform *p, bool on);

#endif /* PLATFORM_H */
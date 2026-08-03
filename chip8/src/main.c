/* main.c — 阶段 3：SDL 主循环。
 * 每帧跑 10 条 chip8_step ≈ 600 Hz CPU，60 Hz 定时器递减。 */

#include "chip8.h"
#include "platform.h"

#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char **argv) {
#ifdef _WIN32
    /* Windows 终端默认是 GBK，切到 UTF-8 才能正确显示中文提示 */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc < 2) {
        fprintf(stderr, "用法 / Usage: %s <rom.ch8>\n", argv[0]);
        return 1;
    }

    Platform *p = platform_init();
    if (!p) return 1;

    Chip8 c;
    chip8_init(&c);

    int n = chip8_load_rom(&c, argv[1]);
    if (n < 0) {
        platform_destroy(p);
        return 1;
    }
    printf("Loaded %d bytes from %s\n", n, argv[1]);

    /* 主循环：SDL 事件 → CPU 执行 → 定时器递减 → 渲染 → 帧率控制 */
    bool running = true;
    while (running) {
        /* 1. 处理 SDL 事件 + 更新键盘 */
        running = platform_poll(p, &c);

        /* 2. CPU 该帧执行 10 条指令 ≈ 600 Hz */
        for (int i = 0; i < 10; i++) {
            chip8_step(&c);
        }

        /* 3. 定时器 60 Hz 递减 */
        if (c.delay_timer > 0) c.delay_timer--;
        if (c.sound_timer > 0) c.sound_timer--;
        platform_beep(p, c.sound_timer > 0);

        /* 4. 渲染 */
        if (c.draw_flag) {
            platform_render(p, &c);
            c.draw_flag = false;
        }

        /* 5. 控制帧率到 60 FPS */
        platform_delay(p);
    }

    platform_destroy(p);
    return 0;
}
# 当前未解决问题

## Delay Timer Test ROM 无输出

- **现象：** `.\chip8.exe 'roms\Delay Timer Test [Matthew Mikolay, 2010].ch8'` 画面一直没变化
- **可能原因：** delay_timer 递减逻辑、FX07（读 DT）或 FX15（设 DT）指令实现有误
- **状态：** 已决定不调试 — 真实游戏 Pong 可正常运行，说明 timer 实际工作正常；此测试 ROM 问题用户判断无收益（2026-08-03 项目收尾）

## rand() 缺少头文件

- **现象：** chip8.c 使用了 rand()（RND 指令）但没有 `#include <stdlib.h>` 和 `#include <time.h>`
- **修复：** 在 chip8.c 顶部添加 `#include <stdlib.h>` 和 `#include <time.h>`，在 chip8_init() 中添加 `srand((unsigned)time(NULL))`
- **状态：** 已修复（chip8.c 已包含头文件并调用 srand）

## FX0A 按键等待是电平触发

- **现象：** FX0A 用轮询方式检测按键，按住不放会重复触发
- **修复方向：** 改为边沿触发（检测按键按下事件），需要在 platform.c 中跟踪按键变化
- **状态：** 已决定不处理 — 用户判断边际收益低（2026-08-03 项目收尾），标记为"可回访"

## SDL3 平台层用户理解不完整

- **用户原话：** "sdl的内容不需要我看吗，我发现这个好像被略过了 你直接做完了" → "算了这个先不管，我以后再了解"
- **状态：** 用户明确表示以后再了解，不要在当前会话中主动提出。项目收尾后若回访，此为用户优先项。

## 编译命令路径问题

- **现象：** 编译时需要正确指定 SDL3 include 路径（`-I SDL3/include` 而非 `-I SDL3/include/SDL3`）
- **原因：** SDL3 头文件内部用 `#include <SDL3/SDL_stdinc.h>`，需要在 include 的父目录
- **状态：** 已解决，记录在文档中

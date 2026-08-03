# 03 - 分阶段实现指南

这份文档给你每阶段的代码骨架和关键提示，但不会替你写完整答案。**先自己尝试，再看这里的对照说明**——卡住时往下读一行，不要一次读完。

## 阶段 1: 项目骨架与状态建模

### 目标

让程序能：编译运行 → 把一个 .ch8 文件加载到模拟器内存 → 打印前 32 字节确认加载正确。

### 文件结构

```
src/
├── chip8.h    ← 只放类型定义和函数声明
├── chip8.c    ← 实现 CPU 初始化和 ROM 加载
└── main.c     ← 解析命令行参数、调用 chip8
```

### chip8.h 骨架

```c
#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

#define MEM_SIZE   4096
#define NUM_REGS   16
#define STACK_SIZE 16
#define DISP_W     64
#define DISP_H     32

typedef struct {
    uint8_t  memory[MEM_SIZE];
    uint8_t  V[NUM_REGS];
    uint16_t I;
    uint16_t pc;
    uint16_t stack[STACK_SIZE];
    uint8_t  sp;
    uint8_t  delay_timer;
    uint8_t  sound_timer;
    uint8_t  display[DISP_W * DISP_H];
    uint8_t  keys[16];
    bool     draw_flag;
} Chip8;

void chip8_init(Chip8 *c);
int  chip8_load_rom(Chip8 *c, const char *path);
void chip8_step(Chip8 *c);    // 阶段 2 才会实现

#endif
```

### 关键函数

**`chip8_init`**：把整个结构体清零（`memset`），然后：
- `pc = 0x200`
- 加载字体到 `memory[0x000]` 开始（80 字节，见 01 文档）

**`chip8_load_rom`**：用 `fopen(path, "rb")` 以二进制打开 .ch8 文件，读入 `memory + 0x200`。
- ROM 最大能多大？4096 - 512 = 3584 字节。超出要报错。
- 用 `fread` 一次读完更简洁。
- 返回读取的字节数，方便 main 打印确认。

**`main`**：检查 argc、调用 init、load、打印 hex dump。

### 完成标志

```bash
./chip8 roms/ibm_logo.ch8
```

输出形如：
```
Loaded 132 bytes from roms/ibm_logo.ch8
0x0200: 00 E0 A2 2A ...
0x0210: ...
```

### 你可能踩的坑

- fopen 模式忘记 `"rb"`：在 Windows 上文本模式会改 \n，导致 ROM 读坏
- 字体地址搞错：是 `0x000`，不是 `0x200`
- 结构体直接 `= {0}` 也行，但记得 PC 还得显式设 `0x200`

---

## 阶段 2: 取指-译码-执行循环

### 目标

不接 SDL，先把核心循环跑通。用一个测试 ROM（比如 IBM Logo 或 chip8-test-suite 的某些子 ROM），看 `display[]` 数组里是否出现非零值。

### 主循环（main 里或单独函数）

```c
while (running) {
    chip8_step(&c);
    if (c.draw_flag) {
        // 临时：把 display[] 按字符打印到终端
        print_display(&c);
        c.draw_flag = false;
    }
    // 暂时不接定时器，阶段 4 再加 60Hz 节拍
}
```

### chip8_step 骨架

```c
void chip8_step(Chip8 *c) {
    uint16_t opcode = (c->memory[c->pc] << 8) | c->memory[c->pc + 1];
    c->pc += 2;

    uint8_t x = (opcode >> 8) & 0x0F;
    uint8_t y = (opcode >> 4) & 0x0F;
    uint8_t n = opcode & 0x000F;
    uint8_t nn = opcode & 0x00FF;
    uint16_t nnn = opcode & 0x0FFF;

    switch (opcode & 0xF000) {
    case 0x0000:
        switch (opcode & 0x00FF) {
        case 0xE0: /* CLS */ memset(c->display, 0, sizeof(c->display)); c->draw_flag = true; break;
        case 0xEE: /* RET */ c->pc = c->stack[--c->sp]; break;
        default: /* 0NNN SYS 在现代实现中通常忽略 */ break;
        }
        break;
    case 0x1000: c->pc = nnn; break;
    case 0x2000: c->stack[c->sp++] = c->pc; c->pc = nnn; break;
    case 0x6000: c->V[x] = nn; break;
    case 0x7000: c->V[x] += nn; break;   // 不影响 VF
    case 0xA000: c->I = nnn; break;
    case 0xD000: draw_sprite(c, x, y, n); break;  // 见下
    // ... 其他指令
    default: printf("Unknown opcode: 0x%04X at 0x%03X\n", opcode, c->pc - 2); break;
    }
}
```

### DXYN 绘制（最复杂的指令之一，仔细写）

```c
void draw_sprite(Chip8 *c, uint8_t x, uint8_t y, uint8_t height) {
    c->V[0xF] = 0;
    uint8_t vx = c->V[x] % DISP_W;     // 边界处理方案：取模
    uint8_t vy = c->V[y] % DISP_H;

    for (int row = 0; row < height; row++) {
        uint8_t sprite_byte = c->memory[c->I + row];
        for (int col = 0; col < 8; col++) {
            if (sprite_byte & (0x80 >> col)) {
                int px = vx + col;
                int py = vy + row;
                // 边界裁剪（最简单实现）
                if (px >= DISP_W || py >= DISP_H) continue;
                int idx = py * DISP_W + px;
                if (c->display[idx] == 1) c->V[0xF] = 1;  // 碰撞
                c->display[idx] ^= 1;
            }
        }
    }
    c->draw_flag = true;
}
```

### 阶段 2 至少实现这些指令

- `00E0` CLS
- `00EE` RET
- `1NNN` JP
- `2NNN` CALL
- `6XNN` LD Vx, NN
- `7XNN` ADD Vx, NN
- `ANNN` LD I
- `DXYN` DRW

够让 IBM Logo ROM 跑出画面（虽然现在还没有窗口，能从 `display[]` 数组上看到变化就行）。

### 怎么"看到" display 发生了变化

写一个简易打印函数：

```c
void print_display(const Chip8 *c) {
    for (int y = 0; y < DISP_H; y++) {
        for (int x = 0; x < DISP_W; x++) {
            putchar(c->display[y * DISP_W + x] ? '#' : ' ');
        }
        putchar('\n');
    }
}
```

IBM Logo ROM 跑完后会画出 "IBM" 三个字——能在 # 符号阵列里认出形状就成功了。

### 完成标志

跑 IBM Logo ROM，终端里能看到 IBM 标志（虽然终端字符是竖长的，比例不对，能辨认即可）。

---

## 阶段 3: 显示与输入（接 SDL2）

### 目标

告别终端字符，用窗口看到真正的 64×32 像素显示。键盘能控制游戏。

### 安装 SDL2

- Linux: `sudo apt install libsdl2-dev`
- macOS: `brew install sdl2`
- Windows MSYS2: `pacman -S mingw-w64-x86_64-SDL2`

### CMakeLists.txt 加入

```cmake
find_package(SDL2 REQUIRED)
target_link_libraries(chip8 PRIVATE SDL2::SDL2)
```

### platform.c 骨架职责

提供：
- `void platform_init()` — 创建窗口、renderer、texture
- `void platform_update_display(Chip8 *c)` — 把 `c->display[]` 渲染到纹理
- `int  platform_poll_keys(Chip8 *c)` — 处理 SDL 事件，更新 `c->keys[]`，返回是否应该退出
- `void platform_beep(Chip8 *c)` — `sound_timer > 0` 时发蜂鸣（可暂时空实现，阶段 4 再细化）
- `void platform_delay(uint32_t ms)` — 控制帧率

### SDL 核心循环结构

```c
while (running) {
    // 1. 处理事件
    running = platform_poll_keys(&c);

    // 2. 该帧 CPU 执行多少条指令？按 60Hz 帧率 / 10 条 = 600 Hz
    for (int i = 0; i < 10; i++) chip8_step(&c);

    // 3. 定时器递减
    if (c.delay_timer > 0) c.delay_timer--;
    if (c.sound_timer > 0) c.sound_timer--;

    // 4. 刷新画面
    if (c.draw_flag) {
        platform_update_display(&c);
        c.draw_flag = false;
    }

    // 5. 等下一帧 (~16.6ms)
    platform_delay(16);
}
```

### 键盘映射示例（4×4 网格）

```c
int keymap[16] = {
    SDLK_x, SDLK_1, SDLK_2, SDLK_3,
    SDLK_q, SDLK_w, SDLK_e, SDLK_a,
    SDLK_s, SDLK_d, SDLK_z, SDLK_c,
    SDLK_4, SDLK_r, SDLK_f, SDLK_v,
};
```

调整到你顺手即可。

### 完成标志

能跑 PONG 类游戏，能用键盘控制一方。

### 你可能踩的坑

- SDL 窗口不显示：先检查 SDL 是否正确初始化、事件循环是否在跑
- 画面颜色不对：先全填黑、白不要硬编码 RGB 255，有些后端会相反。建一个 `Uint32 pixels[DISP_W*DISP_H]`，每次 update 时按 `display[i]` 设成 0x00FFFFFF 或 0x00000000 再 `SDL_UpdateTexture`
- 程序吃了 100% CPU：你忘了 `platform_delay` 或 SDL_Delay

---

## 阶段 4: 补齐指令集与测试

### 目标

实现剩余指令（条件跳转、逻辑运算、子程序、定时器、随机数、FX0A 等），用测试 ROM 验证。

### 剩余指令清单

- `3XNN` / `4XNN` / `5XY0` / `9XY0` 条件跳转
- `8XY1` ~ `8XY7` / `8XYE` 逻辑和算术
- `BNNN` 跳转偏移
- `CXNN` 随机数
- `EX9E` / `EXA1` 键检测
- `FX07` / `FX0A` / `FX15` / `FX18` 定时器
- `FX1E` / `FX29` / `FX33` / `FX55` / `FX65` I 操作

照 `02-instruction-set.md` 逐条实现，每加一条就重新跑测试 ROM 看是否仍正常。

### 渐进式调试策略

跑某个测试 ROM 失败时：

1. **打开调试模式**：在 `chip8_step` 开头打印 `pc` 和 `opcode`。
2. 找到出问题的位置。比如报告某条指令行为不对。
3. 对比 `02-instruction-set.md` 确认语义。
4. 修。重跑。

### 推荐测试 ROM（自己搜索）

- **chip8-test-suite**（Timendus 出品，最全面）：逐条指令验证
- **BC_test.ch8**：经典的算术 / 逻辑指令测试
- **IBM Logo**：早期绘制测试
- **Maze.ch8 / Pong.ch8**：跑通后再上复杂游戏

### 完成标志

通过 chip8-test-suite 的全部分项测试。同时能跑 PONG、TETRIS、TANK 等经典游戏不崩溃。

---

## 阶段 5: 优化与扩展

到这一步你已经达成核心目标。下面是一些可选方向，挑感兴趣的做：

### 调试器
- 添加 `--debug` 参数，进入单步模式（F10 单步，F5 继续）
- 用 ncurses 做个简单的寄存器/内存查看窗
- 这能让以后调试任何模拟器都更轻松

### SUPERCHIP 扩展
- 128×64 显示模式
- 额外指令（滚动屏幕、音频更细等）
- 这扩展能跑更多游戏，但要去学一个新世界

### 用 Rust 重写
- 拿 C 版作为参考实现
- 重点体会：
  - `Chip8` 状态现在是 `struct`，所有权很自然
  - `memory` / `display` / `keys` 等共享读写，Rust 强迫你想清楚借用关系
  - SDL2 有 `sdl2` crate，用法对照 C 版
- 这是一次"低强度 Rust 入门"，因为你已经懂逻辑

### 6502 真实 CPU
- APPLE II、NES、Commodore 64 的 CPU
- ~56 条指令、变长编码、多个寻址模式
- 比 CHIP-8 数据量大但社区资料更丰富
- 是模拟器爱好者的"必修课"之一

### 自定义指令集
- 你在图灵完备里造过自己的 CPU——把它的指令集搬到模拟器里
- 写汇编器 + 模拟器 + 简单示例
- 这是把游戏里的"个人 CPU"在现实里延续下去的方式

---

## 关于"什么时候看这份文档"

这套指南刻意把每阶段切成"目标 → 骨架 → 完成标志 → 常见坑"四段。**真的卡了再往下读对应小节**。提前读完整骨架会让学习收益大打折扣——你练的是"把硬件直觉转成代码的能力"，照抄就少练一次。
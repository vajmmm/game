# 02 - CHIP-8 指令集速查

所有指令都是 2 字节（16 位），大端存储。即 `memory[pc]` 是高字节，`memory[pc+1]` 是低字节。取出后拼成 `uint16_t opcode = (memory[pc] << 8) | memory[pc+1]`。

## 指令格式中的通配符

一条指令的十六进制表示里，不同位置的字母含义：

| 位置 | 含义 |
|------|------|
| `NNN` | 12 位地址 (0..0xFFF) |
| `NN` | 8 位立即数 (常量) |
| `X`  | 4 位，指 V0..VF 中的一个寄存器 (低 4 位取出来表示寄存器编号 0..15) |
| `Y`  | 同上，另一个寄存器编号 |
| `N`  | 4 位 (常用作行数 / 字节数) |

举例：`DXYN` 表示 opcode 的高 4 位是 `D`，接下来 4 位是寄存器编号 X，再 4 位是 Y，最低 4 位是 N。比如 `D014` 表示"在 (V0, V1) 位置画 4 行高精灵"。

## 译码套路

不要一条条 if 去比，用"高层 4 位 + 细分"的方式：

```c
switch (opcode & 0xF000) {
case 0x0000:
    switch (opcode & 0x00FF) {
    case 0xE0: /* CLS */ break;
    case 0xEE: /* RET */ break;
    }
    break;
case 0x1000: /* 1NNN: JP */ break;
case 0x2000: /* 2NNN: CALL */ break;
case 0x3000: /* 3XNN: SE Vx, NN */ break;
...
case 0xF000:
    switch (opcode & 0x00FF) {
    case 0x07: /* FX07 */ break;
    case 0x15: /* FX15 */ break;
    ...
    }
    break;
}
```

## 完整指令表

### 0字号 - 系统控制

| 指令 | 名称 | 说明 |
|------|------|------|
| `00E0` | CLS | 清屏：把 `display[]` 全部置 0，设 `draw_flag` |
| `00EE` | RET | 从子程序返回：`pc = stack[--sp]` |

### 1字号 - 跳转

| 指令 | 名称 | 说明 |
|------|------|------|
| `1NNN` | JP NNN | 无条件跳转：`pc = NNN` |
| `2NNN` | CALL NNN | 调子程序：`stack[sp++] = pc; pc = NNN` |
| `BNNN` | JP V0, NNN | 跳到 `NNN + V0`（原版用 V0，SUPERCHIP 可选 VX） |

### 条件跳转（执行后**不**改 pc，pc 已经 +2）

| 指令 | 名称 | 说明 |
|------|------|------|
| `3XNN` | SE Vx, NN | 如果 `Vx == NN`，跳过下一条（`pc += 2`） |
| `4XNN` | SNE Vx, NN | 如果 `Vx != NN`，跳过下一条 |
| `5XY0` | SE Vx, Vy | 如果 `Vx == Vy`，跳过下一条 |
| `9XY0` | SNE Vx, Vy | 如果 `Vx != Vy`，跳过下一条 |

### 赋值与算术

| 指令 | 名称 | 说明 |
|------|------|------|
| `6XNN` | LD Vx, NN | `Vx = NN` |
| `7XNN` | ADD Vx, NN | `Vx = Vx + NN`（**不**影响 VF，8 位回绕） |
| `8XY0` | LD Vx, Vy | `Vx = Vy` |
| `8XY1` | OR Vx, Vy | `Vx = Vx \| Vy` |
| `8XY2` | AND Vx, Vy | `Vx = Vx & Vy` |
| `8XY3` | XOR Vx, Vy | `Vx = Vx ^ Vy` |
| `8XY4` | ADD Vx, Vy | `Vx = Vx + Vy`，**进位写 VF**：和 > 255 时 VF=1，否则 0。`Vx` 取低 8 位 |
| `8XY5` | SUB Vx, Vy | `Vx = Vx - Vy`，**借位写 VF**：`Vx > Vy` 时 VF=1（无借位），否则 0 |
| `8XY6` | SHR Vx, Vy | 原 CHIP-8：`Vy >>= 1`，移出的最低位写入 VF，再把结果赋给 Vx；现代常见简化：`Vx >>= 1`，最低位写入 VF。**两种约定不同**，建议先按"Vy 先移再赋 Vx"实现，遇到行为不正常的 ROM 再换 |
| `8XY7` | SUBN Vx, Vy | `Vx = Vy - Vx`，借位写 VF：`Vy > Vx` 时 VF=1 |
| `8XYE` | SHL Vx, Vy | 同 SHR，方向相反，最高位写入 VF |

### 数据交互

| 指令 | 名称 | 说明 |
|------|------|------|
| `ANNN` | LD I, NNN | `I = NNN` |
| `CXNN` | RND Vx, NN | `Vx = rand() & NN`（rand 是 0..255 的随机数） |
| `DXYN` | DRW Vx, Vy, N | **绘制精灵**：从 `I` 指向的内存读 N 个字节，在 (Vx, Vy) 处与显示 XOR，碰撞写 VF。`I` 在绘制后是否变化不同实现有差异，传统上**不变** |

### 输入与计时

| 指令 | 名称 | 说明 |
|------|------|------|
| `EX9E` | SKP Vx | 如果键 `Vx` 当前按下，跳过下一条 |
| `EXA1` | SKNP Vx | 如果键 `Vx` 当前**未**按下，跳过下一条 |
| `FX07` | LD Vx, DT | `Vx = delay_timer` |
| `FX0A` | LD Vx, K | 等待按键：暂停 CPU 直到任意键按下，键号写入 Vx。常见实现：检测到一次按键"按下事件"时才把键号放 Vx 并继续 |
| `FX15` | LD DT, Vx | `delay_timer = Vx` |
| `FX18` | LD ST, Vx | `sound_timer = Vx` |

### 内存与 I 寄存器

| 指令 | 名称 | 说明 |
|------|------|------|
| `FX1E` | ADD I, Vx | `I = I + Vx`（不影响 VF，注意溢出回绕 16 位） |
| `FX29` | LD F, Vx | `I = 字体 Vx 字符的地址`。Vx 取低 4 位作字符编号 0..F，I 设为 `0x00 + 字符编号 × 5` |
| `FX33` | LD B, Vx | **BCD**：把 Vx 拆成百、十、个位，存到 `I, I+1, I+2`。例如 Vx=0xFF=255 → `memory[I]=2, memory[I+1]=5, memory[I+2]=5` |
| `FX55` | LD [I], Vx | **存**：把 V0..Vx 依次存到从 I 开始的内存。传统 CHIP-8 把 I 加上 X+1；现代实现不修改 I。**两种都要知道** |
| `FX65` | LD Vx, [I] | **读**：从 I 开始读 X+1 字节到 V0..Vx。I 修改约定同上 |

## 常见的"陷阱"约定差异

CHIP-8 没有官方规范，社区实现有几种"方言"，主要差异：

| 行为 | 选项 A（传统） | 选项 B（现代/SUPERCHIP） |
|------|---------------|--------------------------|
| `8XY6`/`8XYE` 移位 | 用 Vy 移位后赋给 Vx | 直接用 Vx 移位 |
| `FX55`/`FX65` 读写 I 后 | I 增加 X+1 | I 不变 |
| `BNNN` 跳转地址偏移 | 加 V0 | 加 VX（X 由第二个 nibble 指定） |
| 显示越界 | 绕回 | 裁剪 |

新人写第一个模拟器建议：**先全部选"传统方式"**，跑测试 ROM 通过后再说。如果碰到画面错乱的游戏，针对性切换。

## 常用宏 / 提取操作数

```c
#define X     ((opcode >> 8) & 0x0F)
#define Y     ((opcode >> 4) & 0x0F)
#define N      (opcode & 0x000F)
#define NN     (opcode & 0x00FF)
#define NNN    (opcode & 0x0FFF)
```

用这套宏会让译码代码干净很多。
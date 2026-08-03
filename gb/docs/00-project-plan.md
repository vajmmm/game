# 00 - GB 项目总体路线图

## 为什么做 GB

CHIP-8 是虚拟机规范，GB 是第一台**真实硬件**。CHIP-8 教会了"取指-译码-执行"的核心循环，GB 在此基础上引入三个 CHIP-8 完全没有的新维度：

1. **总线与地址空间** —— CPU 通过 64KB 统一地址总线访问 ROM/VRAM/WRAM/寄存器，CPU 不知道（也不关心）地址背后是哪个硬件。
2. **并行硬件 + 共享时钟** —— CPU、PPU（显示）、APU（音频）是三个并行工作的独立硬件，共享同一根 4.19 MHz 时钟，一帧 70,000 周期里交错推进。
3. **中断** —— 5 个中断源，向量地址固定，由 IME/IE/IF 三级控制。

模拟器必须**按周期交错模拟**这些并行硬件，时序错一位就花屏、中断漏触发。这正是"真实硬件"和 CHIP-8 的本质区别。

## 五阶段路线图

每个阶段有明确验收点，跟 CHIP-8 的"看到像素"一样可验证。

### 阶段 1: CPU 核心（最大的硬骨头）

**目标**：完整 LR35902 指令集 + 中断基础，通过 **blargg cpu_instrs** 测试。

- `bus` 最小集：ROM / WRAM(含镜像) / HRAM / IF / IE（骨架已就位）
- CPU 全部指令（~250+ 条）：8/16 位加载、算术逻辑、跳转调用、栈、CB 前缀位操作、特殊指令（DAA/HALT/STOP/DI/EI/RLCA…）
- 每条指令返回正确的 T-cycle 数（时钟精确）
- 中断：IF/IE 检查、向量跳转、RETI

**完成标志**：`cpu_instrs` 全部测试绿（console 模式靠串口输出判定，见下）。

### 阶段 2: 内存映射 + 卡带

**目标**：完整地址空间 + MBC0/MBC1，能加载超过 32KB 的 ROM。

- VRAM / OAM / I/O 寄存器区挂到总线
- MBC1 的 ROM/RAM bank 切换、锁存
- 验证：blargg `mem_timing`、`dmg_sound`（sound 暂放，可只看映射部分）

### 阶段 3: PPU + 显示

**目标**：LCD 渲染管线，看到画面。

- 扫描线渲染（每行 456 周期，144 可见行 + 10 VBlank 行）
- 背景 / 窗口 / 精灵三层
- 从 chip8 平台层复制改造 SDL 显示
- 验证：`dmg-acid2` 对齐测试

### 阶段 4: 计时器 + 输入 + 完整中断

**目标**：真游戏能玩。

- 8/16 位计时器（Timer 中断）
- Joypad 按键
- 完整中断时序（STAT/VBlank 时机）
- 验证：Tetris / Dr. Mario 可玩

### 阶段 5: 音频 + 优化（可选）

APU 4 声道、音高/音量包络，DMA，调试器。

## 阶段 1 详细任务

### 指令集地图（实现时按此分类逐个击破）

| 分类 | 例子 | 难点 |
|---|---|---|
| 8位加载 | LD r,r / LD r,n / LD r,(HL) | HL 作指针；同一寄存器对禁止（LD B,B 合法但无意义） |
| 16位加载 | LD rr,nn / LD (nn),SP / LD SP,HL | 组合寄存器对 |
| 算术逻辑 | ADD/ADC/SUB/SBC/AND/OR/XOR/CP/INC/DEC | **H 半进位**每条都要算 |
| 16位算术 | ADD HL,rr / INC rr / DEC rr | 只影响部分标志 |
| 特殊 | DAA / CPL / SCF / CCF / NOP / HALT / STOP / DI / EI | DAA 依赖 H/N，逻辑最绕 |
| 跳转 | JP nn / JP cc,nn / JR / JR cc,+e | 相对偏移**有符号** |
| 调用返回 | CALL / RET / RET cc / RETI / RST | 栈操作 + 中断相关 |
| 栈 | PUSH rr / POP rr | 先压后弹的 16 位 |
| CB 前缀 | BIT/RES/SET + 移位 RLC/RR/SLA/SWAP… | 第二字节译码，16 条 × 3 种操作 |

### 硬性要求

1. **每条指令返回真实周期数**。GB 时钟精度决定一切，偷懒返回常数会导致 PPU/音频全部错位。实现时查 pandocs 的周期表。
2. **标志位 F 低位恒 0**。写入 F 时只保留高 4 位。
3. **HALT**：置 halted=true，外部中断唤醒后恢复。STOP 阶段 1 可先按 NOP 处理。
4. **中断时序**：当前指令执行完才检查中断；HALT 时周期按 4 T-cycle 计直到被唤醒。

### 验证方法（blargg cpu_instrs）

- 测试套件：`retrio/gb-test-roms`（GitHub），把 `cpu_instrs` 相关 `.gb` 放到 `roms/`。
- 阶段 1 无显示，判定靠**串口输出**：blargg 测试在完成/失败时把字符写入 `FF01`（SC/SB 串口寄存器）。需要在 console 驱动里检测 `FF01` 写入并打印。
  - 全部通过：最后一个字节是 `0x03`（pass 标志），否则 0x00/其他（失败 + 错误码）。
- 实现方式：在 bus_write 里钩住 `FF01` 写（阶段 1 暂不作为 I/O 区，但为验证需要，可在 bus 里加一个"串口输出回调"，或先直接在 main 里每步检查——推荐前者，但实现你定）。

> 提示：cpu_instrs 有几个变体（各指令子集），先把 `cpu_instrs.gb`（全部）跑绿就是阶段 1 通关。

## 编译

项目根目录（当前主线只构建 gb）：

```powershell
cmake -S . -B build
cmake --build build
```

或直接手动编译（阶段 1 无外部依赖）：

```bash
cd gb
gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -I src -o gb.exe src/bus.c src/cpu.c src/main.c
```

运行：

```bash
./gb.exe roms/cpu_instrs.gb 10000000
```

## 参考资料（都免费）

- **pandocs**（Pan Docs）— GB 硬件完整文档，指令周期表 + 寄存器详解，首选
- **The Ultimate Game Boy Talk**（video）— 硬件架构直观总览
- **gbdev 社区** — 文档、测试 ROM、讨论
- 本项目的 `docs/01-*` 各阶段参考手册（后续补充）

## 目录约定

```
gb/
├── docs/          ← 学习文档（00 是路线图，后续 01 架构、02 指令表…）
├── src/           ← bus.c/.h cpu.c/.h main.c，后续 ppu/ apu/ platform/
├── roms/          ← 测试 ROM（版权归原作者，放本地）
├── tests/         ← 自写小测试 / 脚本
└── build/         ← 构建产物
```

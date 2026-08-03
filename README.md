# 模拟器学习项目 / Emulator Learning Project

这里是一个总项目，用来从《图灵完备》游戏过渡到实际的模拟器开发。先做 CHIP-8，后面会尝试更多架构（6502、自定义 CPU、SUPERCHIP 等）。

This is an umbrella project for learning emulator development after finishing the *Turing Complete* game. CHIP-8 first, more architectures later.

## 当前子项目 / Current Sub-project

**CHIP-8** —— 在 `chip8/` 目录下。用 C 从零实现，按五阶段推进。

详见 `chip8/docs/00-project-plan.md`。

## 目录约定 / Conventions

```
game/
├── README.md           ← 你在这里 / You are here
├── CMakeLists.txt      ← 总构建入口 / Top-level build
├── chip8/              ← 第一个子项目 / First sub-project
│   ├── docs/           ← 学习文档 / Learning docs
│   ├── src/            ← 源代码 / Source code
│   ├── roms/           ← 测试 ROM / Test ROMs
│   ├── tests/          ← 测试 / Tests
│   └── build/          ← 构建产物 / Build output
└── <未来其他子项目>/    ← 6502 / 自定义 CPU 等放在并列位置
```

每个子项目一个独立目录，包含自己的 docs/src/roms/tests/build，互不打扰。根 CMakeLists.txt 目前只构建 chip8——以后加新子项目时升级为 add_subdirectory 模式即可。

## 总体学习路线 / Overall Roadmap

**CHIP-8（进行中）**：从 NAND 直觉到第一个能跑的模拟器。五阶段，详见 `chip8/docs/00-project-plan.md`。

- [ ] 阶段 1: 项目骨架与硬件状态建模
- [ ] 阶段 2: 取指-译码-执行主循环
- [ ] 阶段 3: 显示与输入
- [ ] 阶段 4: 完善指令集与测试
- [ ] 阶段 5: 优化与扩展思考

**后续计划**（CHIP-8 完成后）：
- **GB（Game Boy）** —— 真实硬件：Sharp LR35902 CPU + PPU + 中断 + 卡带映射器。从"虚拟机"到"真实硬件"的关键一步。
- **SFC / NES** —— 6502（NES）或 5A22（SFC）方向，经典主机。具体先做哪个到时候再定。
- **GBA** —— 32 位 ARM7TDMI + Z80 音频 CPU，ARM/Thumb 双指令集，DMA、多 PPU 模式。
- 其他可选探索：SUPERCHIP 扩展、用 Rust 重写其中一个、自定义指令集。

## 快速开始 / Quick Start

```bash
cd chip8/build
cmake ..
make
./chip8 path/to/rom.ch8
```

## 你的背景 / Your Background

- 完成了《图灵完备》CPU 关卡
- 学过 C、Python
- 目标：通过写模拟器把游戏里的硬件直觉变成实际的工程能力
- 规划：CHIP-8 之后继续其他模拟器，循序渐进
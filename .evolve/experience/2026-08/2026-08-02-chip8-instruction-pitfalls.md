# CHIP-8 指令实现经验

## 源：2026-08-02 会话（多轮连续）

### 8XXX 组算术指令的 VF 标志

**SUBN (8XY7) borrow 方向：** VF = V[y] >= V[x] ? 1 : 0。容易写成 V[x] >= V[y]（和 SUB 混淆）。用户亲自发现并纠正了这个 bug。

**SHL (8XYE) VF 赋值：** VF = (Vy >> 7) & 0x01，取最高位作为进位标志。写过 VF = 31 的 bug，因为手误。

**SHR (8XY6) VF 赋值：** VF = Vy & 0x01，取最低位。

### 传统 CHIP-8 vs SUPERCHIP 的 SHR/SHL 行为

- 传统 CHIP-8：SHR/SHL 的目标操作数是 Vy（Vx = Vy >> 1 或 Vx = Vy << 1），VF 取被移位前的 LSB/MSB
- SUPERCHIP：SHR/SHL 的目标操作数是 Vx（Vx = Vx >> 1 或 Vx = Vx << 1）
- 本项目选择了传统 CHIP-8 行为

### FX55/FX65 的 I 寄存器修改

- 传统 CHIP-8：FX55 和 FX65 执行后 I 会增加 x+1（有些实现不修改）
- 本项目选择不修改 I（更兼容现代 ROM）

### FX0A 等待按键

- 简化实现：轮询 keys[] 数组，无键按下时 pc -= 2 退回重试
- 缺点：电平触发，按住不放会反复触发
- Phase 5 可改为边沿触发（跟踪上次按键状态）

### 键盘映射

- 物理布局：1234/QWER/ASDF/ZXCV → CHIP-8 0x0..0xF
- 用 SDL_GetKeyboardState（全程轮询）而非 SDL_KEYDOWN 事件
- 原因：CHIP-8 按键检测和 CPU 执行同步（FX0A 等），用轮询更简单
